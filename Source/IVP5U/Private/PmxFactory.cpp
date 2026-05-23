// Copyright 2015-2026 IVP5U contributors

// see S:\UE5SRC\EpicUE5\UE_5.0\Engine\Source\Editor\UnrealEd\Private\Fbx\FbxSkeletalMeshImport.cpp

#include "PmxFactory.h"

#include "CoreMinimal.h"

#include "Animation/Skeleton.h"
#include "AssetNotifications.h"
#include "ComponentReregisterContext.h"
#include "Components/SkeletalMeshComponent.h"
#include "Developer/PhysicsUtilities/Public/PhysicsAssetUtils.h"
#include "Editor.h"
#include "Engine.h"
#include "Engine/StaticMesh.h"
#include "Factories.h"
#include "IMeshBuilderModule.h"
#include "ImportUtils/SkelImport.h"
#include "ImportUtils/SkeletalMeshImportUtils.h"
#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "MeshDescription.h"
#include "ObjectTools.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Rendering/SkeletalMeshModel.h"

#include "IVP5USettings.h"
#include "MMDSkeletalMeshImportData.h"
#include "MMDStaticMeshImportData.h"
#include "PmdImporter.h"
#include "PmxImporter.h"
#include "PmxImportUI.h"

#define LOCTEXT_NAMESPACE "PMXImporter"

DEFINE_LOG_CATEGORY(LogMMD4UE5_PMXFactory)

/////////////////////////////////////////////////////////
// 3 "ProcessImportMesh..." functions outputing Unreal data from a filled FSkeletalMeshBinaryImport
// and a handfull of other minor stuff needed by these
// Fully taken from SkeletalMeshImport.cpp

using namespace SkeletalMeshImportUtils;
using namespace MMD4UE5;
/////////////////////////////////////////////////////////

UPmxFactory::UPmxFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = nullptr;
	// SupportedClass = UPmxFactory::StaticClass();
	Formats.Empty();

	Formats.Add(TEXT("pmd;PMD meshes and animations"));
	Formats.Add(TEXT("pmx;PMX meshes and animations"));

	bCreateNew = false;
	bText = false;
	bEditorImport = true;

	bOperationCanceled = false;
	bDetectImportTypeOnImport = false;

	const UIVP5USettings* Settings = GetDefault<UIVP5USettings>();
	ImportPriority = FMath::Max(1, Settings->ImportPriority);

	// ImportUI = NewObject<UPmxImportUI>(this, NAME_None, RF_NoFlags);
}

bool UPmxFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Equals(TEXT("pmd"), ESearchCase::IgnoreCase)
		|| Extension.Equals(TEXT("pmx"), ESearchCase::IgnoreCase);
}

void UPmxFactory::PostInitProperties()
{
	Super::PostInitProperties();

	ImportUI = NewObject<UPmxImportUI>(this, NAME_None, RF_NoFlags);
}

bool UPmxFactory::DoesSupportClass(UClass* Class)
{
	return (Class == UPmxFactory::StaticClass());
}

UClass* UPmxFactory::ResolveSupportedClass()
{
	return UPmxFactory::StaticClass();
}

bool UPmxFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	return false;
}

void UPmxFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
}

EReimportResult::Type UPmxFactory::Reimport(UObject* Obj)
{
	return EReimportResult::Failed;
}

int32 UPmxFactory::GetPriority() const
{
	const UIVP5USettings* Settings = GetDefault<UIVP5USettings>();
	const int CurrentImportPriority = FMath::Max(1, Settings->ImportPriority);
	return CurrentImportPriority;
}

//////////////////////////////////////////////
// IVP5U Develop Temp Define
//////////////////////////////////////////////
#define DEBUG_MMD_PLUGIN_SKELETON (1)
//////////////////////////////////////////////

