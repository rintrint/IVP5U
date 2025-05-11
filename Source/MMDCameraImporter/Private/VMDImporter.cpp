// Copyright Epic Games, Inc. All Rights Reserved.

#include "VMDImporter.h"

#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "ISequencerModule.h"
#include "LevelEditorViewport.h"
#include "MMDCameraImporter.h"
#include "MMDImportHelper.h"
#include "MovieSceneToolHelpers.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FMmdCameraImporterModule"

void FVmdImporter::SetFilePath(const FString& InFilePath)
{
	FilePath = InFilePath;
}

bool FVmdImporter::IsValidVmdFile()
{
	if (!FileReader.IsValid())
	{
		FileReader = TUniquePtr<FArchive>(OpenFile(FilePath));

		if (!FileReader.IsValid())
		{
			UE_LOG(LogMMDCameraImporter, Error, TEXT("Can't open file(%s)"), *FilePath);
			return false;
		}
	}

	const int64 FileSize = FileReader->TotalSize();

	if (FileSize < sizeof(FVmdObject::FHeader))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(FileSize < sizeof(FVmdObject::FHeader))"));
		return false;
	}

	FileReader->Seek(0);
	uint8 Magic[30];
	FileReader->Serialize(Magic, sizeof Magic);
	if (FMmdImportHelper::ShiftJisToFString(Magic, sizeof Magic) != "Vocaloid Motion Data 0002")
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File is not vmd format"));
		return false;
	}

	int64 Offset = sizeof(FVmdObject::FHeader);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of bone keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 BoneKeyFrameCount = 0;
	FileReader->Serialize(&BoneKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32) + (sizeof(FVmdObject::FBoneKeyFrame) * BoneKeyFrameCount);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of morph keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 MorphKeyFrameCount = 0;
	FileReader->Serialize(&MorphKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32) + (sizeof(FVmdObject::FMorphKeyFrame) * MorphKeyFrameCount);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of camera keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 CameraKeyFrameCount = 0;
	FileReader->Serialize(&CameraKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32) + (sizeof(FVmdObject::FCameraKeyFrame) * CameraKeyFrameCount);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of light keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 LightKeyFrameCount = 0;
	FileReader->Serialize(&LightKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32) + (sizeof(FVmdObject::FLightKeyFrame) * LightKeyFrameCount);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of self shadow keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 SelfShadowKeyFrameCount = 0;
	FileReader->Serialize(&SelfShadowKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32) + (sizeof(FVmdObject::FSelfShadowKeyFrame) * SelfShadowKeyFrameCount);

	if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of properties keyframes)"));
		return false;
	}
	FileReader->Seek(Offset);
	uint32 PropertyKeyFrameCount = 0;
	FileReader->Serialize(&PropertyKeyFrameCount, sizeof(uint32));
	Offset += sizeof(uint32);

	for (PTRINT i = 0; i < PropertyKeyFrameCount; ++i)
	{
		Offset += sizeof(FVmdObject::FPropertyKeyFrame);

		if (FileSize < Offset + static_cast<int64>(sizeof(uint32)))
		{
			UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(Failed to read number of IK state keyframes)"));
			return false;
		}
		FileReader->Seek(Offset);
		uint32 IkStateCount = 0;
		FileReader->Serialize(&IkStateCount, sizeof(uint32));
		Offset += sizeof(uint32) + (sizeof(FVmdObject::FPropertyKeyFrame::FIkState) * IkStateCount);
	}

	if (FileSize < Offset)
	{
		UE_LOG(LogMMDCameraImporter, Error, TEXT("File seems to be corrupt(FileSize < Offset)"));
		return false;
	}

	if (FileSize != Offset)
	{
		UE_LOG(LogMMDCameraImporter, Warning, TEXT("File seems to be corrupt or additional data exists"));
	}

	return true;
}

