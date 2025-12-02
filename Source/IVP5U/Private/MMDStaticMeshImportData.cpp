// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "MMDStaticMeshImportData.h"
#include "IVP5UPrivatePCH.h"
#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"

UMMDStaticMeshImportData::UMMDStaticMeshImportData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StaticMeshLODGroup = NAME_None;
	bRemoveDegenerates = true;
	bGenerateLightmapUVs = true;
	bOneConvexHullPerUCX = true;
	bAutoGenerateCollision = true;
	VertexOverrideColor = FColor(255, 255, 255, 255);
}

UMMDStaticMeshImportData* UMMDStaticMeshImportData::GetImportDataForStaticMesh(UStaticMesh* StaticMesh, UMMDStaticMeshImportData* TemplateForCreation)
{
	check(StaticMesh);

	UMMDStaticMeshImportData* ImportData = Cast<UMMDStaticMeshImportData>(StaticMesh->GetAssetImportData());
	if (!ImportData)
	{
		ImportData = NewObject<UMMDStaticMeshImportData>(StaticMesh, NAME_None, RF_NoFlags, TemplateForCreation);

		// Try to preserve the source file path if possible
		if (StaticMesh->GetAssetImportData() != NULL)
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