bool UPmxFactory::FImportPmxFromFile(const FString& file)
{
	if (!FPaths::FileExists(file))
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("PMX Import error: FIle is not exist."));
		return false;
	}

	const FString filepath = FPaths::GetBaseFilename(file);
	const FName Name(*filepath);
	const FName OutputName(*FString::Printf(TEXT("SKM_%s"), *Name.ToString()));

	const FString _path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("Content/") + filepath);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*_path))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*_path);
	}

	TArray<uint8> File_Result;
	if (!FFileHelper::LoadFileToArray(File_Result, *file))
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("PMX Import error: LoadFileToArray."));
		return false;
	}

	const uint8* DataPtr = File_Result.GetData();
	UObject* NewObject = nullptr;
	FPmxImporter* PmxImporter = FPmxImporter::GetInstance();
	EPMXImportType ForcedImportType = PMXIT_SkeletalMesh;
	// For multiple files, use the same settings
	bDetectImportTypeOnImport = false;
	importAssetTypeMMD = E_MMD_TO_UE5_SKELETON;
	bool bIsPmxFormat = FPaths::GetExtension(file).Equals(TEXT("pmx"), ESearchCase::IgnoreCase);
	// Load MMD Model From binary File
	MMD4UE5::PmxMeshInfo pmxMeshInfoPtr;
	// pmxMaterialImportHelper.InitializeBaseValue(InParent);
	bool pmxImportResult = false;
	if (bIsPmxFormat)
	{
		// pmx ver
		pmxImportResult = pmxMeshInfoPtr.PMXLoaderBinary(DataPtr, nullptr);
	}
	else
	{
		// pmd ver
		MMD4UE5::PmdMeshInfo PmdMeshInfo;
		if (PmdMeshInfo.PMDLoaderBinary(DataPtr, nullptr))
		{
			// Up convert From PMD to PMX format gfor UE5
			pmxImportResult = PmdMeshInfo.ConvertToPmxFormat(&pmxMeshInfoPtr);
		}
	}
	if (!pmxImportResult)
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("PMX Import error: PMXLoaderBinary."));
		return false;
	}

	// show Import Option Slate
	bool bImportAll = false;
	ImportUI->OriginalImportType = EPMXImportType::PMXIT_SkeletalMesh;
	ImportUI->bImportTextures = true;

	FString packpath = "/Game/" + filepath + "/" + filepath;

	UObject* InParent = CreatePackage(*packpath);
	// UObject* InParent = LoadObject<UObject>(ParentPackage, *Name.ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet);
	// InParent->MarkPackageDirty();

	PMXImportOptions* ImportOptions = GetImportOptions(
		PmxImporter,
		ImportUI,
		false, // bShowImportDialog,
		InParent->GetPathName(),
		bOperationCanceled,
		bImportAll,
		bIsPmxFormat,
		ForcedImportType);
	if (bImportAll)
	{
		// If the user chose to import all, we don't show the dialog again and use the same settings for each object until importing another set of files
		// bShowOption = false;
	}

	if (ImportOptions)
	{
		// Warn->BeginSlowTask(NSLOCTEXT("PmxFactory", "BeginImportingPmxMeshTask", "Importing Pmx mesh"), true);

		// For animation and static mesh we assume there is at lease one interesting node by default
		int32 InterestingNodeCount = 1;

		if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON)
		{
#ifdef DEBUG_MMD_PLUGIN_SKELETON

			InterestingNodeCount = 1; // test ? not Anime?

#endif
		}
		else if (importAssetTypeMMD == E_MMD_TO_UE5_STATICMESH)
		{
		}

		if (InterestingNodeCount > 1)
		{
			// the option only works when there are only one asset
			//				ImportOptions->bUsedAsFullName = false;
		}
		UFactory::CurrentFilename = file;
		FString Filename(UFactory::CurrentFilename);

		UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("PMX Import: %s"), *Filename);
		if (InterestingNodeCount > 0)
		{
			int32 NodeIndex = 0;

			int32 ImportedMeshCount = 0;
			UStaticMesh* NewStaticMesh = nullptr;

			if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON) // skeletal mesh
			{
#ifdef DEBUG_MMD_PLUGIN_SKELETON
				int32 TotalNumNodes = 0;

				// for (int32 i = 0; i < SkelMeshArray.Num(); i++)
				for (int32 i = 0; i < 1; i++)
				{
					int32 LODIndex = 0;

					// for MMD?
					int32 MaxLODLevel = 1;
					FSkeletalMeshImportData smid;
					USkeletalMesh* NewMesh = nullptr;
					if (LODIndex == 0)
					{
						NewMesh = ImportSkeletalMesh(
							InParent,
							&pmxMeshInfoPtr,
							OutputName,
							Name,
							RF_Public | RF_Standalone | RF_MarkAsNative | RF_Transactional,
							FPaths::GetBaseFilename(Filename),
							&smid);
						NewObject = NewMesh;
					}

					// add self
					if (NewObject)
					{
						// MMD Extend asset
						CreateMMDExtendFromMMDModel(
							InParent,
							Cast<USkeletalMesh>(NewObject),
							&pmxMeshInfoPtr,
							Name);
					}

					// end phese
					if (NewObject)
					{
						TotalNumNodes++;
						NodeIndex++;
						FFormatNamedArguments Args;
						Args.Add(TEXT("NodeIndex"), NodeIndex);
						Args.Add(TEXT("ArrayLength"), 1); // SkelMeshArray.Num());
						GWarn->StatusUpdate(NodeIndex, 1, FText::Format(NSLOCTEXT("UnrealEd", "Importingf", "Importing ({NodeIndex} of {ArrayLength})"), Args));
					}

					// MarkPackageDirty();
				}

				// if total nodes we found is 0, we didn't find anything.
				if (TotalNumNodes == 0)
				{
					AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_NoMeshFoundOnRoot", "Could not find any valid mesh on the root hierarchy. If you have mesh in the sub hierarchy, please enable option of [Import Meshes In Bone Hierarchy] when import.")), "FFbxErrors::SkeletalMesh_NoMeshFoundOnRoot");
				}
#endif
			}
		}
		else
		{
			if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON)
			{
				AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_InvalidBone", "Failed to find any bone hierarchy. Try disabling the \"Import As Skeletal\" option to import as a rigid mesh. ")), "FFbxErrors::SkeletalMesh_InvalidBone");
			}
			else
			{
				AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_InvalidNode", "Could not find any node.")), "FFbxErrors::SkeletalMesh_InvalidNode");
			}
		}
	}

	if (NewObject == nullptr)
	{
		AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_NoObject", "Import failed.")), "FFbxErrors::Generic_ImportingNewObjectFailed");
		return false;
	}

	InParent->MarkPackageDirty();
	return true;
}

