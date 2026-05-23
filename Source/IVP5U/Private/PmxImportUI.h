// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "PmxImportUI.generated.h"

/** Import mesh type */
UENUM()
enum EPMXImportType
{
	/** Select Static Mesh if you'd like to import static mesh. */
	PMXIT_StaticMesh UMETA(DisplayName = "Static Mesh"),
	/** Select Skeletal Mesh if you'd like to import skeletal mesh. */
	PMXIT_SkeletalMesh UMETA(DisplayName = "Skeletal Mesh"),
	/** Select Animation if you'd like to import only animation. */
	PMXIT_Animation UMETA(DisplayName = "Animation"),
	PMXIT_MAX,
};

UCLASS(config = EditorUserSettings, AutoExpandCategories = (FTransform), HideCategories = Object, MinimalAPI)
class UPmxImportUI : public UObject
{
	GENERATED_BODY()

public:
	UPmxImportUI(const FObjectInitializer& ObjectInitializer);

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	// UPROPERTY(EditAnywhere, Category = Mesh)
	class USkeleton* Skeleton;

	/** If checked, create new PhysicsAsset if it doesn't have it */
	UPROPERTY(EditAnywhere, config, Category = Mesh, meta = (ImportType = "SkeletalMesh"))
	uint32 bCreatePhysicsAsset : 1;

	/** If this is set, use this PhysicsAsset. It is possible bCreatePhysicsAsset == false, and PhysicsAsset == nullptr. It is possible they do not like to create anything. */
	// UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Mesh, meta = (ImportType = "SkeletalMesh", editcondition = "!bCreatePhysicsAsset"))
	class UPhysicsAsset* PhysicsAsset;

	/** create Unreal materials of MaterialInst Type */
	UPROPERTY(EditAnywhere, config, Category = Material, meta = (ToolTip = "If enabled, Create Material Instance and Duplicate Mat-Assets from IVP5U Base Mat. "))
	uint32 bCreateMaterialInstanceMode : 1;

	/** create Unreal materials of Unlit Type */
	UPROPERTY(EditAnywhere, config, Category = Material, meta = (ToolTip = "If CreateMaterialInstMode enabled, effective. Create Mat Shading Model is Unlit."))
	uint32 bUnlitMaterials : 1;

	/** Import data used when importing static meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "StaticMesh"))
	class UMMDStaticMeshImportData* StaticMeshImportData;

	/** Import data used when importing skeletal meshes */
	// UPROPERTY(EditAnywhere, Instanced, Category = Mesh, meta = (ImportType = "SkeletalMesh"))
	class UMMDSkeletalMeshImportData* SkeletalMeshImportData;

	/////////////////////////

	/** Skeleton to use for imported asset. When importing a mesh, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	// UPROPERTY(EditAnywhere, Category = Animation)
	class UAnimSequence* AnimSequenceAsset;

	/** MMD2UE5NameTableRow to use for imported asset. When importing a Anim, leaving this as "None" will create a new skeleton. When importing and animation this MUST be specified to import the asset. */
	// UPROPERTY(EditAnywhere, Category = Animation)
	UDataTable* MMD2UE5NameTableRow;

	/** mmd extend assset to use for calc ik . */
	// UPROPERTY(EditAnywhere, Category = Animation)
	class UMMDExtendAsset* MmdExtendAsset;
};
