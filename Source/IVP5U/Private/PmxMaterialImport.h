// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "Engine.h"
#include "Factories/Factory.h"
#include "PmxImporter.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCategoryPMXMaterialImport, Log, All)

/** 复制源材质索引*/
enum EDuplicateBaseMatTypeIndex
{
	E_DupBaseMat_Typ_Lit = 0,
	E_DupBaseMat_Typ_Lit_AutoLuminous,
	E_DupBaseMat_Typ_UnLit,
	E_DupBaseMat_Typ_UnLit_AutoLuminous,
	E_DupBaseMat_Typ_Max,
};

class UPmxMaterialImport
{
	/*
	 *复制导入模型时生成的材质的资源路径
	 *※将来如果能拥有PJ固有的设定路径的话
	 */
#define D_IVP5U_MMD_Base_Path_Lit "/IVP5U/Material/M_MMD_Base_Lit.M_MMD_Base_Lit"
#define D_IVP5U_MMD_Base_Path_Lit_AutoLuminous "/IVP5U/Material/M_MMD_Base_Lit_AutoLuminous.M_MMD_Base_Lit_AutoLuminous"
#define D_IVP5U_MMD_Base_Path_UnLit "/IVP5U/Material/M_MMD_Base_UnLit.M_MMD_Base_UnLit"
#define D_IVP5U_MMD_Base_Path_UnLit_AutoLuminous "/IVP5U/Material/M_MMD_Base_UnLit_AutoLuminous.M_MMD_Base_UnLit_AutoLuminous"

	/*
	 * Material Inst用Parameter Name
	 */
	/* Texture Base */
#define D_IVP5U_MatInst_Name_BaseTexture "TextureBase"
	/* is Texture Enabled Flag */
#define D_IVP5U_MatInst_Name_isTextureEnabled "isTextureEnabled"
	/* Texture Base */
#define D_IVP5U_MatInst_Name_Toon "TextureToon"

	/* DiffuseColor*/
#define D_IVP5U_MatInst_Name_DiffuseColor "DiffuseColor"
	/* SpecularPower */
#define D_IVP5U_MatInst_Name_SpecularPower "SpecularPower"
	/* AmbientColor */
#define D_IVP5U_MatInst_Name_AmbientColor "AmbientColor"
	/* isTimeEmissiveEnabled */
#define D_IVP5U_MatInst_Name_isTimeEmissiveEnabled "isTimeEmissiveEnabled"

	/* Luminous Specular Power th*/
#define D_IVP5U_Param_SpecularPower_Thd 110.0f
	/* Luminous Specular Power Min*/
#define D_IVP5U_Param_SpecularPower_Min 100.0f

	/***********************/

	UObject* InParent; // parent

	TMap<FString, UTexture*> ExistImages;

public:
	void InitializeBaseValue(
		UObject* InParentPtr)
	{
		InParent = InParentPtr;
	}

	void AssetsCreateTexture(
		UObject* InParentx,
		FString CurPath,
		FString filePath,
		TArray<UTexture*>& textureAssetList);

	UTexture* ImportTexture(
		FString InTextureFileName,
		bool bSetupAsNormalMap);

	void CreateUnrealMaterial(
		FString ParentObjName,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		bool bCreateMaterialInstanceMode,
		bool bMaterialUnlit,
		TArray<UMaterialInterface*>& OutMaterials,
		TArray<UTexture*>& textureAssetList);

	void FixupMaterial(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial);

	bool CreateAndLinkExpressionForMaterialProperty(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial,
		const char* MaterialProperty,
		FExpressionInput& MaterialInput,
		bool bSetupAsNormalMap,
		const FVector2D& Location,
		TArray<UTexture*>& textureAssetList);

	bool CreateAndLinkExpressionForMaterialProperty_ForMmdAutoLuminous(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial,
		FExpressionInput& MaterialInput,
		const FVector2D& Location,
		TArray<UTexture*>& textureAssetList);

	///////////////////////////////////////
	// IVP5U V2 Material Func

	//-------------------------------------------------------------------------
	// Duplicate  Base Material
	//-------------------------------------------------------------------------
	UMaterialInterface* DuplicateBaseMaterial(
		FString ParentObjName,
		EDuplicateBaseMatTypeIndex targetMatIndex);
	//-------------------------------------------------------------------------
	// Create Material Instance
	//-------------------------------------------------------------------------
	UMaterialInterface* CreateMaterialInst(
		FString ParentObjName,
		FString TargetMaterialName,
		UMaterialInterface* ParentMaterial);

	//-------------------------------------------------------------------------
	// Create Material Inst. for Masked Mat
	//-------------------------------------------------------------------------
	UMaterialInterface* CreateMaterialInst_Masked(
		FString ParentObjName,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		FString MaterialFullName,
		TArray<UTexture*>& textureAssetList);
	//-------------------------------------------------------------------------
	// Create Material Inst. for Masked Mat Unlit
	//-------------------------------------------------------------------------
	UMaterialInterface* CreateMaterialInst_Masked_Unlit(
		FString ParentObjName,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		FString MaterialFullName,
		TArray<UTexture*>& textureAssetList);
	//-------------------------------------------------------------------------
	// Create Material Inst. for Luminous Mat
	//-------------------------------------------------------------------------
	UMaterialInterface* CreateMaterialInst_Luminous(
		FString ParentObjName,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		FString MaterialFullName,
		TArray<UTexture*>& textureAssetList);
	//-------------------------------------------------------------------------
	// Create Material Inst. for Luminous Mat Unlit
	//-------------------------------------------------------------------------
	UMaterialInterface* CreateMaterialInst_Luminous_Unlit(
		FString ParentObjName,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		FString MaterialFullName,
		TArray<UTexture*>& textureAssetList);
};