FVmdParseResult FVmdImporter::ParseVmdFile()
{
	if (!FileReader.IsValid())
	{
		FileReader = TUniquePtr<FArchive>(OpenFile(FilePath));

		if (!FileReader.IsValid())
		{
			UE_LOG(LogMMDCameraImporter, Error, TEXT("Can't open file(%s)"), *FilePath);

			FVmdParseResult FailedResult;
			FailedResult.bIsSuccess = false;

			return FailedResult;
		}
	}

	FScopedSlowTask ImportVmdTask(7, LOCTEXT("ReadingVMDFile", "Reading VMD File"));
	ImportVmdTask.MakeDialog(true, true);

	FVmdParseResult VmdParseResult;
	VmdParseResult.bIsSuccess = false;

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileHeader", "Reading Header"));
	FileReader->Seek(0);
	FileReader->Serialize(&VmdParseResult.Header, sizeof(FVmdObject::FHeader));

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileBoneKeyFrames", "Reading Bone Key Frames"));
	uint32 BoneKeyFrameCount = 0;
	FileReader->Serialize(&BoneKeyFrameCount, sizeof(uint32));
	VmdParseResult.BoneKeyFrames.SetNum(BoneKeyFrameCount);
	FileReader->Serialize(VmdParseResult.BoneKeyFrames.GetData(), sizeof(FVmdObject::FBoneKeyFrame) * BoneKeyFrameCount);
	VmdParseResult.BoneKeyFrames.Sort([](const FVmdObject::FBoneKeyFrame& A, const FVmdObject::FBoneKeyFrame& B) { return A.FrameNumber < B.FrameNumber; });

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileMorphKeyFrames", "Reading Morph Key Frames"));
	uint32 MorphKeyFrameCount = 0;
	FileReader->Serialize(&MorphKeyFrameCount, sizeof(uint32));
	VmdParseResult.MorphKeyFrames.SetNum(MorphKeyFrameCount);
	FileReader->Serialize(VmdParseResult.MorphKeyFrames.GetData(), sizeof(FVmdObject::FMorphKeyFrame) * MorphKeyFrameCount);
	VmdParseResult.MorphKeyFrames.Sort([](const FVmdObject::FMorphKeyFrame& A, const FVmdObject::FMorphKeyFrame& B) { return A.FrameNumber < B.FrameNumber; });

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileCameraKeyFrames", "Reading Camera Key Frames"));
	uint32 CameraKeyFrameCount = 0;
	FileReader->Serialize(&CameraKeyFrameCount, sizeof(uint32));
	VmdParseResult.CameraKeyFrames.SetNum(CameraKeyFrameCount);
	FileReader->Serialize(VmdParseResult.CameraKeyFrames.GetData(), sizeof(FVmdObject::FCameraKeyFrame) * CameraKeyFrameCount);
	VmdParseResult.CameraKeyFrames.Sort([](const FVmdObject::FCameraKeyFrame& A, const FVmdObject::FCameraKeyFrame& B) { return A.FrameNumber < B.FrameNumber; });

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileLightKeyFrames", "Reading Light Key Frames"));
	uint32 LightKeyFrameCount = 0;
	FileReader->Serialize(&LightKeyFrameCount, sizeof(uint32));
	VmdParseResult.LightKeyFrames.SetNum(LightKeyFrameCount);
	FileReader->Serialize(VmdParseResult.LightKeyFrames.GetData(), sizeof(FVmdObject::FLightKeyFrame) * LightKeyFrameCount);
	VmdParseResult.LightKeyFrames.Sort([](const FVmdObject::FLightKeyFrame& A, const FVmdObject::FLightKeyFrame& B) { return A.FrameNumber < B.FrameNumber; });

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFileSelfShadowKeyFrames", "Reading Self Shadow Key Frames"));
	uint32 SelfShadowKeyFrameCount = 0;
	FileReader->Serialize(&SelfShadowKeyFrameCount, sizeof(uint32));
	VmdParseResult.SelfShadowKeyFrames.SetNum(SelfShadowKeyFrameCount);
	FileReader->Serialize(VmdParseResult.SelfShadowKeyFrames.GetData(), sizeof(FVmdObject::FSelfShadowKeyFrame) * SelfShadowKeyFrameCount);
	VmdParseResult.SelfShadowKeyFrames.Sort([](const FVmdObject::FSelfShadowKeyFrame& A, const FVmdObject::FSelfShadowKeyFrame& B) { return A.FrameNumber < B.FrameNumber; });

	if (ImportVmdTask.ShouldCancel())
	{
		return VmdParseResult;
	}
	ImportVmdTask.EnterProgressFrame(1, LOCTEXT("ReadingVMDFilePropertyKeyFrames", "Reading Property Key Frames"));
	uint32 PropertyKeyFrameCount = 0;
	FileReader->Serialize(&PropertyKeyFrameCount, sizeof(uint32));
	VmdParseResult.PropertyKeyFrames.SetNum(PropertyKeyFrameCount);
	{
		FScopedSlowTask ImportPropertyKeyFramesTask(PropertyKeyFrameCount, LOCTEXT("ReadingVMDFilePropertyKeyFrames", "Reading Property Key Frames"));

		for (PTRINT i = 0; i < PropertyKeyFrameCount; ++i)
		{
			if (ImportPropertyKeyFramesTask.ShouldCancel())
			{
				return VmdParseResult;
			}
			ImportPropertyKeyFramesTask.EnterProgressFrame();

			FVmdObject::FPropertyKeyFrame PropertyKeyFrame;
			FileReader->Serialize(&PropertyKeyFrame, sizeof(FVmdObject::FPropertyKeyFrame));
			VmdParseResult.PropertyKeyFrames[i].FrameNumber = PropertyKeyFrame.FrameNumber;
			VmdParseResult.PropertyKeyFrames[i].Visible = static_cast<bool>(PropertyKeyFrame.Visible);

			uint32 IkStateCount = 0;
			FileReader->Serialize(&IkStateCount, sizeof(uint32));
			VmdParseResult.PropertyKeyFrames[i].IkStates.SetNum(IkStateCount);
			FileReader->Serialize(VmdParseResult.PropertyKeyFrames[i].IkStates.GetData(), sizeof(FVmdObject::FPropertyKeyFrame::FIkState) * IkStateCount);
		}
	}
	VmdParseResult.PropertyKeyFrames.Sort([](const FVmdParseResult::FPropertyKeyFrameWithIkState& A, const FVmdParseResult::FPropertyKeyFrameWithIkState& B) { return A.FrameNumber < B.FrameNumber; });

	VmdParseResult.bIsSuccess = true;

	return VmdParseResult;
}

