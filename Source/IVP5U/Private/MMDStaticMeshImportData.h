// Copyright 2015-2026 IVP5U contributors

/**
 * Import data and options used when importing a static mesh from MMD
 */

#pragma once

#include "Engine/StaticMesh.h"
#include "MMDMeshImportData.h"
#include "MMDStaticMeshImportData.generated.h"

UCLASS(config = EditorUserSettings, AutoExpandCategories = (Options), MinimalAPI)
class UMMDStaticMeshImportData : public UMMDMeshImportData
{
	GENERATED_BODY()

	/** Gets or creates MMD import data for the specified static mesh */
	static UMMDStaticMeshImportData* GetImportDataForStaticMesh(UStaticMesh* StaticMesh, UMMDStaticMeshImportData* TemplateForCreation);

	virtual bool CanEditChange(const FProperty* InProperty) const override;
};
