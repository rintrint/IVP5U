// Copyright 2015-2026 IVP5U contributors

#include "MMDSkeletalMeshImportData.h"
#include "CoreMinimal.h"

UMMDSkeletalMeshImportData::UMMDSkeletalMeshImportData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bImportMeshesInBoneHierarchy(true)
{
}

UMMDSkeletalMeshImportData* UMMDSkeletalMeshImportData::GetImportDataForSkeletalMesh(USkeletalMesh* SkeletalMesh, UMMDSkeletalMeshImportData* TemplateForCreation)
{
	check(SkeletalMesh);

	UMMDSkeletalMeshImportData* ImportData = Cast<UMMDSkeletalMeshImportData>(SkeletalMesh->GetAssetImportData());
	if (!ImportData)
	{
		ImportData = NewObject<UMMDSkeletalMeshImportData>(SkeletalMesh, NAME_None, RF_NoFlags, TemplateForCreation);

		// Try to preserve the source file path if possible
		if (SkeletalMesh->GetAssetImportData() != nullptr)
		{
			ImportData->SourceData = SkeletalMesh->GetAssetImportData()->SourceData;
		}

		SkeletalMesh->SetAssetImportData(ImportData);
	}

	return ImportData;
}

bool UMMDSkeletalMeshImportData::CanEditChange(const FProperty* InProperty) const
{
	bool bMutable = Super::CanEditChange(InProperty);
	UObject* Outer = GetOuter();
	if (Outer && bMutable)
	{
		// Let the MMDImportUi object handle the editability of our properties
		bMutable = Outer->CanEditChange(InProperty);
	}
	return bMutable;
}