void FVmdImporter::ImportVmdCamera(
	const FVmdParseResult& InVmdParseResult,
	UMovieSceneSequence* InSequence,
	ISequencer& InSequencer,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	const bool bNotifySlate = !FApp::IsUnattended() && !GIsRunningUnattendedScript;

	if (InVmdParseResult.CameraKeyFrames.Num() == 0)
	{
		UE_LOG(LogMMDCameraImporter, Warning, TEXT("This VMD file is not camera motion"));

		if (bNotifySlate)
		{
			FNotificationInfo Info(LOCTEXT("NoCameraMotionError", "This VMD file is not camera motion"));
			Info.ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Fail);
		}

		return;
	}

	TArray<FGuid> CameraGuids;
	TArray<FGuid> CameraCenterGuids;
	CameraGuids.Reserve(ImportVmdSettings->CameraCount);
	CameraCenterGuids.Reserve(ImportVmdSettings->CameraCount);

	for (PTRINT i = 0; i < ImportVmdSettings->CameraCount; ++i)
	{
		UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
		check(World != nullptr && "World is null");

		FActorSpawnParameters CameraCenterSpawnParams;
		CameraCenterSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* NewCameraCenter = World->SpawnActor<AActor>(CameraCenterSpawnParams);
		NewCameraCenter->SetActorLabel(FString::Format(TEXT("MmdCameraCenter{0}"), { i }));
		USceneComponent* RootSceneComponent = NewObject<USceneComponent>(NewCameraCenter, TEXT("SceneComponent"));
		NewCameraCenter->SetRootComponent(RootSceneComponent);
		NewCameraCenter->AddInstanceComponent(RootSceneComponent);

		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACineCameraActor* NewCamera = World->SpawnActor<ACineCameraActor>(CameraSpawnParams);
		NewCamera->SetActorLabel(FString::Format(TEXT("MmdCamera{0}"), { i }));

		NewCamera->AttachToActor(NewCameraCenter, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

		{
			// ReSharper disable once CppUseStructuredBinding
			const FVmdObject::FCameraKeyFrame FirstFrame = InVmdParseResult.CameraKeyFrames[0];
			const float UniformScale = ImportVmdSettings->ImportUniformScale * 100.f;

			FVector CameraRelativeLocation = FVector::ZeroVector;
			switch (ImportVmdSettings->DistanceAxisMapping)
			{
				case EVMDAxisMapping::VMD_X:
					CameraRelativeLocation = FVector((FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale, 0, 0);
					break;
				case EVMDAxisMapping::VMD_Y:
					CameraRelativeLocation = FVector(0, (FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale, 0);
					break;
				case EVMDAxisMapping::VMD_Z:
					CameraRelativeLocation = FVector(0, 0, (FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale);
					break;
				case EVMDAxisMapping::VMD_NEG_X:
					CameraRelativeLocation = FVector(-(FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale, 0, 0);
					break;
				case EVMDAxisMapping::VMD_NEG_Y:
					CameraRelativeLocation = FVector(0, -(FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale, 0);
					break;
				case EVMDAxisMapping::VMD_NEG_Z:
					CameraRelativeLocation = FVector(0, 0, -(FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale);
					break;
				default:
					CameraRelativeLocation = FVector((FirstFrame.Distance - ImportVmdSettings->DistanceOffset) * UniformScale, 0, 0);
					break;
			}
			NewCamera->SetActorRelativeLocation(CameraRelativeLocation);

			// Position:
			// X -> Y
			// Y -> Z
			// Z -> X
			// Position mapping with switch cases for each axis
			float xPosition, yPosition, zPosition;

			// X position mapping
			switch (ImportVmdSettings->AxisMappingX)
			{
				case EVMDAxisMapping::VMD_X:
					xPosition = FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_Y:
					xPosition = FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_Z:
					xPosition = FirstFrame.Position[2];
					break;
				case EVMDAxisMapping::VMD_NEG_X:
					xPosition = -FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_NEG_Y:
					xPosition = -FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_NEG_Z:
					xPosition = -FirstFrame.Position[2];
					break;
				default:
					xPosition = FirstFrame.Position[2];
					break;
			}

			// Y position mapping
			switch (ImportVmdSettings->AxisMappingY)
			{
				case EVMDAxisMapping::VMD_X:
					yPosition = FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_Y:
					yPosition = FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_Z:
					yPosition = FirstFrame.Position[2];
					break;
				case EVMDAxisMapping::VMD_NEG_X:
					yPosition = -FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_NEG_Y:
					yPosition = -FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_NEG_Z:
					yPosition = -FirstFrame.Position[2];
					break;
				default:
					yPosition = FirstFrame.Position[0];
					break;
			}

			// Z position mapping
			switch (ImportVmdSettings->AxisMappingZ)
			{
				case EVMDAxisMapping::VMD_X:
					zPosition = FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_Y:
					zPosition = FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_Z:
					zPosition = FirstFrame.Position[2];
					break;
				case EVMDAxisMapping::VMD_NEG_X:
					zPosition = -FirstFrame.Position[0];
					break;
				case EVMDAxisMapping::VMD_NEG_Y:
					zPosition = -FirstFrame.Position[1];
					break;
				case EVMDAxisMapping::VMD_NEG_Z:
					zPosition = -FirstFrame.Position[2];
					break;
				default:
					zPosition = FirstFrame.Position[1];
					break;
			}

			// Now set the actor location with the mapped values
			NewCameraCenter->SetActorRelativeLocation(
				FVector(
					xPosition * UniformScale,
					yPosition * UniformScale,
					zPosition * UniformScale));

			// Rotation:
			// X -> Y
			// Y -> Z
			// Z -> X
			NewCameraCenter->SetActorRelativeRotation(
				FRotator(
					FMath::RadiansToDegrees(FirstFrame.Rotation[2]) + ImportVmdSettings->RotationOffsetY,
					FMath::RadiansToDegrees(FirstFrame.Rotation[0]) + ImportVmdSettings->RotationOffsetZ,
					-FMath::RadiansToDegrees(FirstFrame.Rotation[1]) + ImportVmdSettings->RotationOffsetX));

			UCineCameraComponent* CineCameraComponent = NewCamera->GetCineCameraComponent();

			CineCameraComponent->Filmback.SensorWidth = ImportVmdSettings->CameraFilmback.SensorWidth;
			CineCameraComponent->Filmback.SensorHeight = ImportVmdSettings->CameraFilmback.SensorHeight;

			CineCameraComponent->CurrentFocalLength =
				ComputeFocalLength(FirstFrame.ViewAngle + ImportVmdSettings->FOVOffset,
					CineCameraComponent->Filmback.SensorWidth)
				* ImportVmdSettings->FocalLengthScale * 0.5f;

			CineCameraComponent->FocusSettings.FocusMethod = ECameraFocusMethod::Disable;
		}

		TArray<TWeakObjectPtr<AActor>> NewActors;
		NewActors.Add(NewCameraCenter);
		NewActors.Add(NewCamera);
		TArray<FGuid> NewActorGuids = InSequencer.AddActors(NewActors);

		CameraCenterGuids.Add(NewActorGuids[0]);
		CameraGuids.Add(NewActorGuids[1]);
	}

	ImportVmdCameraToExisting(
		InVmdParseResult,
		InSequence,
		&InSequencer,
		InSequencer.GetFocusedTemplateID(),
		CameraGuids,
		CameraCenterGuids,
		ImportVmdSettings);
}

FArchive* FVmdImporter::OpenFile(const FString FilePath)
{
	return IFileManager::Get().CreateFileReader(*FilePath);
}

void FVmdImporter::ImportVmdCameraToExisting(
	const FVmdParseResult& InVmdParseResult,
	UMovieSceneSequence* InSequence,
	IMovieScenePlayer* Player,
	FMovieSceneSequenceIDRef TemplateID,
	const TArray<FGuid>& CameraGuids,
	const TArray<FGuid>& CameraCenterGuids,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	TArray<FGuid> CameraPropertyOwnerGuids;
	TArray<UCineCameraComponent*> CameraComponents;

	// ReSharper disable once CppUseStructuredBinding
	for (const FGuid& MmdCameraGuid : CameraGuids)
	{
		const TArrayView<TWeakObjectPtr<>> BoundObjects = Player->FindBoundObjects(MmdCameraGuid, TemplateID);
		for (TWeakObjectPtr<>& WeakObject : BoundObjects)
		{
			// ReSharper disable once CppTooWideScopeInitStatement
			UObject* FoundObject = WeakObject.Get();

			if (FoundObject && FoundObject->GetClass()->IsChildOf(ACineCameraActor::StaticClass()))
			{
				const ACineCameraActor* CineCameraActor = Cast<ACineCameraActor>(FoundObject);
				UCineCameraComponent* CameraComponent = CineCameraActor->GetCineCameraComponent();

				// Set the default value of the current focal length or field of view section
				// FGuid PropertyOwnerGuid = Player->GetHandleToObject(CameraComponent);
				FGuid PropertyOwnerGuid = GetHandleToObject(CameraComponent, InSequence, Player, TemplateID, true);

				if (!PropertyOwnerGuid.IsValid())
				{
					continue;
				}

				// If copying properties to a spawnable object, the template object must be updated
				// ReSharper disable once CppTooWideScope
				FMovieSceneSpawnable* Spawnable = MovieScene->FindSpawnable(MmdCameraGuid);
				if (Spawnable)
				{
					Spawnable->CopyObjectTemplate(*FoundObject, *InSequence);
				}

				CameraPropertyOwnerGuids.Add(PropertyOwnerGuid);
				CameraComponents.Add(CameraComponent);
			}
		}
	}

	check(CameraPropertyOwnerGuids.Num() == CameraGuids.Num());
	check(CameraComponents.Num() == CameraGuids.Num());

	const TArray<TRange<uint32>> CameraCuts = ImportVmdSettings->CameraCount == 1
		? TArray{ TRange<uint32>(0, InVmdParseResult.CameraKeyFrames.Last().FrameNumber + 1) }
		: ComputeCameraCuts(InVmdParseResult.CameraKeyFrames);

	CreateCameraCutTrack(CameraCuts, CameraGuids, InSequence);

	ImportVmdCameraFocalLengthProperty(
		InVmdParseResult.CameraKeyFrames,
		CameraCuts,
		CameraPropertyOwnerGuids,
		InSequence,
		CameraComponents,
		ImportVmdSettings);

	if (ImportVmdSettings->bAddMotionBlurKey)
	{
		CreateVmdCameraMotionBlurProperty(
			InVmdParseResult.CameraKeyFrames,
			CameraCuts,
			CameraPropertyOwnerGuids,
			InSequence,
			ImportVmdSettings);
	}

	ImportVmdCameraTransform(
		InVmdParseResult.CameraKeyFrames,
		CameraCuts,
		CameraGuids,
		InSequence,
		ImportVmdSettings);

	ImportVmdCameraCenterTransform(
		InVmdParseResult.CameraKeyFrames,
		CameraCuts,
		CameraCenterGuids,
		InSequence,
		ImportVmdSettings);
}

void FVmdImporter::CreateCameraCutTrack(
	const TArray<TRange<uint32>>& InCameraCuts,
	const TArray<FGuid>& ObjectBindings,
	const UMovieSceneSequence* InSequence)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	UMovieSceneCameraCutTrack* CameraCutTrack = GetCameraCutTrack(MovieScene);
	const FFrameRate FrameRate = CameraCutTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
	const int32 FrameRatio = static_cast<int32>(FrameRate.AsDecimal() / 30.f);

	CameraCutTrack->RemoveAllAnimationData();

	for (PTRINT i = 0; i < InCameraCuts.Num(); ++i)
	{
		const TRange<uint32>& CameraCut = InCameraCuts[i];
		const FGuid CameraBinding = ObjectBindings[i % ObjectBindings.Num()];
		CameraCutTrack->AddNewCameraCut(
			UE::MovieScene::FRelativeObjectBindingID(CameraBinding),
			static_cast<int>(CameraCut.GetLowerBoundValue() * FrameRatio));
	}
}

bool FVmdImporter::ImportVmdCameraFocalLengthProperty(
	const TArray<FVmdObject::FCameraKeyFrame>& CameraKeyFrames,
	const TArray<TRange<uint32>>& InCameraCuts,
	const TArray<FGuid>& ObjectBindings,
	const UMovieSceneSequence* InSequence,
	const TArray<UCineCameraComponent*>& InCineCameraComponents,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	check(ObjectBindings.Num() != 0);
	check(InCineCameraComponents.Num() != 0);

	const UMovieScene* MovieScene = InSequence->GetMovieScene();

	const FName TrackName = TEXT("CurrentFocalLength");

	TArray<FMovieSceneFloatChannel*> Channels;
	Channels.Reserve(ObjectBindings.Num());

	for (FGuid ObjectBinding : ObjectBindings)
	{
		UMovieSceneFloatTrack* FloatTrack = MovieScene->FindTrack<UMovieSceneFloatTrack>(ObjectBinding, TrackName);
		if (FloatTrack == nullptr)
		{
			return false;
		}

		FloatTrack->Modify();
		FloatTrack->RemoveAllAnimationData();

		bool bSectionAdded = false;
		UMovieSceneFloatSection* FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->FindOrAddSection(0, bSectionAdded));
		if (!FloatSection)
		{
			return false;
		}

		FloatSection->Modify();

		if (bSectionAdded)
		{
			FloatSection->SetRange(TRange<FFrameNumber>::All());
		}

		FMovieSceneFloatChannel* Channel = FloatSection->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
		Channels.Add(Channel);
	}

	const FFrameRate SampleRate = MovieScene->GetDisplayRate();
	const FFrameRate FrameRate = MovieScene->GetTickResolution();
	const float SensorWidth = InCineCameraComponents.Last()->Filmback.SensorWidth;
	FTangentAccessIndices TangentAccessIndices;
	{
		TangentAccessIndices.ArriveTangentX = 21;
		TangentAccessIndices.ArriveTangentY = 23;
		TangentAccessIndices.LeaveTangentX = 20;
		TangentAccessIndices.LeaveTangentY = 22;
	}

	ImportCameraSingleChannel(
		CameraKeyFrames,
		InCameraCuts,
		Channels,
		SampleRate,
		FrameRate,
		ImportVmdSettings->CameraCutImportType,
		TangentAccessIndices,
		[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
			// 對每一個關鍵幀都應用 FOVOffset
			return KeyFrames.ViewAngle + ImportVmdSettings->FOVOffset;
		},
		[SensorWidth, ImportVmdSettings](const float Value) {
			// 對每一個關鍵幀都應用 FocalLengthScale
			return ComputeFocalLength(Value, SensorWidth) * ImportVmdSettings->FocalLengthScale * 0.5f;
		});

	return true;
}

bool FVmdImporter::CreateVmdCameraMotionBlurProperty(
	const TArray<FVmdObject::FCameraKeyFrame>& CameraKeyFrames,
	const TArray<TRange<uint32>>& InCameraCuts,
	const TArray<FGuid>& ObjectBindings,
	const UMovieSceneSequence* InSequence,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	const FName TrackName = TEXT("PostProcessSettings.MotionBlurAmount");

	TArray<FMovieSceneFloatChannel*> Channels;
	Channels.Reserve(ObjectBindings.Num());

	for (FGuid ObjectBinding : ObjectBindings)
	{
		UMovieSceneFloatTrack* FloatTrack = MovieScene->FindTrack<UMovieSceneFloatTrack>(ObjectBinding, TrackName);
		if (FloatTrack == nullptr)
		{
			MovieScene->Modify();
			FloatTrack = MovieScene->AddTrack<UMovieSceneFloatTrack>(ObjectBinding);
			FloatTrack->SetPropertyNameAndPath("MotionBlurAmount", "PostProcessSettings.MotionBlurAmount");
		}

		FloatTrack->Modify();
		FloatTrack->RemoveAllAnimationData();

		bool bSectionAdded = false;
		UMovieSceneFloatSection* FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->FindOrAddSection(0, bSectionAdded));
		if (!FloatSection)
		{
			return false;
		}

		FloatSection->Modify();

		if (bSectionAdded)
		{
			FloatSection->SetRange(TRange<FFrameNumber>::All());
		}

		if (CameraKeyFrames.Num() == 0)
		{
			return true;
		}

		FMovieSceneFloatChannel* Channel = FloatSection->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
		Channels.Add(Channel);
	}

	const FFrameRate SampleRate = MovieScene->GetDisplayRate();
	const FFrameRate FrameRate = MovieScene->GetTickResolution();
	const FFrameNumber OneSampleFrame = (FrameRate / SampleRate).AsFrameNumber(1);
	const int32 FrameRatio = static_cast<int32>(FrameRate.AsDecimal() / 30.f);
	const float MotionBlurAmount = ImportVmdSettings->MotionBlurAmount;
	const ECameraCutImportType CameraCutImportType = ImportVmdSettings->CameraCutImportType;

	for (FMovieSceneFloatChannel* MovieSceneFloatChannel : Channels)
	{
		MovieSceneFloatChannel->SetDefault(MotionBlurAmount);
	}

	TArray<TRange<uint32>> CameraCutRanges;
	{
		uint32 RangeStart = CameraKeyFrames[0].FrameNumber;

		for (PTRINT i = 1; i < CameraKeyFrames.Num(); ++i)
		{
			// ReSharper disable once CppUseStructuredBinding
			const FVmdObject::FCameraKeyFrame& PreviousFrame = CameraKeyFrames[i - 1];
			// ReSharper disable once CppUseStructuredBinding
			const FVmdObject::FCameraKeyFrame& CurrentFrame = CameraKeyFrames[i];

			if (
				(CurrentFrame.FrameNumber - PreviousFrame.FrameNumber) <= 1 &&

				(CurrentFrame.ViewAngle != PreviousFrame.ViewAngle || CurrentFrame.Distance != PreviousFrame.Distance || CurrentFrame.Position[0] != PreviousFrame.Position[0] || CurrentFrame.Position[1] != PreviousFrame.Position[1] || CurrentFrame.Position[2] != PreviousFrame.Position[2] || CurrentFrame.Rotation[0] != PreviousFrame.Rotation[0] || CurrentFrame.Rotation[1] != PreviousFrame.Rotation[1] || CurrentFrame.Rotation[2] != PreviousFrame.Rotation[2]))
			{
				continue;
			}

			if (PreviousFrame.FrameNumber != RangeStart)
			{
				CameraCutRanges.Push(TRange<uint32>(RangeStart, PreviousFrame.FrameNumber));
			}
			RangeStart = CurrentFrame.FrameNumber;
		}
	}

	TArray<TPair<FFrameNumber, FMovieSceneFloatValue>> Keys;

	if (0 < CameraCutRanges.Num() && CameraCutRanges[0].GetLowerBoundValue() != 0)
	{
		FMovieSceneFloatValue MovieSceneFloatValue;
		MovieSceneFloatValue.Value = MotionBlurAmount;
		MovieSceneFloatValue.InterpMode = RCIM_Constant;
		Keys.Add({ 0, MovieSceneFloatValue });
	}

	for (TRange<uint32>& CameraCutRange : CameraCutRanges)
	{
		const uint32 LowerBound = CameraCutRange.GetLowerBoundValue();
		const uint32 UpperBound = CameraCutRange.GetUpperBoundValue();

		if (CameraCutImportType == ECameraCutImportType::ImportAsIs)
		{
			FMovieSceneFloatValue MovieSceneFloatValue;
			MovieSceneFloatValue.Value = 0.0f;
			MovieSceneFloatValue.InterpMode = RCIM_Constant;
			Keys.Add({ static_cast<int32>(LowerBound) * FrameRatio, MovieSceneFloatValue });
		}
		else
		{
			FMovieSceneFloatValue MovieSceneFloatValue;
			MovieSceneFloatValue.Value = 0.0f;
			MovieSceneFloatValue.InterpMode = RCIM_Constant;
			Keys.Add({ (static_cast<int32>(LowerBound + 1) * FrameRatio) - OneSampleFrame, MovieSceneFloatValue });
		}

		{
			FMovieSceneFloatValue MovieSceneFloatValue;
			MovieSceneFloatValue.Value = MotionBlurAmount;
			MovieSceneFloatValue.InterpMode = RCIM_Constant;
			Keys.Add({ (static_cast<int32>(UpperBound) * FrameRatio) + OneSampleFrame, MovieSceneFloatValue });
		}
	}

	PTRINT CurrentCameraCutIndex = 0;
	for (PTRINT i = 0; i < Keys.Num(); ++i)
	{
		const TPair<FFrameNumber, FMovieSceneFloatValue>& CurrentKey = Keys[i];

		while (static_cast<int32>(InCameraCuts[CurrentCameraCutIndex].GetUpperBoundValue() * FrameRatio) <= CurrentKey.Key)
		{
			CurrentCameraCutIndex += 1;

			TMovieSceneChannelData<FMovieSceneFloatValue> CurrentChannelData = Channels[CurrentCameraCutIndex % Channels.Num()]->GetData();
			const TRange<uint32>& PreviousCameraCut = InCameraCuts[CurrentCameraCutIndex - 1];

			const TPair<FFrameNumber, FMovieSceneFloatValue>& PreviousKey = 0 <= i - 1
				? Keys[i - 1]
				: Keys[0];

			const FFrameNumber CurrentCameraCutStartFrameNumber = static_cast<int32>(PreviousCameraCut.GetUpperBoundValue() * FrameRatio);

			if (CurrentCameraCutIndex != InCameraCuts.Num())
			{
				if (CurrentKey.Key != CurrentCameraCutStartFrameNumber)
				{
					CurrentChannelData.AddKey(CurrentCameraCutStartFrameNumber, PreviousKey.Value);
				}
			}

			if (CurrentCameraCutIndex == InCameraCuts.Num())
			{
				break;
			}
		}

		if (CurrentCameraCutIndex == InCameraCuts.Num())
		{
			break;
		}

		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = Channels[CurrentCameraCutIndex % Channels.Num()]->GetData();
		ChannelData.AddKey(CurrentKey.Key, CurrentKey.Value);
	}

	const TPair<FFrameNumber, FMovieSceneFloatValue>& LastKey = Keys.Last();

	for (PTRINT i = 0; i < Channels.Num() - 1; ++i)
	{
		CurrentCameraCutIndex += 1;
		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = Channels[CurrentCameraCutIndex % Channels.Num()]->GetData();
		ChannelData.AddKey(LastKey.Key, LastKey.Value);
	}

	return true;
}

bool FVmdImporter::ImportVmdCameraTransform(
	const TArray<FVmdObject::FCameraKeyFrame>& CameraKeyFrames,
	const TArray<TRange<uint32>>& InCameraCuts,
	const TArray<FGuid>& ObjectBindings,
	const UMovieSceneSequence* InSequence,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	// 打印相機幀數
	UE_LOG(LogMMDCameraImporter, Log, TEXT("Importing camera transform. KeyFrame count: %d"), CameraKeyFrames.Num());

	// 為所有 6 個轉換通道創建數組
	TArray<FMovieSceneDoubleChannel*> LocationXChannels;
	LocationXChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> LocationYChannels;
	LocationYChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> LocationZChannels;
	LocationZChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationXChannels;
	RotationXChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationYChannels;
	RotationYChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationZChannels;
	RotationZChannels.Reserve(ObjectBindings.Num());

	// 打印前五幀和後五幀的數據
	if (CameraKeyFrames.Num() > 0)
	{
		const int32 MaxFrontFrames = 5;
		const int32 MaxBackFrames = 5;

		for (int32 i = 0; i < CameraKeyFrames.Num(); ++i)
		{
			if (i < MaxFrontFrames || i >= CameraKeyFrames.Num() - MaxBackFrames)
			{
				const FVmdObject::FCameraKeyFrame& Frame = CameraKeyFrames[i];
				UE_LOG(LogMMDCameraImporter, Log, TEXT("VMD Frame[%d]: FrameNumber=%u, Distance=%f, ViewAngle=%u, Perspective=%u, Position=[%f, %f, %f], Rotation=[%f, %f, %f]"),
					i,
					Frame.FrameNumber,
					Frame.Distance,
					Frame.ViewAngle,
					Frame.Perspective,
					Frame.Position[0],
					Frame.Position[1],
					Frame.Position[2],
					Frame.Rotation[0],
					Frame.Rotation[1],
					Frame.Rotation[2]);
			}
			else if (i == MaxFrontFrames)
			{
				UE_LOG(LogMMDCameraImporter, Log, TEXT("..."));
			}
		}
	}

	for (FGuid ObjectBinding : ObjectBindings)
	{
		UMovieScene3DTransformTrack* TransformTrack = MovieScene->FindTrack<UMovieScene3DTransformTrack>(ObjectBinding);
		if (!TransformTrack)
		{
			MovieScene->Modify();
			TransformTrack = MovieScene->AddTrack<UMovieScene3DTransformTrack>(ObjectBinding);
		}

		TransformTrack->Modify();

		bool bSectionAdded = false;
		UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->FindOrAddSection(0, bSectionAdded));
		if (!TransformSection)
		{
			return false;
		}

		TransformSection->Modify();

		if (bSectionAdded)
		{
			TransformSection->SetRange(TRange<FFrameNumber>::All());
		}

		// 獲取所有轉換通道
		const TArrayView<FMovieSceneDoubleChannel*> Channels = TransformSection->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();

		// 檢查我們是否有足夠的通道
		if (Channels.Num() >= 6)
		{
			// 添加所有位置和旋轉通道
			LocationXChannels.Add(Channels[0]); // X 位置通道
			LocationYChannels.Add(Channels[1]); // Y 位置通道
			LocationZChannels.Add(Channels[2]); // Z 位置通道
			RotationXChannels.Add(Channels[3]); // X 旋轉通道
			RotationYChannels.Add(Channels[4]); // Y 旋轉通道
			RotationZChannels.Add(Channels[5]); // Z 旋轉通道
		}
		else
		{
			UE_LOG(LogMMDCameraImporter, Error, TEXT("Not enough transform channels! Expected at least 6, got %d"), Channels.Num());
			return false;
		}
	}

	const FFrameRate SampleRate = MovieScene->GetDisplayRate();
	const FFrameRate FrameRate = MovieScene->GetTickResolution();
	const float UniformScale = ImportVmdSettings->ImportUniformScale * 100.f;
	FTangentAccessIndices TangentAccessIndices;
	{
		TangentAccessIndices.ArriveTangentX = 17;
		TangentAccessIndices.ArriveTangentY = 19;
		TangentAccessIndices.LeaveTangentX = 16;
		TangentAccessIndices.LeaveTangentY = 18;
	}

	// 打印關鍵導入參數 (FrameRate實際上是Desired Tick Interval，在Sequence的進階設置裡)
	UE_LOG(LogMMDCameraImporter, Log, TEXT("Import parameters - UniformScale: %f, FrameRate: %f, SampleRate: %f"),
		UniformScale * 0.01,
		FrameRate.AsDecimal(),
		SampleRate.AsDecimal());

	// 根據 DistanceAxisMapping 決定將 Distance 應用到哪個軸 (以前是寫死將 Distance 導入到 X 軸位置)
	switch (ImportVmdSettings->DistanceAxisMapping)
	{
		case EVMDAxisMapping::VMD_X:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationXChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = Value * UniformScale;
					return scaledValue;
				});
			break;

		case EVMDAxisMapping::VMD_Y:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationYChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = Value * UniformScale;
					return scaledValue;
				});
			break;

		case EVMDAxisMapping::VMD_Z:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationZChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = Value * UniformScale;
					return scaledValue;
				});
			break;

		case EVMDAxisMapping::VMD_NEG_X:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationXChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = -Value * UniformScale;
					return scaledValue;
				});
			break;

		case EVMDAxisMapping::VMD_NEG_Y:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationYChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = -Value * UniformScale;
					return scaledValue;
				});
			break;

		case EVMDAxisMapping::VMD_NEG_Z:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationZChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = -Value * UniformScale;
					return scaledValue;
				});
			break;

		default:
			ImportCameraSingleChannel(
				CameraKeyFrames,
				InCameraCuts,
				LocationXChannels,
				SampleRate,
				FrameRate,
				ImportVmdSettings->CameraCutImportType,
				TangentAccessIndices,
				[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
					return KeyFrames.Distance - ImportVmdSettings->DistanceOffset;
				},
				[UniformScale](const double Value) {
					double scaledValue = Value * UniformScale;
					return scaledValue;
				});
			break;
	}

	return true;
}

