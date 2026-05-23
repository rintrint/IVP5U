// Copyright 2015-2026 IVP5U contributors

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

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Mesh)
	class USkeleton* Skeleton;

	/** SkeletonMesh to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Mesh)
	class USkeletalMesh* SkeletonMesh;

	/////////////////////////

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Animation)
	class UAnimSequence* AnimSequenceAsset;

	/** Import uniform scale for the asset */
	UPROPERTY(EditAnywhere, Category = Transform, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ImportUniformScale;

	/** MMD2UE5NameTableRow to use for imported asset. When importing a Anim, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	UPROPERTY(EditAnywhere, Category = Animation)
	UDataTable* MMD2UE5NameTableRow;

	/** mmd extend assset to use for calc ik . */
	// UPROPERTY(EditAnywhere, Category = Animation)
	class UMMDExtendAsset* MmdExtendAsset;
};
