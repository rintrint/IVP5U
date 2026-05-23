// Copyright 2015-2026 IVP5U contributors

/*=============================================================================
Main implementation of FFbxImporter : import FBX data to Unreal
=============================================================================*/

#include "CoreMinimal.h"

#include "Factories.h"
#include "Engine.h"
#include "ImportUtils/SkelImport.h"
#include "PmxImporter.h"
#include "PmxFactory.h"
#include "PmxOptionWindow.h"
#include "MainFrame.h"
#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"

#include "MMDSkeletalMeshImportData.h"
#include "MMDStaticMeshImportData.h"

#include "IVP5USettings.h"

// DEFINE_LOG_CATEGORY(LogPmx);

#define LOCTEXT_NAMESPACE "PmxMainImport"

// TSharedPtr<PmxMeshInfo> PmxMeshInfo::StaticInstance;

PMXImportOptions* GetImportOptions(
	class FPmxImporter* PmxImporter,
	UPmxImportUI* ImportUI,
	bool bShowOptionDialog,
	const FString& FullPath,
	bool& bOutOperationCanceled,
	bool& bOutImportAll,
	bool bForceImportType,
	EPMXImportType ImportType)
{
	bOutOperationCanceled = false;
	if (bShowOptionDialog)
	{
		bOutImportAll = false;

		PMXImportOptions* ImportOptions = PmxImporter->GetImportOptions();
		// if Skeleton was set by outside, please make sure copy back to UI
		if (ImportOptions->SkeletonForAnimation)
		{
			ImportUI->Skeleton = ImportOptions->SkeletonForAnimation;
		}
		else
		{
			ImportUI->Skeleton = nullptr;
		}

		if (ImportOptions->PhysicsAsset)
		{
			ImportUI->PhysicsAsset = ImportOptions->PhysicsAsset;
		}
		else
		{
			ImportUI->PhysicsAsset = nullptr;
		}
		if (bForceImportType)
		{
			ImportUI->MeshTypeToImport = ImportType;
			ImportUI->OriginalImportType = ImportType;
		}

		// last select asset ref
		if (ImportOptions->MmdExtendAsset)
		{
			ImportUI->MmdExtendAsset = ImportOptions->MmdExtendAsset;
		}
		else
		{
			ImportUI->MmdExtendAsset = nullptr;
		}
		if (ImportOptions->MMD2UE5NameTableRow)
		{
			ImportUI->MMD2UE5NameTableRow = ImportOptions->MMD2UE5NameTableRow;
		}
		else
		{
			ImportUI->MMD2UE5NameTableRow = nullptr;
		}
		if (ImportOptions->AnimSequenceAsset)
		{
			ImportUI->AnimSequenceAsset = ImportOptions->AnimSequenceAsset;
		}
		else
		{
			ImportUI->AnimSequenceAsset = nullptr;
		}

		TSharedPtr<SWindow> ParentWindow;

		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		TSharedRef<SWindow> Window = SNew(SWindow)
										 //.Title(NSLOCTEXT("UnrealEd", "FBXImportOpionsTitle", "FBX Import Options"))
										 .Title(NSLOCTEXT("IVP5U", "MMDImportOpionsTitle", "MMD Import Options"))
										 .SizingRule(ESizingRule::Autosized);

		TSharedPtr<SPmxOptionWindow> PmxOptionWindow;
		Window->SetContent(
			SAssignNew(PmxOptionWindow, SPmxOptionWindow)
				.ImportUI(ImportUI)
				.WidgetWindow(Window)
				.FullPath(FText::FromString(FullPath))
				.ForcedImportType(bForceImportType ? TOptional<EPMXImportType>(ImportType) : TOptional<EPMXImportType>()));

		// @todo: we can make this slow as showing progress bar later
		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		ImportUI->SaveConfig();

		if (ImportUI->StaticMeshImportData)
		{
			ImportUI->StaticMeshImportData->SaveConfig();
		}

		if (ImportUI->SkeletalMeshImportData)
		{
			ImportUI->SkeletalMeshImportData->SaveConfig();
		}
		if (PmxOptionWindow->ShouldImport())
		{
			bOutImportAll = PmxOptionWindow->ShouldImportAll();

			// open dialog
			// see if it's canceled
			ApplyImportUIToImportOptions(ImportUI, *ImportOptions);

			return ImportOptions;
		}
		else
		{
			bOutOperationCanceled = true;
		}
	}
	else if (GIsAutomationTesting)
	{
		// Automation tests set ImportUI settings directly.  Just copy them over
		PMXImportOptions* ImportOptions = PmxImporter->GetImportOptions();
		ApplyImportUIToImportOptions(ImportUI, *ImportOptions);
		return ImportOptions;
	}
	else
	{
		return PmxImporter->GetImportOptions();
	}
	return nullptr;
}