bool FVmdImporter::ImportVmdCameraCenterTransform(
	const TArray<FVmdObject::FCameraKeyFrame>& CameraKeyFrames,
	const TArray<TRange<uint32>>& InCameraCuts,
	const TArray<FGuid>& ObjectBindings,
	const UMovieSceneSequence* InSequence,
	const UMmdUserImportVmdSettings* ImportVmdSettings)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	TArray<FMovieSceneDoubleChannel*> LocationXChannels;
	LocationXChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> LocationYChannels;
	LocationYChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> LocationZChannels;
	LocationZChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationXChannels;
	RotationXChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationYChannels;
	RotationYChannels.Reserve(ObjectBindings.Num());
	TArray<FMovieSceneDoubleChannel*> RotationZChannels;
	RotationZChannels.Reserve(ObjectBindings.Num());

	for (FGuid ObjectBinding : ObjectBindings)
	{
		UMovieScene3DTransformTrack* TransformTrack = MovieScene->FindTrack<UMovieScene3DTransformTrack>(ObjectBinding);
		if (!TransformTrack)
		{
			MovieScene->Modify();
			TransformTrack = MovieScene->AddTrack<UMovieScene3DTransformTrack>(ObjectBinding);
		}
		TransformTrack->Modify();

		bool bSectionAdded = false;
		UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->FindOrAddSection(0, bSectionAdded));
		if (!TransformSection)
		{
			return false;
		}

		TransformSection->Modify();

		if (bSectionAdded)
		{
			TransformSection->SetRange(TRange<FFrameNumber>::All());
		}

		const TArrayView<FMovieSceneDoubleChannel*> Channels = TransformSection->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();

		LocationXChannels.Add(Channels[0]);
		LocationYChannels.Add(Channels[1]);
		LocationZChannels.Add(Channels[2]);
		RotationXChannels.Add(Channels[3]);
		RotationYChannels.Add(Channels[4]);
		RotationZChannels.Add(Channels[5]);
	}

	const FFrameRate SampleRate = MovieScene->GetDisplayRate();
	const FFrameRate FrameRate = MovieScene->GetTickResolution();
	const float UniformScale = ImportVmdSettings->ImportUniformScale * 100.f;
	const ECameraCutImportType CameraCutImportType = ImportVmdSettings->CameraCutImportType;

	{
		FTangentAccessIndices LocationXTangentAccessIndices;
		// 根據軸映射選擇適當的切線索引 (這些索引值對應 VMD 文件內部的數據結構位置)
		switch (ImportVmdSettings->AxisMappingX)
		{
			case EVMDAxisMapping::VMD_X:
			case EVMDAxisMapping::VMD_NEG_X:
				// VMD 文件中 X 軸的切線索引值
				LocationXTangentAccessIndices.ArriveTangentX = 1;
				LocationXTangentAccessIndices.ArriveTangentY = 3;
				LocationXTangentAccessIndices.LeaveTangentX = 0;
				LocationXTangentAccessIndices.LeaveTangentY = 2;
				break;
			case EVMDAxisMapping::VMD_Y:
			case EVMDAxisMapping::VMD_NEG_Y:
				// VMD 文件中 Y 軸的切線索引值
				LocationXTangentAccessIndices.ArriveTangentX = 5;
				LocationXTangentAccessIndices.ArriveTangentY = 7;
				LocationXTangentAccessIndices.LeaveTangentX = 4;
				LocationXTangentAccessIndices.LeaveTangentY = 6;
				break;
			case EVMDAxisMapping::VMD_Z:
			case EVMDAxisMapping::VMD_NEG_Z:
				// VMD 文件中 Z 軸的切線索引值
				LocationXTangentAccessIndices.ArriveTangentX = 9;
				LocationXTangentAccessIndices.ArriveTangentY = 11;
				LocationXTangentAccessIndices.LeaveTangentX = 8;
				LocationXTangentAccessIndices.LeaveTangentY = 10;
				break;
			default:
				// 預設使用 VMD 文件中 Z 軸的切線索引值
				LocationXTangentAccessIndices.ArriveTangentX = 9;
				LocationXTangentAccessIndices.ArriveTangentY = 11;
				LocationXTangentAccessIndices.LeaveTangentX = 8;
				LocationXTangentAccessIndices.LeaveTangentY = 10;
				break;
		}

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			LocationXChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			LocationXTangentAccessIndices,
			[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				switch (ImportVmdSettings->AxisMappingX)
				{
					case EVMDAxisMapping::VMD_X:
						return KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_Y:
						return KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_Z:
						return KeyFrames.Position[2];
					case EVMDAxisMapping::VMD_NEG_X:
						return -KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_NEG_Y:
						return -KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_NEG_Z:
						return -KeyFrames.Position[2];
					default:
						return KeyFrames.Position[2];
				}
			},
			[UniformScale](const double Value) {
				return Value * UniformScale;
			});
	}

	{
		FTangentAccessIndices LocationYTangentAccessIndices;
		// 根據軸映射選擇適當的切線索引 (這些索引值對應 VMD 文件內部的數據結構位置)
		switch (ImportVmdSettings->AxisMappingY)
		{
			case EVMDAxisMapping::VMD_X:
			case EVMDAxisMapping::VMD_NEG_X:
				// VMD 文件中 X 軸的切線索引值
				LocationYTangentAccessIndices.ArriveTangentX = 1;
				LocationYTangentAccessIndices.ArriveTangentY = 3;
				LocationYTangentAccessIndices.LeaveTangentX = 0;
				LocationYTangentAccessIndices.LeaveTangentY = 2;
				break;
			case EVMDAxisMapping::VMD_Y:
			case EVMDAxisMapping::VMD_NEG_Y:
				// VMD 文件中 Y 軸的切線索引值
				LocationYTangentAccessIndices.ArriveTangentX = 5;
				LocationYTangentAccessIndices.ArriveTangentY = 7;
				LocationYTangentAccessIndices.LeaveTangentX = 4;
				LocationYTangentAccessIndices.LeaveTangentY = 6;
				break;
			case EVMDAxisMapping::VMD_Z:
			case EVMDAxisMapping::VMD_NEG_Z:
				// VMD 文件中 Z 軸的切線索引值
				LocationYTangentAccessIndices.ArriveTangentX = 9;
				LocationYTangentAccessIndices.ArriveTangentY = 11;
				LocationYTangentAccessIndices.LeaveTangentX = 8;
				LocationYTangentAccessIndices.LeaveTangentY = 10;
				break;
			default:
				// 預設使用 VMD 文件中 X 軸的切線索引值
				LocationYTangentAccessIndices.ArriveTangentX = 1;
				LocationYTangentAccessIndices.ArriveTangentY = 3;
				LocationYTangentAccessIndices.LeaveTangentX = 0;
				LocationYTangentAccessIndices.LeaveTangentY = 2;
				break;
		}

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			LocationYChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			LocationYTangentAccessIndices,
			[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				switch (ImportVmdSettings->AxisMappingY)
				{
					case EVMDAxisMapping::VMD_X:
						return KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_Y:
						return KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_Z:
						return KeyFrames.Position[2];
					case EVMDAxisMapping::VMD_NEG_X:
						return -KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_NEG_Y:
						return -KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_NEG_Z:
						return -KeyFrames.Position[2];
					default:
						return KeyFrames.Position[0];
				}
			},
			[UniformScale](const double Value) {
				return Value * UniformScale;
			});
	}

	{
		FTangentAccessIndices LocationZTangentAccessIndices;
		// 根據軸映射選擇適當的切線索引 (這些索引值對應 VMD 文件內部的數據結構位置)
		switch (ImportVmdSettings->AxisMappingZ)
		{
			case EVMDAxisMapping::VMD_X:
			case EVMDAxisMapping::VMD_NEG_X:
				// VMD 文件中 X 軸的切線索引值
				LocationZTangentAccessIndices.ArriveTangentX = 1;
				LocationZTangentAccessIndices.ArriveTangentY = 3;
				LocationZTangentAccessIndices.LeaveTangentX = 0;
				LocationZTangentAccessIndices.LeaveTangentY = 2;
				break;
			case EVMDAxisMapping::VMD_Y:
			case EVMDAxisMapping::VMD_NEG_Y:
				// VMD 文件中 Y 軸的切線索引值
				LocationZTangentAccessIndices.ArriveTangentX = 5;
				LocationZTangentAccessIndices.ArriveTangentY = 7;
				LocationZTangentAccessIndices.LeaveTangentX = 4;
				LocationZTangentAccessIndices.LeaveTangentY = 6;
				break;
			case EVMDAxisMapping::VMD_Z:
			case EVMDAxisMapping::VMD_NEG_Z:
				// VMD 文件中 Z 軸的切線索引值
				LocationZTangentAccessIndices.ArriveTangentX = 9;
				LocationZTangentAccessIndices.ArriveTangentY = 11;
				LocationZTangentAccessIndices.LeaveTangentX = 8;
				LocationZTangentAccessIndices.LeaveTangentY = 10;
				break;
			default:
				// 預設使用 VMD 文件中 Y 軸的切線索引值
				LocationZTangentAccessIndices.ArriveTangentX = 5;
				LocationZTangentAccessIndices.ArriveTangentY = 7;
				LocationZTangentAccessIndices.LeaveTangentX = 4;
				LocationZTangentAccessIndices.LeaveTangentY = 6;
				break;
		}

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			LocationZChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			LocationZTangentAccessIndices,
			[ImportVmdSettings](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				switch (ImportVmdSettings->AxisMappingZ)
				{
					case EVMDAxisMapping::VMD_X:
						return KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_Y:
						return KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_Z:
						return KeyFrames.Position[2];
					case EVMDAxisMapping::VMD_NEG_X:
						return -KeyFrames.Position[0];
					case EVMDAxisMapping::VMD_NEG_Y:
						return -KeyFrames.Position[1];
					case EVMDAxisMapping::VMD_NEG_Z:
						return -KeyFrames.Position[2];
					default:
						return KeyFrames.Position[1];
				}
			},
			[UniformScale](const double Value) {
				return Value * UniformScale;
			});
	}

	{
		FTangentAccessIndices RotationTangentAccessIndices;
		{
			RotationTangentAccessIndices.ArriveTangentX = 13;
			RotationTangentAccessIndices.ArriveTangentY = 15;
			RotationTangentAccessIndices.LeaveTangentX = 12;
			RotationTangentAccessIndices.LeaveTangentY = 14;
		}

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			RotationXChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			RotationTangentAccessIndices,
			[](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				return KeyFrames.Rotation[2];
			},
			[ImportVmdSettings](const double Value) {
				return FMath::RadiansToDegrees(Value) + ImportVmdSettings->RotationOffsetX;
			});

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			RotationYChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			RotationTangentAccessIndices,
			[](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				return KeyFrames.Rotation[0];
			},
			[ImportVmdSettings](const double Value) {
				return FMath::RadiansToDegrees(Value) + ImportVmdSettings->RotationOffsetY;
			});

		ImportCameraSingleChannel(
			CameraKeyFrames,
			InCameraCuts,
			RotationZChannels,
			SampleRate,
			FrameRate,
			CameraCutImportType,
			RotationTangentAccessIndices,
			[](const FVmdObject::FCameraKeyFrame& KeyFrames) {
				return KeyFrames.Rotation[1];
			},
			[ImportVmdSettings](const double Value) {
				return -FMath::RadiansToDegrees(Value) + ImportVmdSettings->RotationOffsetZ;
			});
	}

	return true;
}

