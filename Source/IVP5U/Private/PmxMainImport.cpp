// Copyright 2015-2026 IVP5U contributors

/*=============================================================================
Main implementation of FPmxImporter : import PMX data to Unreal
=============================================================================*/

#include "CoreMinimal.h"

#include "Engine.h"
#include "PmxImporter.h"
#include "PmxFactory.h"
#include "PmxOptionWindow.h"
#include "MainFrame.h"

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
	bool& bOutImportAll)
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
										 .Title(NSLOCTEXT("IVP5U", "MMDImportOpionsTitle", "MMD Import Options"))
										 .SizingRule(ESizingRule::Autosized);

		TSharedPtr<SPmxOptionWindow> PmxOptionWindow;
		Window->SetContent(
			SAssignNew(PmxOptionWindow, SPmxOptionWindow)
				.ImportUI(ImportUI)
				.WidgetWindow(Window)
				.FullPath(FText::FromString(FullPath)));

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

	InOutImportOptions.SkeletonForAnimation = ImportUI->Skeleton;
	InOutImportOptions.PhysicsAsset = ImportUI->PhysicsAsset;
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
	delete ImportOptions;
	ImportOptions = nullptr;
}

PMXImportOptions* FPmxImporter::GetImportOptions() const
{
	return ImportOptions;
}

UPmxImportUI::UPmxImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const UIVP5USettings* Settings = GetDefault<UIVP5USettings>();
	bCreatePhysicsAsset = Settings->bCreatePhysicsAsset;
	bCreateMaterialInstanceMode = Settings->bCreateMaterialInstanceMode;
	bUnlitMaterials = Settings->bUnlitMaterials;

	StaticMeshImportData = CreateDefaultSubobject<UMMDStaticMeshImportData>(TEXT("StaticMeshImportData"));
	SkeletalMeshImportData = CreateDefaultSubobject<UMMDSkeletalMeshImportData>(TEXT("SkeletalMeshImportData"));
}

#undef LOCTEXT_NAMESPACE
