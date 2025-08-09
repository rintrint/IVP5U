// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#pragma once
#include "VmdImportUI.generated.h"

/** Import mesh type */
UENUM()
enum EVMDImportType
{
	/** Select Animation if you'd like to import only animation. */
	VMDIT_Animation UMETA(DisplayName = "Animation"),
	VMDIT_MAX,
};

UCLASS(config = EditorUserSettings, AutoExpandCategories = (FTransform), HideCategories = Object, MinimalAPI)
class UVmdImportUI : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Whether or not the imported file is in OBJ format */
	// UPROPERTY()
	bool bIsObjImport;

	/** The original detected type of this import */
	// UPROPERTY()
	TEnumAsByte<enum EVMDImportType> OriginalImportType;

	/** Type of asset to import from the FBX file */
	// UPROPERTY()
	TEnumAsByte<enum EVMDImportType> MeshTypeToImport;

	/** Use the string in "Name" field as full name of mesh. The option only works when the scene contains one mesh. */
	// UPROPERTY(EditAnywhere, AdvancedDisplay, config, Category = Miscellaneous, meta = (OBJRestrict = "true"))
	uint32 bOverrideFullName : 1;

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Mesh, meta = (OBJRestrict = "false"))
	class USkeleton* Skeleton;

	/** SkeletonMesh to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Mesh, meta = (OBJRestrict = "false"))
	class USkeletalMesh* SkeletonMesh;

	// UPROPERTY(EditAnywhere, Category = Mesh, meta = (OBJRestrict = "false"))
	class UIKRigDefinition* IKRig;

	/** True to import animations from the FBX File */
	// UPROPERTY(EditAnywhere, config, Category = Animation, meta = (ImportType = "SkeletalMesh|Animation"))
	uint32 bImportAnimations : 1;

	/** Override for the name of the animation to import **/
	// UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Animation, meta = (editcondition = "bImportAnimations", ImportType = "SkeletalMesh"))
	FString AnimationName;

	/** Import data used when importing static meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "StaticMesh"))
	// class UFbxStaticMeshImportData* StaticMeshImportData;

	/** Import data used when importing static meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "StaticMesh"))
	class UMMDStaticMeshImportData* StaticMeshImportData;

	/** Import data used when importing skeletal meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "SkeletalMesh"))
	// class UFbxSkeletalMeshImportData* SkeletalMeshImportData;

	/** Import data used when importing skeletal meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "SkeletalMesh"))
	class UMMDSkeletalMeshImportData* SkeletalMeshImportData;

	/** Import data used when importing animations */
	// UPROPERTY(EditAnywhere, Instanced, Category = Animation, meta = (editcondition = "bImportAnimations", ImportType = "Animation"))
	// class UFbxAnimSequenceImportData* AnimSequenceImportData;

	/** Type of asset to import from the FBX file */
	// UPROPERTY(EditAnywhere, AdvancedDisplay, config, Category = Animation, meta = (editcondition = "bImportAnimations", ImportType = "SkeletalMesh|Animation"))
	bool bPreserveLocalTransform;

	/** Import data used when importing textures */
	// UPROPERTY(EditAnywhere, Instanced, Category = Material)
	// class UFbxTextureImportData* TextureImportData;

	// Begin UObject Interface
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// End UObject Interface

	/////////////////////////

	/** Whether to automatically create Unreal materials for materials found in the FBX scene */

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Animation, meta = (OBJRestrict = "false"))
	class UAnimSequence* AnimSequenceAsset;

	/** Import uniform scale for the asset */
	UPROPERTY(EditAnywhere, Category = Transform, meta = (OBJRestrict = "false", ClampMin = "0.001", UIMin = "0.001"))
	float ImportUniformScale;

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	// UPROPERTY(EditAnywhere, Category = AnimationList, meta = (ImportType = "Animation"))
	TArray<class UMMDSkeletalMeshImportData*> TestArrayList;

	/** MMD2UE5NameTableRow to use for imported asset. When importing a Anim, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Animation, meta = (OBJRestrict = "false"))
	UDataTable* MMD2UE5NameTableRow;

	/** mmd extend assset to use for calc ik . */
	// UPROPERTY(EditAnywhere, Category = Animation, meta = (OBJRestrict = "false"))
	class UMMDExtendAsset* MmdExtendAsset;

	// struct FTableRowBase MMD2UE5NameTableRowDmmy;
};
