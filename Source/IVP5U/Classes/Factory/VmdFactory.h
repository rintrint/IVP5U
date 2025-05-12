// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"

#include "Factories/Factory.h"
#include "Factories.h"

#include "ImportUtils\SkelImport.h"
#include "AnimationUtils.h"

#include "VmdImportUI.h"

#include "VmdImporter.h"

#include "MMD2UE5NameTableRow.h"
#include "MMDExtendAsset.h"

#include "VmdFactory.generated.h"

// Forward declaration
class UInterpGroupInst;
static TMap<FName, FName> NameMap;

DECLARE_LOG_CATEGORY_EXTERN(LogMMD4UE5_VMDFactory, Log, All)

UCLASS()
class IVP5U_API UVmdFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// Begin UFactory Interface
	virtual void PostInitProperties() override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual UObject* FactoryCreateBinary(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		UObject* Context,
		const TCHAR* Type,
		const uint8*& Buffer,
		const uint8* BufferEnd,
		FFeedbackContext* Warn,
		bool& bOutOperationCanceled) override;

	/*Create AnimSequence from VMD data.（新建用父函数*/
	UAnimSequence* ImportAnimations(
		USkeleton* Skeleton,
		USkeletalMesh* SkeletalMesh,
		UObject* Outer,
		const FString& Name,
		UIKRigDefinition* IKRig,
		UDataTable* ReNameTable,
		UMMDExtendAsset* mmdExtend,
		MMD4UE5::VmdMotionInfo* vmdMotionInfo);
	//////////////
	class UVmdImportUI* ImportUI;

	/*面向MMD的贝塞尔曲线的计算处理*/
	float interpolateBezier(float x1, float y1, float x2, float y2, float x);

	/*将VMD表情数据添加到现有AnimSequ资源的过程
	 *与MMD4Mecanimu的综合利用测试功能*/
	UAnimSequence* AddtionalMorphCurveImportToAnimations(
		USkeletalMesh* SkeletalMesh,
		UAnimSequence* exsistAnimSequ,
		UDataTable* ReNameTable,
		MMD4UE5::VmdMotionInfo* vmdMotionInfo);

	/*准备VMD骨骼动画数据，但不执行AnimDataController操作*/
	bool PrepareVMDBoneAnimData(
		UAnimSequence* DestSeq,
		USkeleton* Skeleton,
		UDataTable* ReNameTable,
		UIKRigDefinition* IKRig,
		UMMDExtendAsset* mmdExtend,
		MMD4UE5::VmdMotionInfo* vmdMotionInfo,
		TArray<FName>& OutBoneNames,
		TArray<FRawAnimSequenceTrack>& OutRawTracks);

	/*准备VMD变形动画数据，但不执行AnimDataController操作*/
	bool PrepareMorphCurveData(
		UAnimSequence* DestSeq,
		USkeleton* Skeleton,
		USkeletalMesh* SkeletalMesh,
		UDataTable* ReNameTable,
		MMD4UE5::VmdMotionInfo* vmdMotionInfo,
		TArray<FAnimationCurveIdentifier>& OutCurvesToAdd,
		TMap<FAnimationCurveIdentifier, TArray<FRichCurveKey>>& OutCurveKeysMap);

	/*从MMD侧的名称检索并取得TableRow的UE侧名称*/
	bool FindTableRowMMD2UEName(
		UDataTable* ReNameTable,
		FName mmdName,
		FName* ue5Name);

	/*从Bone名称中搜索并获取与RefSkelton匹配的BoneIndex*/
	int32 FindRefBoneInfoIndexFromBoneName(
		const FReferenceSkeleton& RefSkelton,
		const FName& TargetName);

	/*递归计算当前关键帧中指定Bone的Glb坐标*/
	FTransform CalcGlbTransformFromBoneIndex(
		UAnimSequence* DestSeq,
		USkeleton* Skeleton,
		int32 BoneIndex,
		int32 keyIndex);
	float baseFrameRate;

public:
	UFUNCTION(BlueprintCallable, Category = "IVP5U")
	static bool ImportVmdFromFile(FString file, USkeletalMesh* SkeletalMesh);
};