void ApplyImportUIToImportOptions(
	UPmxImportUI* ImportUI,
	PMXImportOptions& InOutImportOptions)
{
	check(ImportUI);

	InOutImportOptions.bImportTextures = ImportUI->bImportTextures;
	InOutImportOptions.bCreateMaterialInstanceMode = ImportUI->bCreateMaterialInstanceMode;
	InOutImportOptions.bUnlitMaterials = ImportUI->bUnlitMaterials;
	InOutImportOptions.bUsedAsFullName = ImportUI->bOverrideFullName;
	InOutImportOptions.bConvertScene = ImportUI->bConvertScene;
	InOutImportOptions.bImportAnimations = ImportUI->bImportAnimations;
	InOutImportOptions.SkeletonForAnimation = ImportUI->Skeleton;

	if (ImportUI->MeshTypeToImport == PMXIT_StaticMesh)
	{
		UMMDStaticMeshImportData* StaticMeshData = ImportUI->StaticMeshImportData;
		InOutImportOptions.ImportTranslation = StaticMeshData->ImportTranslation;
		InOutImportOptions.ImportRotation = StaticMeshData->ImportRotation;
		InOutImportOptions.ImportUniformScale = StaticMeshData->ImportUniformScale;
	}
	else if (ImportUI->MeshTypeToImport == PMXIT_SkeletalMesh)
	{
		UMMDSkeletalMeshImportData* SkeletalMeshData = ImportUI->SkeletalMeshImportData;
		InOutImportOptions.ImportTranslation = SkeletalMeshData->ImportTranslation;
		InOutImportOptions.ImportRotation = SkeletalMeshData->ImportRotation;
		InOutImportOptions.ImportUniformScale = SkeletalMeshData->ImportUniformScale;
	}
	// only re-sample if they don't want to use default sample rate
	InOutImportOptions.bResample = ImportUI->bUseDefaultSampleRate == false;
	InOutImportOptions.bUpdateSkeletonReferencePose = ImportUI->SkeletalMeshImportData->bUpdateSkeletonReferencePose;
	InOutImportOptions.bImportRigidMesh = ImportUI->OriginalImportType == PMXIT_StaticMesh && ImportUI->MeshTypeToImport == PMXIT_SkeletalMesh;
	InOutImportOptions.bUseT0AsRefPose = ImportUI->SkeletalMeshImportData->bUseT0AsRefPose;
	InOutImportOptions.bPreserveSmoothingGroups = ImportUI->SkeletalMeshImportData->bPreserveSmoothingGroups;
	InOutImportOptions.bKeepOverlappingVertices = ImportUI->SkeletalMeshImportData->bKeepOverlappingVertices;
	InOutImportOptions.bCombineToSingle = ImportUI->bCombineMeshes;
	InOutImportOptions.VertexColorImportOption = ImportUI->StaticMeshImportData->VertexColorImportOption;
	InOutImportOptions.VertexOverrideColor = ImportUI->StaticMeshImportData->VertexOverrideColor;
	InOutImportOptions.bRemoveDegenerates = ImportUI->StaticMeshImportData->bRemoveDegenerates;
	InOutImportOptions.bGenerateLightmapUVs = ImportUI->StaticMeshImportData->bGenerateLightmapUVs;
	InOutImportOptions.bOneConvexHullPerUCX = ImportUI->StaticMeshImportData->bOneConvexHullPerUCX;
	InOutImportOptions.bAutoGenerateCollision = ImportUI->StaticMeshImportData->bAutoGenerateCollision;
	InOutImportOptions.StaticMeshLODGroup = ImportUI->StaticMeshImportData->StaticMeshLODGroup;
	InOutImportOptions.bImportMeshesInBoneHierarchy = ImportUI->SkeletalMeshImportData->bImportMeshesInBoneHierarchy;
	InOutImportOptions.bCreatePhysicsAsset = ImportUI->bCreatePhysicsAsset;
	InOutImportOptions.PhysicsAsset = ImportUI->PhysicsAsset;
	// add self
	InOutImportOptions.AnimSequenceAsset = ImportUI->AnimSequenceAsset;
	InOutImportOptions.MMD2UE5NameTableRow = ImportUI->MMD2UE5NameTableRow;
	InOutImportOptions.MmdExtendAsset = ImportUI->MmdExtendAsset;
}

TSharedPtr<FPmxImporter> FPmxImporter::StaticInstance;

FPmxImporter::FPmxImporter()
	: ImportOptions(new PMXImportOptions())
{
}

FPmxImporter::~FPmxImporter()
{
	CleanUp();
}

FPmxImporter* FPmxImporter::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FPmxImporter());
	}
	return StaticInstance.Get();
}

void FPmxImporter::DeleteInstance()
{
	StaticInstance.Reset();
}

void FPmxImporter::CleanUp()
{
#if 0
	ClearTokenizedErrorMessages();
	ReleaseScene();

	delete GeometryConverter;
	GeometryConverter = nullptr;
#endif
	delete ImportOptions;
	ImportOptions = nullptr;
#if 0
	if (SdkManager)
	{
		SdkManager->Destroy();
	}
	SdkManager = nullptr;
	Logger = nullptr;
#endif
}

PMXImportOptions* FPmxImporter::GetImportOptions() const
{
	return ImportOptions;
}

UPmxImportUI::UPmxImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCombineMeshes = true;

	const UIVP5USettings* Settings = GetDefault<UIVP5USettings>();
	bCreatePhysicsAsset = Settings->bCreatePhysicsAsset;
	bCreateMaterialInstanceMode = Settings->bCreateMaterialInstanceMode;
	bUnlitMaterials = Settings->bUnlitMaterials;

	StaticMeshImportData = CreateDefaultSubobject<UMMDStaticMeshImportData>(TEXT("StaticMeshImportData"));
	SkeletalMeshImportData = CreateDefaultSubobject<UMMDSkeletalMeshImportData>(TEXT("SkeletalMeshImportData"));
}

#undef LOCTEXT_NAMESPACE