float FVmdImporter::ComputeFocalLength(const float FieldOfView, const float SensorWidth)
{
	// Focal Length = (Film or sensor width) / (2 * tan(FOV / 2))
	return (SensorWidth / 2.f) / FMath::Tan(FMath::DegreesToRadians(FieldOfView / 2.f));
}

TArray<TRange<uint32>> FVmdImporter::ComputeCameraCuts(const TArray<FVmdObject::FCameraKeyFrame>& CameraKeyFrames)
{
	TArray<TRange<uint32>> CameraCuts;

	uint32 RangeStart = CameraKeyFrames[0].FrameNumber;

	for (PTRINT i = 1; i < CameraKeyFrames.Num(); ++i)
	{
		// ReSharper disable once CppUseStructuredBinding
		const FVmdObject::FCameraKeyFrame& PreviousFrame = CameraKeyFrames[i - 1];
		// ReSharper disable once CppUseStructuredBinding
		const FVmdObject::FCameraKeyFrame& CurrentFrame = CameraKeyFrames[i];

		if (
			(CurrentFrame.FrameNumber - PreviousFrame.FrameNumber) <= 1 &&

			(CurrentFrame.ViewAngle != PreviousFrame.ViewAngle || CurrentFrame.Distance != PreviousFrame.Distance || CurrentFrame.Position[0] != PreviousFrame.Position[0] || CurrentFrame.Position[1] != PreviousFrame.Position[1] || CurrentFrame.Position[2] != PreviousFrame.Position[2] || CurrentFrame.Rotation[0] != PreviousFrame.Rotation[0] || CurrentFrame.Rotation[1] != PreviousFrame.Rotation[1] || CurrentFrame.Rotation[2] != PreviousFrame.Rotation[2]))
		{
			CameraCuts.Push(TRange<uint32>(RangeStart, CurrentFrame.FrameNumber));
			RangeStart = CurrentFrame.FrameNumber;
		}
	}

	CameraCuts.Push(TRange<uint32>(RangeStart, CameraKeyFrames.Last().FrameNumber + 1));

	return CameraCuts;
}

