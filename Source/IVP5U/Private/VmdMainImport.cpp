// Copyright 2015-2026 IVP5U contributors

/*=============================================================================
Main implementation of FVmdImporter : import VMD data to Unreal
=============================================================================*/

#include "CoreMinimal.h"

#include "Engine.h"
#include "VmdImporter.h"
#include "VmdOptionWindow.h"
#include "MainFrame.h"

// DEFINE_LOG_CATEGORY(LogPmx);

#define LOCTEXT_NAMESPACE "VmdMainImport"

// TSharedPtr<PmxMeshInfo> PmxMeshInfo::StaticInstance;

VMDImportOptions* GetVMDImportOptions(
	class FVmdImporter* VmdImporter,
	UVmdImportUI* ImportUI,
	bool bShowOptionDialog,
	const FString& FullPath,
	bool& bOutOperationCanceled,
	bool& bOutImportAll)
{
	bOutOperationCanceled = false;

	if (bShowOptionDialog)
	{
		bOutImportAll = false;

		VMDImportOptions* ImportOptions = VmdImporter->GetImportOptions();
		// if Skeleton was set by outside, please make sure copy back to UI
		if (ImportOptions->SkeletonForAnimation)
		{
			ImportUI->Skeleton = ImportOptions->SkeletonForAnimation;
		}
		else
		{
			ImportUI->Skeleton = nullptr;
		}

		if (ImportOptions->SkeletalMeshForAnimation)
		{
			ImportUI->SkeletonMesh = ImportOptions->SkeletalMeshForAnimation;
		}
		else
		{
			ImportUI->SkeletonMesh = nullptr;
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
		ImportUI->ImportUniformScale = ImportOptions->ImportUniformScale;

		TSharedPtr<SWindow> ParentWindow;

		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		TSharedRef<SWindow> Window = SNew(SWindow)
										 .Title(NSLOCTEXT("IVP5U", "VMDImportOpionsTitle", "VMD Import Options"))
										 .SizingRule(ESizingRule::Autosized);

		TSharedPtr<SVmdOptionWindow> VmdOptionWindow;
		Window->SetContent(
			SAssignNew(VmdOptionWindow, SVmdOptionWindow)
				.ImportUI(ImportUI)
				.WidgetWindow(Window)
				.FullPath(FText::FromString(FullPath)));

		// @todo: we can make this slow as showing progress bar later
		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		ImportUI->SaveConfig();

		if (VmdOptionWindow->ShouldImport())
		{
			bOutImportAll = VmdOptionWindow->ShouldImportAll();

			// open dialog
			// see if it's canceled
			ApplyVMDImportUIToImportOptions(ImportUI, *ImportOptions);

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
		VMDImportOptions* ImportOptions = VmdImporter->GetImportOptions();
		ApplyVMDImportUIToImportOptions(ImportUI, *ImportOptions);
		return ImportOptions;
	}
	else
	{
		return VmdImporter->GetImportOptions();
	}
	return nullptr;
}

void ApplyVMDImportUIToImportOptions(
	UVmdImportUI* ImportUI,
	VMDImportOptions& InOutImportOptions)
{
	check(ImportUI);

	InOutImportOptions.SkeletonForAnimation = ImportUI->Skeleton;
	InOutImportOptions.SkeletalMeshForAnimation = ImportUI->SkeletonMesh;

	// add self
	InOutImportOptions.AnimSequenceAsset = ImportUI->AnimSequenceAsset;
	InOutImportOptions.MMD2UE5NameTableRow = ImportUI->MMD2UE5NameTableRow;
	InOutImportOptions.MmdExtendAsset = ImportUI->MmdExtendAsset;
	InOutImportOptions.ImportUniformScale = ImportUI->ImportUniformScale;
}

TSharedPtr<FVmdImporter> FVmdImporter::StaticInstance;

FVmdImporter::FVmdImporter()
	: ImportOptions(new VMDImportOptions())
{
}

FVmdImporter::~FVmdImporter()
{
	CleanUp();
}

FVmdImporter* FVmdImporter::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FVmdImporter());
	}
	return StaticInstance.Get();
}

void FVmdImporter::DeleteInstance()
{
	StaticInstance.Reset();
}

void FVmdImporter::CleanUp()
{
	delete ImportOptions;
	ImportOptions = nullptr;
}

VMDImportOptions* FVmdImporter::GetImportOptions() const
{
	return ImportOptions;
}

UVmdImportUI::UVmdImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ImportUniformScale = 0.08f;
}

#undef LOCTEXT_NAMESPACE