bool UPmxFactory::ImportPmxFromFile(FString file)
{
	auto Factory = NewObject<UPmxFactory>();
	return Factory->FImportPmxFromFile(file);
}
UObject* UPmxFactory::FactoryCreateBinary(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	const TCHAR* Type,
	const uint8*& Buffer,
	const uint8* BufferEnd,
	FFeedbackContext* Warn,
	bool& bOutOperationCanceled)
{
	FName OutputName = "";
	OutputName = FName(*FString::Printf(TEXT("SKM_%s"), *Name.ToString()));

	// MMD Default
	importAssetTypeMMD = E_MMD_TO_UE5_SKELETON;

	if (bOperationCanceled)
	{
		bOutOperationCanceled = true;

		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, nullptr);
		return nullptr;
	}

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPreImport.Broadcast(this, Class, InParent, Name, Type);

	UObject* NewObject = nullptr;

	FPmxImporter* PmxImporter = FPmxImporter::GetInstance();

	EPMXImportType ForcedImportType = PMXIT_StaticMesh;

	// For multiple files, use the same settings
	bDetectImportTypeOnImport = false;

	// judge MMD format(pmx or pmd)
	bool bIsPmxFormat = FString(Type).Equals(TEXT("pmx"), ESearchCase::IgnoreCase);
	// Load MMD Model From binary File
	MMD4UE5::PmxMeshInfo pmxMeshInfoPtr;
	pmxMaterialImportHelper.InitializeBaseValue(InParent);
	bool pmxImportResult = false;
	if (bIsPmxFormat)
	{
		// pmx ver
		pmxImportResult = pmxMeshInfoPtr.PMXLoaderBinary(Buffer, BufferEnd);
	}
	else
	{
		// pmd ver
		MMD4UE5::PmdMeshInfo PmdMeshInfo;
		if (PmdMeshInfo.PMDLoaderBinary(Buffer, BufferEnd))
		{
			// Up convert From PMD to PMX format gfor UE5
			pmxImportResult = PmdMeshInfo.ConvertToPmxFormat(&pmxMeshInfoPtr);
		}
	}
	if (!pmxImportResult)
	{
		// Log the error message and fail the import.
		Warn->Log(ELogVerbosity::Error, "PMX Import ERR...FLT");
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, nullptr);
		return nullptr;
	}
	else
	{
		// 读取模型后的警告文显示：注释栏
		FText TitleStr = FText::Format(LOCTEXT("ImportReadMe_Generic", "{0}"), FText::FromString("tilesss"));
		const FText Message = FText::Format(LOCTEXT("ImportReadMe_Generic_Msg",
												"Important!! \nReadMe Lisence \n modele Name:\n'{0}'\n \n Model Comment JP:\n'{1}'"),
			FText::FromString(pmxMeshInfoPtr.modelNameJP), FText::FromString(pmxMeshInfoPtr.modelCommentJP));
		// if (EAppReturnType::Ok != FMessageDialog::Open(EAppMsgType::OkCancel, Message))
		//{
		//	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, nullptr);
		//	return nullptr;
		// }
		TitleStr = FText::Format(LOCTEXT("ImportReadMe_Generic_Dbg", "{0} 制限事項"), FText::FromString("IVP5U Plugin"));
		const FText MessageDbg = FText(LOCTEXT("ImportReadMe_Generic_Dbg_Msg",
			"下一个导入选项的Slate仍在实施中。\n\
			导入对象只能生成Skeleton。\n\
			当前有效的参数是显示的项目有效。"));
		// FMessageDialog::Open(EAppMsgType::Ok, MessageDbg, &TitleStr);
	}

	// show Import Option Slate
	bool bImportAll = false;
	ImportUI->OriginalImportType = EPMXImportType::PMXIT_SkeletalMesh;
	ImportUI->bImportTextures = true;
	UE_LOG(LogMMD4UE5_PMXFactory, Log, TEXT("PMX Import: %s"), *(InParent->GetPathName()));

	PMXImportOptions* ImportOptions = GetImportOptions(
		PmxImporter,
		ImportUI,
		true, // bShowImportDialog,
		InParent->GetPathName(),
		bOperationCanceled,
		bImportAll,
		bIsPmxFormat,
		ForcedImportType);
	if (bImportAll)
	{
		// If the user chose to import all, we don't show the dialog again and use the same settings for each object until importing another set of files
		// bShowOption = false;
	}

	if (ImportOptions)
	{
		Warn->BeginSlowTask(NSLOCTEXT("PmxFactory", "BeginImportingPmxMeshTask", "Importing Pmx mesh"), true);
		{
			// For animation and static mesh we assume there is at lease one interesting node by default
			int32 InterestingNodeCount = 1;

			if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON)
			{
#ifdef DEBUG_MMD_PLUGIN_SKELETON

				InterestingNodeCount = 1; // test ? not Anime?

#endif
			}
			else if (importAssetTypeMMD == E_MMD_TO_UE5_STATICMESH)
			{
			}

			if (InterestingNodeCount > 1)
			{
				// the option only works when there are only one asset
				//				ImportOptions->bUsedAsFullName = false;
			}

			const FString Filename(UFactory::CurrentFilename);
			if (/*RootNodeToImport &&*/ InterestingNodeCount > 0)
			{
				int32 NodeIndex = 0;

				int32 ImportedMeshCount = 0;
				UStaticMesh* NewStaticMesh = nullptr;
				if (importAssetTypeMMD == E_MMD_TO_UE5_STATICMESH) // static mesh
				{
				}
				else if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON) // skeletal mesh
				{
#ifdef DEBUG_MMD_PLUGIN_SKELETON
					int32 TotalNumNodes = 0;
					// for (int32 i = 0; i < SkelMeshArray.Num(); i++)
					for (int32 i = 0; i < 1 /*SkelMeshArray.Num()*/; i++)
					{
						int32 LODIndex = 0;

						// for MMD?
						int32 MaxLODLevel = 1;
						FSkeletalMeshImportData smid;
						USkeletalMesh* NewMesh = nullptr;
						if (LODIndex == 0)
						{
							// UEnum* CompileModeEnum = GetStaticEnum <EObjectFlags>();

							UE_LOG(LogMMD4UE5_PMXFactory, Log, TEXT("PMX Import: %s"), *OutputName.ToString());

							NewMesh = ImportSkeletalMesh(
								InParent,
								&pmxMeshInfoPtr,
								OutputName,
								Name,
								Flags,
								// ImportUI->SkeletalMeshImportData,
								FPaths::GetBaseFilename(Filename),
								&smid);
							NewObject = NewMesh;
						}

						// add self
						if (NewObject)
						{
							// MMD Extend asset
							CreateMMDExtendFromMMDModel(
								InParent,
								Cast<USkeletalMesh>(NewObject),
								&pmxMeshInfoPtr,
								Name);
						}

						// end phese
						if (NewObject)
						{
							TotalNumNodes++;
							NodeIndex++;
							FFormatNamedArguments Args;
							Args.Add(TEXT("NodeIndex"), NodeIndex);
							Args.Add(TEXT("ArrayLength"), 1); // SkelMeshArray.Num());
							GWarn->StatusUpdate(NodeIndex, 1 /*SkelMeshArray.Num()*/, FText::Format(NSLOCTEXT("UnrealEd", "Importingf", "Importing ({NodeIndex} of {ArrayLength})"), Args));
						}
					}

					// if total nodes we found is 0, we didn't find anything.
					if (TotalNumNodes == 0)
					{
						AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_NoMeshFoundOnRoot", "Could not find any valid mesh on the root hierarchy. If you have mesh in the sub hierarchy, please enable option of [Import Meshes In Bone Hierarchy] when import.")), "FFbxErrors::SkeletalMesh_NoMeshFoundOnRoot");
					}
#endif
				}
			}
			else
			{
				if (importAssetTypeMMD == E_MMD_TO_UE5_SKELETON)
				{
					AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_InvalidBone", "Failed to find any bone hierarchy. Try disabling the \"Import As Skeletal\" option to import as a rigid mesh. ")), "FFbxErrors::SkeletalMesh_InvalidBone");
				}
				else
				{
					AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_InvalidNode", "Could not find any node.")), "FFbxErrors::SkeletalMesh_InvalidNode");
				}
			}
		}

		if (NewObject == nullptr)
		{
			AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FailedToImport_NoObject", "Import failed.")), "FFbxErrors::Generic_ImportingNewObjectFailed");
			Warn->Log(ELogVerbosity::Error, "PMX Import ERR [NewObject is nullptr]...FLT");
		}

		Warn->EndSlowTask();
	}
	else
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Log, TEXT("用户取消PMX导入"));
		bOutOperationCanceled = true; // 设置引擎级取消标誌
									  // 不显示错误对话框，直接返回
	}

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, NewObject);

	return NewObject;
}