FGuid FVmdImporter::GetHandleToObject(
	UObject* InObject,
	UMovieSceneSequence* InSequence,
	IMovieScenePlayer* Player,
	FMovieSceneSequenceIDRef TemplateID,
	const bool bCreateIfMissing)
{
	UMovieScene* MovieScene = InSequence->GetMovieScene();

	// Attempt to resolve the object through the movie scene instance first,
	FGuid PropertyOwnerGuid = FGuid();
	if (InObject != nullptr && !MovieScene->IsReadOnly())
	{
		// ReSharper disable once CppTooWideScopeInitStatement
		const FGuid ObjectGuid = Player->FindObjectId(*InObject, TemplateID);
		if (ObjectGuid.IsValid())
		{
			// Check here for spawnable otherwise spawnables get recreated as possessables, which doesn't make sense
			// ReSharper disable once CppTooWideScope
			const FMovieSceneSpawnable* Spawnable = MovieScene->FindSpawnable(ObjectGuid);
			if (Spawnable)
			{
				PropertyOwnerGuid = ObjectGuid;
			}
			else
			{
				// ReSharper disable once CppTooWideScope
				const FMovieScenePossessable* Possessable = MovieScene->FindPossessable(ObjectGuid);
				if (Possessable)
				{
					PropertyOwnerGuid = ObjectGuid;
				}
			}
		}
	}

	if (PropertyOwnerGuid.IsValid())
	{
		return PropertyOwnerGuid;
	}

	if (bCreateIfMissing)
	{
		// Otherwise, create a possessable for this object. Note this will handle creating the parent possessables if this is a component.
		PropertyOwnerGuid = InSequence->CreatePossessable(InObject);
	}

	return PropertyOwnerGuid;
}

UMovieSceneCameraCutTrack* FVmdImporter::GetCameraCutTrack(UMovieScene* InMovieScene)
{
	// Get the camera cut
	UMovieSceneTrack* CameraCutTrack = InMovieScene->GetCameraCutTrack();
	if (CameraCutTrack == nullptr)
	{
		InMovieScene->Modify();
		CameraCutTrack = InMovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
	}
	return CastChecked<UMovieSceneCameraCutTrack>(CameraCutTrack);
}

#undef LOCTEXT_NAMESPACE
