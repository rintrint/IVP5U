// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "EditorFramework/AssetImportData.h"
#include "MMDMeshImportData.generated.h"

/**
 * Import data and options used when importing any mesh from MMD
 */
UCLASS(config = EditorUserSettings, configdonotcheckdefaults, abstract)
class UMMDMeshImportData : public UAssetImportData // UMMDAssetImportData
{
	GENERATED_BODY()

public:
	virtual bool CanEditChange(const FProperty* InProperty) const override;
};