USkeletalMesh* UPmxFactory::ImportSkeletalMesh(
	UObject* InParent,
	MMD4UE5::PmxMeshInfo* pmxMeshInfoPtr,
	const FName& Name,
	const FName& BaseName,
	EObjectFlags Flags,
	FString Filename,
	FSkeletalMeshImportData* OutData)
{
	bool bDiffPose;
	int32 SkelType = 0; // 0 for skeletal mesh, 1 for rigid mesh

	FString FolderPath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());
	FString NewPackagePath = FString::Printf(TEXT("%s/%s"), *FolderPath, *Name.ToString());
	UPackage* SkelMeshPackage = CreatePackage(*NewPackagePath);
	SkelMeshPackage->FullyLoad();
	{
		UObject* ExistingObject = FindObject<UObject>(SkelMeshPackage, *Name.ToString());
		USkeletalMesh* ExistingSkelMesh = Cast<USkeletalMesh>(ExistingObject);

		if (ExistingSkelMesh)
		{
			ExistingSkelMesh->PreEditChange(nullptr);
		}
		// if any other object exists, we can't import with this name
		else if (ExistingObject)
		{
			AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Warning, FText::Format(LOCTEXT("FbxSkeletaLMeshimport_OverlappingName", "Same name but different class: '{0}' already exists"), FText::FromString(ExistingObject->GetName()))), "FFbxErrors::Generic_SameNameAssetExists");
			return nullptr;
		}
	}

	USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(SkelMeshPackage, Name, Flags | RF_Public | RF_Standalone);
	SkeletalMesh->PreEditChange(nullptr);

	FSkeletalMeshImportData TempData;
	// Fill with data from buffer - contains the full .FBX file.
	FSkeletalMeshImportData* SkelMeshImportDataPtr = OutData ? OutData : &TempData;

	/*Import Bone Start*/
	bool bUseTime0AsRefPose = false;
	// Note: importing morph data causes additional passes through this function, so disable the warning dialogs
	// from popping up again on each additional pass.
	if (!ImportBone(pmxMeshInfoPtr, *SkelMeshImportDataPtr, bDiffPose, false, bUseTime0AsRefPose))
	{
		AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, LOCTEXT("FbxSkeletaLMeshimport_MultipleRootFound", "Multiple roots found")), "FFbxErrors::SkeletalMesh_MultipleRoots");
		// I can't delete object here since this is middle of import
		// but I can move to transient package, and GC will automatically collect it
		SkeletalMesh->ClearFlags(RF_Standalone);
		SkeletalMesh->Rename(nullptr, GetTransientPackage());
		UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("ImportBone_out"));
		return nullptr;
	}

	for (int32 MaterialIndex = 0; MaterialIndex < pmxMeshInfoPtr->materialList.Num(); ++MaterialIndex)
	{
		// Add an entry for each unique material
		SkeletalMeshImportData::FMaterial NewMaterial;
		SkelMeshImportDataPtr->Materials.Add(NewMaterial);
	}

	if (!FillSkelMeshImporterFromFbx(*SkelMeshImportDataPtr, pmxMeshInfoPtr, InParent))
	{
		// I can't delete object here since this is middle of import
		// but I can move to transient package, and GC will automatically collect it
		SkeletalMesh->ClearFlags(RF_Standalone);
		SkeletalMesh->Rename(nullptr, GetTransientPackage());
		UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("FillSkelMeshImporterFromFbx_out"));
		return nullptr;
	}
	else
	{
		SkelMeshImportDataPtr->PointToRawMap.AddUninitialized(SkelMeshImportDataPtr->Points.Num());
		for (int32 PointIdx = 0; PointIdx < SkelMeshImportDataPtr->Points.Num(); PointIdx++)
		{
			SkelMeshImportDataPtr->PointToRawMap[PointIdx] = PointIdx;
		}
	}

	// ==============================================================================
	// 【核心修復 1】：強制修補匯入資料的 UV 通道數與最大材質索引，否則建立 MeshDescription 必閃退！
	// ==============================================================================
	// Pass the number of texture coordinate sets to the LODModel.  Ensure there is at least one UV coord
	SkelMeshImportDataPtr->NumTexCoords = FMath::Max<uint32>(1, SkelMeshImportDataPtr->NumTexCoords);
	if (SkelMeshImportDataPtr->Materials.Num() > 0)
	{
		SkelMeshImportDataPtr->MaxMaterialIndex = SkelMeshImportDataPtr->Materials.Num() - 1;
	}

	// process materials from import data
	ProcessImportMeshMaterials(SkeletalMesh->GetMaterials(), *SkelMeshImportDataPtr);

	// process reference skeleton from import data
	int32 SkeletalDepth = 0;
	if (!ProcessImportMeshSkeleton(SkeletalMesh->GetSkeleton(), SkeletalMesh->GetRefSkeleton(), SkeletalDepth, *SkelMeshImportDataPtr))
	{
		SkeletalMesh->ClearFlags(RF_Standalone);
		SkeletalMesh->Rename(nullptr, GetTransientPackage());
		return nullptr;
	}

	// process bone influences from import data
	SkeletalMeshImportUtils::ProcessImportMeshInfluences(*SkelMeshImportDataPtr, L"MMDMeshName");

	SkeletalMesh->PreEditChange(nullptr);
	// Dirty the DDC Key for any imported Skeletal Mesh
	SkeletalMesh->InvalidateDeriveDataCacheGUID();

	// Setup LOD Info
	SkeletalMesh->SetNumSourceModels(0);
	FSkeletalMeshLODInfo& NewLODInfo = SkeletalMesh->AddLODInfo();
	NewLODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	NewLODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
	NewLODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	NewLODInfo.LODHysteresis = 0.02f;
	NewLODInfo.BuildSettings.bRecomputeNormals = false;
	NewLODInfo.BuildSettings.bRecomputeTangents = true;

	// Setup LOD Model
	FSkeletalMeshModel* ImportedResource = SkeletalMesh->GetImportedModel();
	if (ImportedResource->LODModels.Num() != 0)
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("ImportSkeletalMesh: LODModels was not empty (count=%d), clearing."), ImportedResource->LODModels.Num());
		ImportedResource->LODModels.Empty();
	}
	ImportedResource->LODModels.Add(new FSkeletalMeshLODModel());

	UE_LOG(LogMMD4UE5_PMXFactory, Log, TEXT("ImportSkeletalMesh: Added new LODModel. Total LOD count is now: %d. The only valid index should be 0."), ImportedResource->LODModels.Num());
	FSkeletalMeshLODModel& LODModel = ImportedResource->LODModels[0];

	// Create initial bounding box based on expanded version of reference pose for meshes without physics assets. Can be overridden by artist.
	FBox3f BoundingBox(SkelMeshImportDataPtr->Points.GetData(), SkelMeshImportDataPtr->Points.Num());
	FBox3f Temp = BoundingBox;
	FVector3f MidMesh = 0.5f * (Temp.Min + Temp.Max);
	BoundingBox.Min = Temp.Min + 1.0f * (Temp.Min - MidMesh);
	BoundingBox.Max = Temp.Max + 1.0f * (Temp.Max - MidMesh);
	// Tuck up the bottom as this rarely extends lower than a reference pose's (e.g. having its feet on the floor).
	// Maya has Y in the vertical, other packages have Z.
	// BEN const int32 CoordToTuck = bAssumeMayaCoordinates ? 1 : 2;
	// BEN BoundingBox.Min[CoordToTuck]	= Temp.Min[CoordToTuck] + 0.1f*(Temp.Min[CoordToTuck] - MidMesh[CoordToTuck]);
	BoundingBox.Min[2] = Temp.Min[2] + 0.1f * (Temp.Min[2] - MidMesh[2]);

	SkeletalMesh->SetImportedBounds(FBoxSphereBounds(FBox(BoundingBox)));

	// Store whether or not this mesh has vertex colors
	SkeletalMesh->SetHasVertexColors(SkelMeshImportDataPtr->bHasVertexColors);
	SkeletalMesh->SetVertexColorGuid(SkeletalMesh->GetHasVertexColors() ? FGuid::NewGuid() : FGuid());
	// Release the static mesh's resources.
	SkeletalMesh->ReleaseResources();

	// Flush the resource release commands to the rendering thread to ensure that the build doesn't occur while a resource is still
	// allocated, and potentially accessing the UStaticMesh.
	SkeletalMesh->ReleaseResourcesFence.Wait();

	LODModel.NumTexCoords = SkelMeshImportDataPtr->NumTexCoords;

	{
		// --- Create & bind Skeleton (must exist before IMeshBuilderModule call) ---
		USkeleton* Skeleton = nullptr;
		FString SkelObjectName = FString::Printf(TEXT("SK_%s"), *BaseName.ToString());
		Skeleton = CreateAsset<USkeleton>(InParent->GetName(), SkelObjectName, true);
		if (!Skeleton)
		{
			Skeleton = LoadObject<USkeleton>(InParent, *SkelObjectName, nullptr, LOAD_NoWarn | LOAD_Quiet);
			if (!Skeleton)
			{
				AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Error, FText::Format(LOCTEXT("FbxSkeletaLMeshimport_SkeletonRecreateError", "'{0}' already exists. It fails to recreate it."), FText::FromString(SkelObjectName))), "FFbxErrors::SkeletalMesh_SkeletonRecreateError");
				return SkeletalMesh;
			}
		}

		if (!Skeleton->MergeAllBonesToBoneTree(SkeletalMesh))
		{
			if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SkeletonFailed_BoneMerge", "FAILED TO MERGE BONES:\n\n This could happen if significant hierarchical change has been made\n - i.e. inserting bone between nodes\n Would you like to regenerate Skeleton from this mesh? \n\n ***WARNING: THIS WILL REQUIRE RECOMPRESS ALL ANIMATION DATA AND POTENTIALLY INVALIDATE***\n")))
			{
				if (Skeleton->RecreateBoneTree(SkeletalMesh))
				{
					FAssetNotifications::SkeletonNeedsToBeSaved(Skeleton);
				}
			}
		}
		SkeletalMesh->SetSkeleton(Skeleton);

		// --- Import morph targets (before MeshDescription is created) ---
		ImportFbxMorphTarget(*pmxMeshInfoPtr, SkeletalMesh, InParent, Filename, 0, *SkelMeshImportDataPtr);

		// --- Store the original PMX import data; SkelMeshImportDataPtr must not be modified after this ---
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		SkeletalMesh->SaveLODImportedData(0, *SkelMeshImportDataPtr);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// --- Convert to MeshDescription and commit ---
		FMeshDescription MeshDescription;
		if (SkelMeshImportDataPtr->GetMeshDescription(SkeletalMesh, &SkeletalMesh->GetLODInfo(0)->BuildSettings, MeshDescription))
		{
			SkeletalMesh->ModifyMeshDescription(0);
			SkeletalMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
			SkeletalMesh->CommitMeshDescription(0);
		}
		else
		{
			UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("Failed to generate MeshDescription from ImportData."));
			SkeletalMesh->MarkAsGarbage();
			return nullptr;
		}

		// --- Build render data via IMeshBuilderModule ---
		IMeshBuilderModule& MeshBuilderModule = IMeshBuilderModule::GetForRunningPlatform();
		FSkeletalMeshBuildParameters SkeletalMeshBuildParameters(SkeletalMesh, GetTargetPlatformManagerRef().GetRunningTargetPlatform(), 0, false);

		// AllocateResourceForRendering() must be called before BuildSkeletalMesh()
		SkeletalMesh->AllocateResourceForRendering();
		bool bBuildSuccess = MeshBuilderModule.BuildSkeletalMesh(*SkeletalMesh->GetResourceForRendering(), SkeletalMeshBuildParameters);
		SkeletalMesh->ReleaseResources();

		if (!bBuildSuccess)
		{
			UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("IMeshBuilderModule::BuildSkeletalMesh Failed."));
			SkeletalMesh->MarkAsGarbage();
			return nullptr;
		}

		// Initialize material slot mapping (1:1 with sections)
		const int32 NumSections = SkeletalMesh->GetImportedModel()->LODModels[0].Sections.Num();
		for (int32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
		{
			SkeletalMesh->GetLODInfo(0)->LODMaterialMap.Add(SectionIndex);
		}

		SkeletalMesh->GetAssetImportData()->Update(UFactory::CurrentFilename);
		SkeletalMesh->CalculateInvRefMatrices();
		// Note: no need to call SkeletalMesh->Build(), IMeshBuilderModule handles it

		SkeletalMesh->PostEditChange();
		SkeletalMesh->MarkPackageDirty();

		for (TObjectIterator<USkeletalMeshComponent> It; It; ++It)
		{
			USkeletalMeshComponent* SkelComp = *It;
			if (SkelComp->GetSkeletalMeshAsset() == SkeletalMesh)
			{
				FComponentReregisterContext ReregisterContext(SkelComp);
			}
		}
	}

	// ==============================================================================
	// 建立剛體與物理資產
	// ==============================================================================
	if (InParent != GetTransientPackage() && ImportUI->bCreatePhysicsAsset)
	{
		if (SkeletalMesh->GetPhysicsAsset() == nullptr)
		{
			FString PhysObjectName = FString::Printf(TEXT("PA_%s"), *BaseName.ToString());
			UPhysicsAsset* NewPhysicsAsset = CreateAsset<UPhysicsAsset>(InParent->GetName(), PhysObjectName, true);
			if (NewPhysicsAsset)
			{
				FPhysAssetCreateParams NewBodyData;
				NewBodyData.bDisableCollisionsByDefault = true;
				NewBodyData.MinBoneSize = 2;
				FText CreationErrorMessage;
				bool bSuccess = FPhysicsAssetUtils::CreateFromSkeletalMesh(NewPhysicsAsset, SkeletalMesh, NewBodyData, CreationErrorMessage);
				if (!bSuccess)
				{
					AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Warning, CreationErrorMessage), "FFbxErrors::SkeletalMesh_FailedToCreatePhyscisAsset");
					// delete the asset since we could not have create physics asset
					TArray<UObject*> ObjectsToDelete;
					ObjectsToDelete.Add(NewPhysicsAsset);
					ObjectTools::DeleteObjects(ObjectsToDelete, false);
				}
				else
				{
					int bdn = NewPhysicsAsset->SkeletalBodySetups.Num();
					TArray<PMX_RIGIDBODY> bodyRigids;
					bodyRigids.SetNum(bdn);
					for (int i = 0; i < bdn; i++)
					{
						TObjectPtr<USkeletalBodySetup>& bd = NewPhysicsAsset->SkeletalBodySetups[i];
						USkeletalBodySetup* pbd = bd.Get();
						FName bname = pbd->BoneName;
						TArray<PMX_RIGIDBODY> rbs = pmxMeshInfoPtr->findRigid(bname);

						if (rbs.Num() > 0)
						{
							bodyRigids[i] = rbs[0];
						}

						FKAggregateGeom& ag = pbd->AggGeom;
						ag.EmptyElements();
						if (rbs.Num() < 1)
						{
							FKBoxElem ke(0.5, 0.5, 1);
							ag.BoxElems.Add(ke);
							pbd->PhysicsType = PhysType_Kinematic;
							pbd->CollisionReponse = EBodyCollisionResponse::BodyCollision_Disabled;
							ke.SetContributeToMass(false);
							continue;
						}

						for (auto rb : rbs)
						{
							switch (rb.OpType)
							{
								case 0: // 追踪骨骼
									pbd->PhysicsType = PhysType_Kinematic;
									break;
								case 1: // 物理演算
								case 2: // 物理+骨骼位置统一
									pbd->PhysicsType = PhysType_Simulated;
									break;
								default:
									pbd->PhysicsType = PhysType_Kinematic;
									break;
							}
							FTransform ft;
							ft.SetLocation((FVector)rb.Position);
							ft.SetRotation(rb.Quat);

							if (rb.ShapeType == 0)
							{
								FKSphereElem ke(rb.Size.X);
								ke.SetTransform(ft);
								ke.SetContributeToMass(true);
								ag.SphereElems.Add(ke);
							}
							else if (rb.ShapeType == 1)
							{
								FKBoxElem ke(rb.Size.X, rb.Size.Z, rb.Size.Y);
								ke.SetTransform(ft);
								ke.SetContributeToMass(true);
								ag.BoxElems.Add(ke);
							}
							else if (rb.ShapeType == 2)
							{
								FKSphylElem ke(rb.Size.X, rb.Size.Y);
								ke.SetTransform(ft);
								ke.SetContributeToMass(true);
								ag.SphylElems.Add(ke);
							}
						}
					}

					for (int i = 0; i < bdn; i++)
					{
						for (int j = i + 1; j < bdn; j++)
						{
							const PMX_RIGIDBODY& rbi = bodyRigids[i];
							const PMX_RIGIDBODY& rbj = bodyRigids[j];

							uint16 groupI = (uint16)(1 << rbi.RigidBodyGroupIndex);
							uint16 groupJ = (uint16)(1 << rbj.RigidBodyGroupIndex);

							bool iBlocksJ = (rbi.RigidBodyGroupTarget & groupJ) != 0;
							bool jBlocksI = (rbj.RigidBodyGroupTarget & groupI) != 0;

							if (!iBlocksJ && !jBlocksI)
							{
								NewPhysicsAsset->EnableCollision(i, j);
							}
						}
					}

					int csn = NewPhysicsAsset->ConstraintSetup.Num();
					for (int i = 0; i < csn; i++)
					{
						FConstraintInstance& cs = NewPhysicsAsset->ConstraintSetup[i]->DefaultInstance;
						// cs.ProfileInstance.AngularDrive.AngularDriveMode = EAngularDriveMode::TwistAndSwing;
						cs.ProfileInstance.ConeLimit.Swing1Motion = EAngularConstraintMotion::ACM_Limited;
						cs.ProfileInstance.ConeLimit.Swing2Motion = EAngularConstraintMotion::ACM_Limited;
						cs.ProfileInstance.TwistLimit.TwistMotion = EAngularConstraintMotion::ACM_Limited;
						cs.ProfileInstance.TwistLimit.TwistLimitDegrees = 0;
						cs.ProfileInstance.ConeLimit.Swing1LimitDegrees = 25;
						cs.ProfileInstance.ConeLimit.Swing2LimitDegrees = 5;
						cs.SetDisableCollision(true);
					}
				}
			}
		}
	}
	return SkeletalMesh;
}

