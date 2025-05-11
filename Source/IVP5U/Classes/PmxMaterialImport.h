// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"
#include "Factories/Factory.h"
#include "PmxImporter.h"
// #include "PmxMaterialImport.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCategoryPMXMaterialImport, Log, All)

/** 复制源材质索引*/
enum EDuplicateBaseMatTypeIndex
{
	/** Select Static Mesh if you'd like to import static mesh. */
	E_DupBaseMat_Typ_Normal = 0,
	/** Select Skeletal Mesh if you'd like to import skeletal mesh. */
	E_DupBaseMat_Typ_Luminous,
	/** Select Animation if you'd like to import only animation. */
	E_DupBaseMat_Typ_Unlit_Normal,
	/** Select Animation if you'd like to import only animation. */
	E_DupBaseMat_Typ_Unlit_Luminous,
	E_DupBaseMat_Typ_Max,
};

// UCLASS()
class UPmxMaterialImport
//: public UFactory // public UFbxFactory
{
	// GENERATED_UCLASS_BODY()

	// Begin UFactory Interface
	// virtual bool DoesSupportClass(UClass * Class) override;
	// virtual UClass* ResolveSupportedClass() override;

	/*
	 *复制导入模型时生成的材质的资源路径
	 *※将来如果能拥有PJ固有的设定路径的话
	 */
#define D_IVP5U_MMDBaseMat_Path_Normal "/IVP5U/Material/M_MMD_Toon.M_MMD_Toon"
#define D_IVP5U_MMDBaseMat_Path_Luminou "/IVP5U/Material/M_MMD_MatBase_Luminous.M_MMD_MatBase_Luminous"
#define D_IVP5U_MMDBaseMat_Path_Unlit_Normal "/IVP5U/Material/M_MMD_MatBase_UnLit.M_MMD_MatBase_UnLit"
#define D_IVP5U_MMDBaseMat_Path_Unlit_Luminou "/IVP5U/Material/M_MMD_MatBase_UnLit_Luminus.M_MMD_MatBase_UnLit_Luminus"

	/*
	 * Material Inst用Paramem Name
	 */
	/* Texture Base */
#define D_IVP5U_MatInst_Name_BaseTexture "TextureBase"
	/* is Texure Enable Flag */
#define D_IVP5U_MatInst_Name_isTextureEnable "isTextureEnable"
	/* Texture Base */
#define D_IVP5U_MatInst_Name_Toon "TextureToon"
	/* is Texure Enable Flag */
#define D_IVP5U_MatInst_Name_isToonEnable "isToonEnable"

	/* DiffuseColor*/
#define D_IVP5U_MatInst_Name_DiffuseColor "DiffuseColor"
	/* SpecularPower */
#define D_IVP5U_MatInst_Name_SpecularPower "SpecularPower"
	/* AmbientColor */
#define D_IVP5U_MatInst_Name_AmbientColor "AmbientColor"
	/* isTimeEmmisveEnable */
#define D_IVP5U_MatInst_Name_isTimeEmmisveEnable "isTimeEmmisveEnable"

	/* Luminous Specular Powor th*/
#define D_IVP5U_Param_SpecularPowor_Thd 110.0f
	/* Luminous Specular Powor Min*/
#define D_IVP5U_Param_SpecularPowor_Min 100.0f

	/***********************/

	UObject* InParent; // parent

	TMap<FString, UTexture*> ExistImages;

public:
	void InitializeBaseValue(
		UObject* InParentPtr)
	{
		InParent = InParentPtr;
	}
	//////////////////////////////////////
	void AssetsCreateTexture(
		UObject* InParentx,
		// EObjectFlags Flags,
		// FFeedbackContext * Warn,
		FString CurPath,
		FString filePath,
		TArray<UTexture*>& textureAssetList);
	///////////////////////////////////////
	UTexture* ImportTexture(
		// FbxFileTexture* FbxTexture,
		FString InTextureFileName,
		bool bSetupAsNormalMap);
	///////////////////////////////////////
	/*void AssetsCreateUnrealMaterial(
		UObject * InParent,
		FString MaterialName,
		TArray<UMaterialInterface*>& OutMaterials
		);
		*/
	/////////////////////////////////////
	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	void CreateUnrealMaterial(
		FString ParentObjName,
		// UObject * InParent,
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		bool bCreateMaterialInstMode,
		bool bMaterialUnlit,
		TArray<UMaterialInterface*>& OutMaterials,
		TArray<UTexture*>& textureAssetList);
	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	void FixupMaterial(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial);
	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	bool CreateAndLinkExpressionForMaterialProperty(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial,
		const char* MaterialProperty,
		FExpressionInput& MaterialInput,
		bool bSetupAsNormalMap,
		const FVector2D& Location,
		TArray<UTexture*>& textureAssetList);

	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	bool CreateAndLinkExpressionForMaterialProperty_ForMmdAutoluminus(
		MMD4UE5::PMX_MATERIAL& PmxMaterial,
		UMaterial* UnrealMaterial,
		// const char* MaterialProperty,
		FExpressionInput& MaterialInput,
		// bool bSetupAsNormalMap,
		// TArray<FString>& UVSet,
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