// Copyright 2015-2026 IVP5U contributors

#include "MMDStaticMeshImportData.h"
#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"

UMMDStaticMeshImportData* UMMDStaticMeshImportData::GetImportDataForStaticMesh(UStaticMesh* StaticMesh, UMMDStaticMeshImportData* TemplateForCreation)
{
	check(StaticMesh);

	UMMDStaticMeshImportData* ImportData = Cast<UMMDStaticMeshImportData>(StaticMesh->GetAssetImportData());
	if (!ImportData)
	{
		ImportData = NewObject<UMMDStaticMeshImportData>(StaticMesh, NAME_None, RF_NoFlags, TemplateForCreation);

		// Try to preserve the source file path if possible
		if (StaticMesh->GetAssetImportData() != nullptr)
		{
			ImportData->SourceData = StaticMesh->GetAssetImportData()->SourceData;
		}

		StaticMesh->SetAssetImportData(ImportData);
	}

	return ImportData;
}

bool UMMDStaticMeshImportData::CanEditChange(const FProperty* InProperty) const
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