UMMDExtendAsset* UPmxFactory::CreateMMDExtendFromMMDModel(
	UObject* InParent,
	USkeletalMesh* SkeletalMesh, // issue #2: fix param use skeleton mesh
	MMD4UE5::PmxMeshInfo* PmxMeshInfo,
	const FName& BaseName)
{
	UMMDExtendAsset* NewMMDExtendAsset = nullptr;
	if (SkeletalMesh->GetSkeleton() == nullptr)
	{
		UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("CreateMMDExtendFromMMDModel: SkeletalMesh has no Skeleton, aborting."));
		return nullptr;
	}

	// issue #2 : Fix MMDExtend IK Index
	const FReferenceSkeleton ReferenceSkeleton = SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();

	// MMD Extend asset

	// TBD::アセット生成関数で既存アセット時の判断ができていないと思われる。
	// 場合によってはVMDFactoryのアセット生成処理を元に再設計すること
	FString ObjectName = FString::Printf(TEXT("MMDExtend_%s"), *BaseName.ToString());
	NewMMDExtendAsset = CreateAsset<UMMDExtendAsset>(InParent->GetName(), ObjectName, true);
	if (!NewMMDExtendAsset)
	{
		// same object exists, try to see if it's asset, if so, load
		NewMMDExtendAsset = LoadObject<UMMDExtendAsset>(InParent, *ObjectName, nullptr, LOAD_NoWarn | LOAD_Quiet);

		if (!NewMMDExtendAsset)
		{
			AddTokenizedErrorMessage(FTokenizedMessage::Create(EMessageSeverity::Warning, FText::Format(LOCTEXT("CouldNotCreateMMDExtendAsset", "Could not create MMD Extend Asset ('{0}') for '{1}'"), FText::FromString(ObjectName), FText::FromString(BaseName.ToString()))), "FFbxErrors::SkeletalMesh_FailedToCreatePhyscisAsset");
		}
		else
		{
			NewMMDExtendAsset->IkInfoList.Empty();
		}
	}

	// create asset info
	if (NewMMDExtendAsset)
	{
		if (NewMMDExtendAsset->IkInfoList.Num() > 0)
		{
			NewMMDExtendAsset->IkInfoList.Empty();
		}
		// create IK
		// mapping
		NewMMDExtendAsset->ModelName = PmxMeshInfo->modelNameJP;
		NewMMDExtendAsset->ModelComment = FText::FromString(PmxMeshInfo->modelCommentJP);

		for (int boneIdx = 0; boneIdx < PmxMeshInfo->boneList.Num(); ++boneIdx)
		{
			// check IK bone
			if (PmxMeshInfo->boneList[boneIdx].Flag_IK)
			{
				MMD4UE5::PMX_IK* tempPmxIKPtr = &PmxMeshInfo->boneList[boneIdx].IKInfo;
				FMMD_IKInfo addMMDIkInfo;

				addMMDIkInfo.LoopNum = tempPmxIKPtr->LoopNum;
				// set limit rot[rad]
				addMMDIkInfo.RotLimit = tempPmxIKPtr->RotLimit;
				// this bone
				addMMDIkInfo.IKBoneName = FName(*PmxMeshInfo->boneList[boneIdx].Name);
				// issue #2: Fix IK bone index
				// this bone(ik-bone) index, from skeleton.
				addMMDIkInfo.IKBoneIndex = ReferenceSkeleton.FindBoneIndex(addMMDIkInfo.IKBoneName);
				// ik target
				addMMDIkInfo.TargetBoneName = FName(*PmxMeshInfo->boneList[tempPmxIKPtr->TargetBoneIndex].Name);
				// issue #2: Fix Target Bone Index
				// target bone(ik-target bone) index, from skeleton.
				addMMDIkInfo.TargetBoneIndex = ReferenceSkeleton.FindBoneIndex(addMMDIkInfo.TargetBoneName);
				// set sub ik
				addMMDIkInfo.ikLinkList.AddZeroed(tempPmxIKPtr->LinkNum);
				for (int ikInfoID = 0; ikInfoID < tempPmxIKPtr->LinkNum; ++ikInfoID)
				{
					// link bone index
					addMMDIkInfo.ikLinkList[ikInfoID].BoneName = FName(*PmxMeshInfo->boneList[tempPmxIKPtr->Link[ikInfoID].BoneIndex].Name);
					// issue #2: Fix link bone index
					// link bone index from skeleton.
					addMMDIkInfo.ikLinkList[ikInfoID].BoneIndex = ReferenceSkeleton.FindBoneIndex(addMMDIkInfo.ikLinkList[ikInfoID].BoneName);
					// limit flag
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockFlag = tempPmxIKPtr->Link[ikInfoID].RotLockFlag;
					// min
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMin.X = tempPmxIKPtr->Link[ikInfoID].RotLockMin[0];
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMin.Y = tempPmxIKPtr->Link[ikInfoID].RotLockMin[1];
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMin.Z = tempPmxIKPtr->Link[ikInfoID].RotLockMin[2];
					// max
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMax.X = tempPmxIKPtr->Link[ikInfoID].RotLockMax[0];
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMax.Y = tempPmxIKPtr->Link[ikInfoID].RotLockMax[1];
					addMMDIkInfo.ikLinkList[ikInfoID].RotLockMax.Z = tempPmxIKPtr->Link[ikInfoID].RotLockMax[2];
				}
				// add
				NewMMDExtendAsset->IkInfoList.Add(addMMDIkInfo);
			}
		}
		NewMMDExtendAsset->MarkPackageDirty();
	}
	return NewMMDExtendAsset;
}

#undef LOCTEXT_NAMESPACE
