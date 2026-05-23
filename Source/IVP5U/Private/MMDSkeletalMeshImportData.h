// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "Engine/SkeletalMesh.h"
#include "MMDMeshImportData.h"
#include "MMDSkeletalMeshImportData.generated.h"

/**
 * Import data and options used when importing a static mesh from MMD
 */
UCLASS()
class UMMDSkeletalMeshImportData : public UMMDMeshImportData
{
	GENERATED_UCLASS_BODY()

	/** Gets or creates MMD import data for the specified skeletal mesh */
	static UMMDSkeletalMeshImportData* GetImportDataForSkeletalMesh(USkeletalMesh* SkeletalMesh, UMMDSkeletalMeshImportData* TemplateForCreation);

	virtual bool CanEditChange(const FProperty* InProperty) const override;
};
