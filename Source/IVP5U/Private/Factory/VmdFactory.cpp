// Copyright 2023 NaN_Name. All Rights Reserved.

#include "Factory/VmdFactory.h"
#include "../IVP5UPrivatePCH.h"

#include "VmdImporter.h"

#include "CoreMinimal.h"
#include "ImportUtils/SkelImport.h"
#include "AnimationUtils.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "VmdImportUI.h"
#include "RigEditor/IKRigController.h"

#include "Factory/VmdImportOption.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "VMDImportFactory"

DEFINE_LOG_CATEGORY(LogMMD4UE5_VMDFactory)

#define ADD_NAME_MAP(x, y) NameMap.Add((y), (x))

void initMmdNameMap()
{
	if (NameMap.Num() == 0)
	{
		ADD_NAME_MAP(L"左足", L"左足D");
		ADD_NAME_MAP(L"左ひざ", L"左ひざD");
		ADD_NAME_MAP(L"左足首", L"左足首D");
		ADD_NAME_MAP(L"左つま先", L"左足先EX");
		ADD_NAME_MAP(L"右足", L"右足D");
		ADD_NAME_MAP(L"右ひざ", L"右ひざD");
		ADD_NAME_MAP(L"右足首", L"右足首D");
		ADD_NAME_MAP(L"右つま先", L"右足先EX");
		// ADD_NAME_MAP(L"センター", L"腰");
		ADD_NAME_MAP(L"腰キャンセル右", L"右腰キャンセル");
		ADD_NAME_MAP(L"腰キャンセル左", L"左腰キャンセル");
	}

	// if (NameMap.Num() == 0)
	// {
	// 	ADD_NAME_MAP(L"操作中心", L"op_center");
	// 	ADD_NAME_MAP(L"全ての親", L"all_parent");
	// 	ADD_NAME_MAP(L"センター", L"center");
	// 	ADD_NAME_MAP(L"センター2", L"center2");
	// 	ADD_NAME_MAP(L"グルーブ", L"groove");
	// 	ADD_NAME_MAP(L"グルーブ2", L"groove2");
	// 	ADD_NAME_MAP(L"腰", L"waist");
	// 	ADD_NAME_MAP(L"下半身", L"lowerBody");
	// 	ADD_NAME_MAP(L"上半身", L"upperBody");
	// 	ADD_NAME_MAP(L"上半身", L"upperBody2");
	// 	ADD_NAME_MAP(L"首", L"neck");
	// 	ADD_NAME_MAP(L"頭", L"head");
	// 	ADD_NAME_MAP(L"左目", L"eyeL");
	// 	ADD_NAME_MAP(L"右目", L"eyeR");

	// 	ADD_NAME_MAP(L"左肩", L"shoulderL");
	// 	ADD_NAME_MAP(L"左腕", L"armL");
	// 	ADD_NAME_MAP(L"左ひじ", L"elbowL");
	// 	ADD_NAME_MAP(L"左手首", L"wristL");
	// 	ADD_NAME_MAP(L"左親指０", L"thumb0L");
	// 	ADD_NAME_MAP(L"左親指１", L"thumb1L");
	// 	ADD_NAME_MAP(L"左親指２", L"thumb2L");
	// 	ADD_NAME_MAP(L"左人指０", L"fore0L");
	// 	ADD_NAME_MAP(L"左人指１", L"fore1L");
	// 	ADD_NAME_MAP(L"左人指２", L"fore2L");
	// 	ADD_NAME_MAP(L"左中指０", L"middle0L");
	// 	ADD_NAME_MAP(L"左中指１", L"middle1L");
	// 	ADD_NAME_MAP(L"左中指２", L"middle2L");
	// 	ADD_NAME_MAP(L"左薬指０", L"third0L");
	// 	ADD_NAME_MAP(L"左薬指１", L"third1L");
	// 	ADD_NAME_MAP(L"左薬指２", L"third2L");
	// 	ADD_NAME_MAP(L"左小指０", L"little0L");
	// 	ADD_NAME_MAP(L"左小指１", L"little1L");
	// 	ADD_NAME_MAP(L"左小指２", L"little2L");
	// 	ADD_NAME_MAP(L"左足", L"legL");
	// 	ADD_NAME_MAP(L"左ひざ", L"kneeL");
	// 	ADD_NAME_MAP(L"左足首", L"ankleL");

	// 	ADD_NAME_MAP(L"右肩", L"shoulderR");
	// 	ADD_NAME_MAP(L"右腕", L"armR");
	// 	ADD_NAME_MAP(L"右ひじ", L"elbowR");
	// 	ADD_NAME_MAP(L"右手首", L"wristR");
	// 	ADD_NAME_MAP(L"右親指０", L"thumb0R");
	// 	ADD_NAME_MAP(L"右親指１", L"thumb1R");
	// 	ADD_NAME_MAP(L"右親指２", L"thumb2R");
	// 	ADD_NAME_MAP(L"右人指０", L"fore0R");
	// 	ADD_NAME_MAP(L"右人指１", L"fore1R");
	// 	ADD_NAME_MAP(L"右人指２", L"fore2R");
	// 	ADD_NAME_MAP(L"右中指０", L"middle0R");
	// 	ADD_NAME_MAP(L"右中指１", L"middle1R");
	// 	ADD_NAME_MAP(L"右中指２", L"middle2R");
	// 	ADD_NAME_MAP(L"右薬指０", L"third0R");
	// 	ADD_NAME_MAP(L"右薬指１", L"third1R");
	// 	ADD_NAME_MAP(L"右薬指２", L"third2R");
	// 	ADD_NAME_MAP(L"右小指０", L"little0R");
	// 	ADD_NAME_MAP(L"右小指１", L"little1R");
	// 	ADD_NAME_MAP(L"右小指２", L"little2R");
	// 	ADD_NAME_MAP(L"右足", L"legR");
	// 	ADD_NAME_MAP(L"右ひざ", L"kneeR");
	// 	ADD_NAME_MAP(L"右足首", L"ankleR");

	// 	ADD_NAME_MAP(L"両目", L"eyes");
	// 	ADD_NAME_MAP(L"左足ＩＫ", L"ikLegL");
	// 	ADD_NAME_MAP(L"右足ＩＫ", L"ikLegR");
	// 	ADD_NAME_MAP(L"左つま先ＩＫ", L"ikToeL");
	// 	ADD_NAME_MAP(L"右つま先ＩＫ", L"ikToeR");
	// }
}

/////////////////////////////////////////////////////////
// prototype ::from dxlib
// 创建以X轴为中心的旋转矩阵
void CreateRotationXMatrix(FMatrix* Out, float Angle);
// 求仅旋转分量矩阵的积（3）×3以外的部分也不代入值）
void MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(FMatrix* Out, FMatrix* In1, FMatrix* In2);
// 判定角度限制的共同函数（subIndexJdg的判定比较不明…）
void CheckLimitAngle(
	const FVector& RotMin,
	const FVector& RotMax,
	FVector* outAngle, // target angle ( in and out param)
	bool subIndexJdg   //(ik link index < ik loop temp):: linkBoneIndex < ikt
);
///////////////////////////////////////////////////////

UVmdFactory::UVmdFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = NULL;
	// SupportedClass = UPmxFactory::StaticClass();
	Formats.Empty();

	Formats.Add(TEXT("vmd;vmd animations"));

	bCreateNew = false;
	bText = false;
	bEditorImport = true;

	initMmdNameMap();
}

void UVmdFactory::PostInitProperties()
{
	Super::PostInitProperties();

	ImportUI = NewObject<UVmdImportUI>(this, NAME_None, RF_NoFlags);
}

bool UVmdFactory::DoesSupportClass(UClass* Class)
{
	return (Class == UVmdFactory::StaticClass());
}

UClass* UVmdFactory::ResolveSupportedClass()
{
	return UVmdFactory::StaticClass();
}

UObject* UVmdFactory::FactoryCreateBinary(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	const TCHAR* Type,
	const uint8*& Buffer,
	const uint8* BufferEnd,
	FFeedbackContext* Warn,
	bool& bOutOperationCanceled)
{
	UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始FactoryCreateBinary: Name=%s"), *Name.ToString());

	MMD4UE5::VmdMotionInfo vmdMotionInfo;

	// 读取VMD文件，这是第一个主要瓶颈，导致显示导入选项对话框前有一段延迟，直到显示对话框
	UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始读取VMD文件"));
	double StartTime = FPlatformTime::Seconds();
	if (vmdMotionInfo.VMDLoaderBinary(Buffer, BufferEnd) == false)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("VMD导入取消:: VMD数据读取失败"));
		return NULL;
	}
	UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("读取VMD文件耗时：%.3fs"), FPlatformTime::Seconds() - StartTime);

	UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD数据读取成功，keyBoneList=%d，keyFaceList=%d，keyCameraList=%d"),
		vmdMotionInfo.keyBoneList.Num(), vmdMotionInfo.keyFaceList.Num(), vmdMotionInfo.keyCameraList.Num());

	// 输出关键帧统计、动画长度等实际讯息
	{
		int32 totalBoneKeyframes = 0;
		int32 totalFaceKeyframes = 0;
		int32 totalCameraKeyframes = 0;

		// 计算骨骼关键帧总数
		for (int i = 0; i < vmdMotionInfo.keyBoneList.Num(); ++i)
		{
			totalBoneKeyframes += vmdMotionInfo.keyBoneList[i].keyList.Num();
		}

		// 计算表情关键帧总数
		for (int i = 0; i < vmdMotionInfo.keyFaceList.Num(); ++i)
		{
			totalFaceKeyframes += vmdMotionInfo.keyFaceList[i].keyList.Num();
		}

		// 计算相机关键帧总数
		for (int i = 0; i < vmdMotionInfo.keyCameraList.Num(); ++i)
		{
			totalCameraKeyframes += vmdMotionInfo.keyCameraList[i].keyList.Num();
		}

		// 输出总关键帧数量
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD关键帧统计：骨骼关键帧总数=%d，表情关键帧总数=%d，相机关键帧总数=%d，总关键帧数=%d"),
			totalBoneKeyframes, totalFaceKeyframes, totalCameraKeyframes,
			totalBoneKeyframes + totalFaceKeyframes + totalCameraKeyframes);

		// 输出动画长度信息
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD动画长度：最小帧=%d，最大帧=%d，总帧数=%d，时长=%.2f秒"),
			vmdMotionInfo.minFrame, vmdMotionInfo.maxFrame, vmdMotionInfo.maxFrame + 1,
			(vmdMotionInfo.maxFrame + 1) / 30.0f); // 假设VMD帧率为30fps
	}

	/////////////////////////////////////////
	UAnimSequence* LastCreatedAnim = NULL;
	USkeleton* Skeleton = NULL;
	USkeletalMesh* SkeletalMesh = NULL;
	UIKRigDefinition* IKRig = NULL;
	VMDImportOptions* ImportOptions = NULL;

	// 检查是否为相机动画
	if (vmdMotionInfo.keyCameraList.Num() == 0)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("非相机动画，准备导入骨骼和变形动画"));
		// 如果不是摄影机动画
		FVmdImporter* VmdImporter = FVmdImporter::GetInstance();

		EVMDImportType ForcedImportType = VMDIT_Animation;
		bool bOperationCanceled = false;
		bool bIsPmxFormat = true;
		// show Import Option Slate
		bool bImportAll = false;
		ImportUI->bIsObjImport = false; // anim mode
		ImportUI->OriginalImportType = EVMDImportType::VMDIT_Animation;

		// 显示导入选项对话框，等待用户操作
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("显示导入选项对话框，等待用户操作"));
		ImportOptions = GetVMDImportOptions(
			VmdImporter,
			ImportUI,
			true, // bShowImportDialog,
			InParent->GetPathName(),
			bOperationCanceled,
			bImportAll,
			ImportUI->bIsObjImport, // bIsPmxFormat,
			bIsPmxFormat,
			ForcedImportType);

		// 檢查用戶是否取消了操作
		if (bOperationCanceled)
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("用户取消VMD导入"));
			bOutOperationCanceled = true; // 設置引擎級取消標誌
			return NULL;				  // 返回NULL表示沒有創建資產
		}

		/* 第一次判定 */
		if (ImportOptions)
		{
			Skeleton = ImportUI->Skeleton;
			SkeletalMesh = ImportUI->SkeletonMesh;

			/* 最低限度的参数设置检查 */
			if ((!Skeleton) || (!SkeletalMesh) || (Skeleton != SkeletalMesh->GetSkeleton()))
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("导入选项参数检查不通过，重新获取导入选项"));
				ImportOptions = GetVMDImportOptions(
					VmdImporter,
					ImportUI,
					true, // bShowImportDialog,
					InParent->GetPathName(),
					bOperationCanceled,
					bImportAll,
					ImportUI->bIsObjImport, // bIsPmxFormat,
					bIsPmxFormat,
					ForcedImportType);

				// 再次檢查取消狀態
				if (bOperationCanceled)
				{
					UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("用户取消VMD导入"));
					bOutOperationCanceled = true; // 設置引擎級取消標誌
					return NULL;				  // 返回NULL表示沒有創建資產
				}
			}
		}

		if (ImportOptions)
		{
			Skeleton = ImportUI->Skeleton;
			SkeletalMesh = ImportUI->SkeletonMesh;
			IKRig = ImportUI->IKRig;

			bool preParamChk = true;
			/*包含关系检查*/
			if (SkeletalMesh)
			{
				if (Skeleton != SkeletalMesh->GetSkeleton())
				{
					UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("错误: Skeleton不等于SkeletalMesh->GetSkeleton()"));
					preParamChk = false;
				}
			}

			if (preParamChk)
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("参数检查通过，开始导入动画"));
				////////////////////////////////////
				if (!ImportOptions->AnimSequenceAsset)
				{
					// 根据之前从VMD文件读取的数据添加动画序列，这是第二个主要瓶颈，导致点击对话框的Import后有一段延迟，直到导入成功
					UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始添加动画序列"));
					StartTime = FPlatformTime::Seconds();
					// create AnimSequence Asset from VMD
					LastCreatedAnim = ImportAnimations(
						Skeleton,
						SkeletalMesh,
						InParent,
						Name.ToString(),
						IKRig,
						ImportUI->MMD2UE5NameTableRow,
						ImportUI->MmdExtendAsset,
						&vmdMotionInfo);
					UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("添加动画序列耗时：%.3fs"), FPlatformTime::Seconds() - StartTime);
				}
				else
				{
					// add morph curve only to exist ainimation
					LastCreatedAnim = AddtionalMorphCurveImportToAnimations(
						SkeletalMesh,
						ImportOptions->AnimSequenceAsset, // UAnimSequence* exsistAnimSequ,
						ImportUI->MMD2UE5NameTableRow,
						&vmdMotionInfo);
				}
			}
			else
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("参数检查失败，导入错误"));
			}
		}
		else
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("导入选项获取失败"));
		}
	}
	else
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("这个VMD文件是相机动画，请按Sequencer里的Import VMD file按钮"));

		// 添加Slate通知功能
		const bool bNotifySlate = !FApp::IsUnattended() && !GIsRunningUnattendedScript;
		if (bNotifySlate)
		{
			FNotificationInfo Info(LOCTEXT("CameraMotionImportError", "这个VMD文件是相机动画，请按Sequencer里的Import VMD file按钮"));
			Info.ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Fail);
		}

		bOutOperationCanceled = true; // 設置引擎級取消標誌
		return NULL;				  // 返回NULL表示沒有創建資產
	}
	return LastCreatedAnim;
};
/*******************
 * 创建并初始化动画序列
 * 按照两个阶段处理VMD动画数据：
 * 1. 准备阶段：使用PrepareVMDBoneAnimData和PrepareMorphCurveData提取并转换数据
 * 2. 应用阶段：使用AnimDataController批量应用所有骨骼和变形数据
 * 这种分离可提高性能避免多次调用OpenBracket/CloseBracket
 **********************/
UAnimSequence* UVmdFactory::ImportAnimations(
	USkeleton* Skeleton,
	USkeletalMesh* SkeletalMesh,
	UObject* Outer,
	const FString& Name,
	UIKRigDefinition* IKRig,
	UDataTable* ReNameTable,
	UMMDExtendAsset* mmdExtend,
	MMD4UE5::VmdMotionInfo* vmdMotionInfo)
{
	UAnimSequence* LastCreatedAnim = NULL;

	// 检查骨骼是否存在
	if (Skeleton == NULL)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("ImportAnimations: Skeleton为空"));
		return NULL;
	}

	// 优化：使用字符串构建器而不是多次连接
	FString SequenceName = FString::Printf(TEXT("%s_%s"), *Name, *Skeleton->GetName());
	SequenceName = ObjectTools::SanitizeObjectName(SequenceName);

	// 优化：使用字符串视图避免不必要的复制
	FString animpath = SkeletalMesh->GetPathName();
	int32 indexs = animpath.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	if (indexs != INDEX_NONE)
	{
		animpath.LeftInline(indexs);
	}
	else
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("获取动画路径失败"));
		return NULL;
	}

	// 优化：直接使用格式化字符串
	FString ParentPath = Outer ? FString::Printf(TEXT("%s/%s"), *FPackageName::GetLongPackagePath(*Outer->GetName()), *SequenceName) : FString::Printf(TEXT("%s/%s"), *animpath, *SequenceName);

	UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("动画路径: %s"), *ParentPath);

	// 优化：创建包时使用已有的字符串
	UObject* ParentPackage = CreatePackage(*ParentPath);
	UAnimSequence* DestSeq = nullptr;

	// 优化：避免重复字符串连接
	FString AssetPath = FString::Printf(TEXT("%s.%s"), *ParentPath, *SequenceName);

	// 优化：使用更具体的搜索而不是通用搜索
	UObject* ExistingAsset = StaticFindObject(UAnimSequence::StaticClass(), nullptr, *AssetPath);

	if (ExistingAsset)
	{
		// 资产已存在，直接转换
		DestSeq = Cast<UAnimSequence>(ExistingAsset);

		if (!DestSeq)
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("同名资产已存在但不是AnimSequence类型: %s"), *AssetPath);
			return LastCreatedAnim;
		}
		else
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("重置现有AnimSequence: %s"), *SequenceName);
			DestSeq->ResetAnimation();
		}
	}
	else
	{
		// 优化：直接创建动画序列
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("创建新的AnimSequence: %s"), *SequenceName);
		DestSeq = NewObject<UAnimSequence>(ParentPackage, *SequenceName, RF_Public | RF_Standalone);

		// 通知资产注册表
		FAssetRegistryModule::AssetCreated(DestSeq);
	}

	// 设置骨架引用
	DestSeq->SetSkeleton(Skeleton);
	LastCreatedAnim = DestSeq;

	// 如果成功创建了动画序列
	if (LastCreatedAnim)
	{
		bool importSuccessFlag = true;
		double StartTime = 0.0;

		// 准备骨骼动画数据
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始准备VMD骨骼动画数据"));
		StartTime = FPlatformTime::Seconds();
		TArray<FName> BoneNames;
		TArray<FRawAnimSequenceTrack> RawTracks;
		if (!PrepareVMDBoneAnimData(LastCreatedAnim, Skeleton, ReNameTable, IKRig, mmdExtend, vmdMotionInfo, BoneNames, RawTracks))
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("PrepareVMDBoneAnimData失败"));
			importSuccessFlag = false;
		}
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("完成准备VMD骨骼动画数据，耗时: %.3f秒"), FPlatformTime::Seconds() - StartTime);

		// 准备变形动画数据
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始准备VMD变形动画数据"));
		StartTime = FPlatformTime::Seconds();
		TArray<FAnimationCurveIdentifier> CurvesToAdd;
		TMap<FAnimationCurveIdentifier, TArray<FRichCurveKey>> CurveKeysMap;
		if (!PrepareMorphCurveData(LastCreatedAnim, Skeleton, SkeletalMesh, ReNameTable, vmdMotionInfo, CurvesToAdd, CurveKeysMap))
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("PrepareMorphCurveData失败"));
			importSuccessFlag = false;
		}
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("完成准备VMD变形动画数据，耗时: %.3f秒"), FPlatformTime::Seconds() - StartTime);

		// 执行AnimDataController操作
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("开始执行AnimDataController操作"));
			StartTime = FPlatformTime::Seconds();

			auto& adc = LastCreatedAnim->GetController();

			// 开启括号
			adc.OpenBracket(LOCTEXT("ImportAsSkeletalMesh", "Importing VMD Animation"));

			// 添加骨骼轨道
			for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
			{
				FName BoneName = BoneNames[BoneIndex];
				FRawAnimSequenceTrack& RawTrack = RawTracks[BoneIndex];

				// 实际上到这一步所有骨骼都会有2个关键帧了，所以即便是VMD内只有1帧或是静止不动的骨骼，都会添加骨骼动画曲线
				// 腳本目前是給所有Skeleton所有骨骼都添加骨骼动画曲线
				if (RawTrack.PosKeys.Num() > 1)
				{
					if (adc.AddBoneCurve(BoneName))
					{
						if (BoneName == L"腰")
						{
							for (int32 ix = 0; ix < RawTrack.PosKeys.Num(); ix++)
							{
								RawTrack.PosKeys[ix].X = 0.0f;
								RawTrack.PosKeys[ix].Y = 0.0f;
								RawTrack.RotKeys[ix] = FQuat4f(0.0f, 0.0f, 0.0f, 1.0f);
							}
						}

						adc.SetBoneTrackKeys(BoneName, RawTrack.PosKeys, RawTrack.RotKeys, RawTrack.ScaleKeys);
					}
					else
					{
						UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("骨骼[%d]:[%s]添加軌道失敗"),
							BoneIndex, *BoneName.ToString());
					}
				}
			}

			// 添加变形曲线
			for (const FAnimationCurveIdentifier& CurveId : CurvesToAdd)
			{
				adc.AddCurve(CurveId);
			}

			for (auto& Pair : CurveKeysMap)
			{
				adc.SetCurveKeys(Pair.Key, Pair.Value);
			}

			// 更新曲线名称并通知填充
			adc.UpdateCurveNamesFromSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
			adc.NotifyPopulated();

			// 关闭括号
			adc.CloseBracket();

			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("完成执行AnimDataController操作，耗时: %.3f秒"), FPlatformTime::Seconds() - StartTime);
		}

		// 导入成功时更新预览网格
		if ((importSuccessFlag) && (SkeletalMesh))
		{
			LastCreatedAnim->SetPreviewMesh(SkeletalMesh);
		}

		// 标记包为脏
		MarkPackageDirty();
		SkeletalMesh->MarkPackageDirty();

		// 确保初始化正确
		LastCreatedAnim->Modify();
		LastCreatedAnim->PostEditChange();
		LastCreatedAnim->SetPreviewMesh(SkeletalMesh);
		LastCreatedAnim->MarkPackageDirty();

		// 设置骨架预览网格
		Skeleton->SetPreviewMesh(SkeletalMesh);
		Skeleton->PostEditChange();
	}

	return LastCreatedAnim;
}

/******************************************************************************
 * Start
 * copy from: http://d.hatena.ne.jp/edvakf/touch/20111016/1318716097
 * x1~y2 : 0 <= xy <= 1 :bezier points
 * x : 0<= x <= 1 : frame rate
 ******************************************************************************/
float UVmdFactory::interpolateBezier(float x1, float y1, float x2, float y2, float x)
{
	// optimize: fast path for boundary values
	if (x <= 0.0f)
		return 0.0f;
	if (x >= 1.0f)
		return 1.0f;

	float t = 0.5, s = 0.5;
	for (int i = 0; i < 15; i++)
	{
		float ft = (3 * s * s * t * x1) + (3 * s * t * t * x2) + (t * t * t) - x;
		if (ft == 0)
			break; // Math.abs(ft) < 0.00001 でもいいかも
		if (FGenericPlatformMath::Abs(ft) < 0.0001)
			break;
		if (ft > 0)
			t -= 1.0 / (float)(4 << i);
		else // ft < 0
			t += 1.0 / (float)(4 << i);
		s = 1 - t;
	}
	return (3 * s * s * t * y1) + (3 * s * t * t * y2) + (t * t * t);
}

/** End **********************************************************************/

/*******************
 *将VMD表情数据添加到现有AnimSequ资源的过程
 *与MMD4Mecanimu的综合利用测试功能
 **********************/
UAnimSequence* UVmdFactory::AddtionalMorphCurveImportToAnimations(
	USkeletalMesh* SkeletalMesh,
	UAnimSequence* exsistAnimSequ,
	UDataTable* ReNameTable,
	MMD4UE5::VmdMotionInfo* vmdMotionInfo)
{
	USkeleton* Skeleton = NULL;
	// we need skeleton to create animsequence
	if (exsistAnimSequ == NULL)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("exsistAnimSequ为空"));
		return NULL;
	}

	Skeleton = exsistAnimSequ->GetSkeleton();

	///////////////////////////////////
	// 准备变形曲线数据，然后执行AnimDataController操作
	//////////////////////
	if (exsistAnimSequ)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("准备导入变形曲线"));

		// 准备变形曲线数据
		TArray<FAnimationCurveIdentifier> CurvesToAdd;
		TMap<FAnimationCurveIdentifier, TArray<FRichCurveKey>> CurveKeysMap;

		if (!PrepareMorphCurveData(
				exsistAnimSequ,
				Skeleton,
				SkeletalMesh,
				ReNameTable,
				vmdMotionInfo,
				CurvesToAdd,
				CurveKeysMap))
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("PrepareMorphCurveData失败"));
			return exsistAnimSequ;
		}

		// 执行AnimDataController操作
		{
			auto& adc = exsistAnimSequ->GetController();
			// 开启括号
			adc.OpenBracket(LOCTEXT("ImportMorphAnimation", "Importing VMD Morph Animation"));

			// 添加所有曲线
			for (const FAnimationCurveIdentifier& CurveId : CurvesToAdd)
			{
				adc.AddCurve(CurveId);
			}

			// 添加所有关键帧
			for (auto& Pair : CurveKeysMap)
			{
				adc.SetCurveKeys(Pair.Key, Pair.Value);
			}

			// 更新曲线名称并通知填充
			adc.UpdateCurveNamesFromSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
			adc.NotifyPopulated();

			// 关闭括号
			adc.CloseBracket();
		}
	}

	/////////////////////////////////////////
	// end process
	////////////////////////////////////////
	if (exsistAnimSequ)
	{
		// 标记包为脏
		exsistAnimSequ->MarkPackageDirty();

		if (SkeletalMesh)
		{
			exsistAnimSequ->SetPreviewMesh(SkeletalMesh);
			SkeletalMesh->MarkPackageDirty();

			Skeleton->SetPreviewMesh(SkeletalMesh);
			Skeleton->PostEditChange();
		}

		// 确保初始化正确
		exsistAnimSequ->Modify();
		exsistAnimSequ->PostEditChange();
	}

	return exsistAnimSequ;
}
/*******************
 *准备Morph目标AnimCurve数据
 *将VMD文件中的变形目标数据转换为AnimSeq可用的格式，但不直接应用
 *收集并准备所有曲线数据，以便后续批量添加到动画序列中
 **********************/
bool UVmdFactory::PrepareMorphCurveData(
	UAnimSequence* DestSeq,
	USkeleton* Skeleton,
	USkeletalMesh* SkeletalMesh,
	UDataTable* ReNameTable,
	MMD4UE5::VmdMotionInfo* vmdMotionInfo,
	TArray<FAnimationCurveIdentifier>& OutCurvesToAdd,
	TMap<FAnimationCurveIdentifier, TArray<FRichCurveKey>>& OutCurveKeysMap)
{
	if (!DestSeq || !Skeleton || !vmdMotionInfo)
	{
		return false;
	}

	USkeletalMesh* mesh = SkeletalMesh;
	if (!mesh)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("PrepareMorphCurveData GetAssetPreviewMesh未找到"));
		return false;
	}

	// 预加载所有表情映射
	TMap<FName, UMorphTarget*> MorphTargetMap;
	const TArray<TObjectPtr<UMorphTarget>>& MorphTargets = mesh->GetMorphTargets();
	MorphTargetMap.Reserve(MorphTargets.Num());
	for (const TObjectPtr<UMorphTarget>& MorphTarget : MorphTargets)
	{
		MorphTargetMap.Add(MorphTarget->GetFName(), MorphTarget.Get());
	}

	// 预处理重命名表
	TMap<FName, FName> RenameMap;
	if (ReNameTable)
	{
		RenameMap.Reserve(ReNameTable->GetRowNames().Num());
		const TArray<FName> RowNames = ReNameTable->GetRowNames();
		FString ContextString;

		for (const FName& RowName : RowNames)
		{
			const FMMD2UE5NameTableRow* DataRow = ReNameTable->FindRow<FMMD2UE5NameTableRow>(RowName, ContextString);
			if (DataRow)
			{
				RenameMap.Add(RowName, FName(*DataRow->MmdOriginalName));
			}
		}
	}

	// 检查VMD表情在模型中的存在情况
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== 检查VMD表情在模型中的存在情况 ======"));

		// 统计变量
		int32 totalVmdMorphs = vmdMotionInfo->keyFaceList.Num();
		int32 foundMorphs = 0;
		int32 notFoundMorphs = 0;
		int32 totalMeshMorphs = MorphTargets.Num();

		// 用于检测的临时映射
		TSet<FName> TestedMorphs;
		TestedMorphs.Reserve(totalVmdMorphs);

		for (int i = 0; i < totalVmdMorphs; ++i)
		{
			FName Name = *vmdMotionInfo->keyFaceList[i].TrackName;
			FName* MappedName = RenameMap.Find(Name);
			FName TestName = MappedName ? *MappedName : Name;

			if (!TestedMorphs.Contains(TestName))
			{
				TestedMorphs.Add(TestName);

				if (!MorphTargetMap.Contains(TestName))
				{
					UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("VMD表情在模型中未找到: [%s]"),
						*vmdMotionInfo->keyFaceList[i].TrackName);
					notFoundMorphs++;
				}
				else
					foundMorphs++;
			}
		}

		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("模型中共有 %d 个表情，VMD中共有 %d 个表情"),
			totalMeshMorphs, totalVmdMorphs);
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD表情在模型中找到的有 %d 个，未找到的有 %d 个"),
			foundMorphs, notFoundMorphs);

		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== VMD表情检查完成 ======"));
	}

	// 预先计算序列长度
	const float SequenceLength = DestSeq->GetPlayLength();

	// 第一阶段：先收集所有需要添加的曲线，然后一次性添加
	// 预估总关键帧数，避免过多的记忆体重分配
	int32 totalEstimatedKeyframes = 0;
	for (int i = 0; i < vmdMotionInfo->keyFaceList.Num(); ++i)
	{
		if (vmdMotionInfo->keyFaceList[i].keyList.Num() > 1 || (vmdMotionInfo->keyFaceList[i].keyList.Num() == 1 && vmdMotionInfo->keyFaceList[i].keyList[0].Factor != 0.0f))
		{
			totalEstimatedKeyframes += vmdMotionInfo->keyFaceList[i].keyList.Num();
		}
	}

	// 预分配足够的空间
	OutCurvesToAdd.Reset();
	OutCurveKeysMap.Reset();
	OutCurvesToAdd.Reserve(vmdMotionInfo->keyFaceList.Num());
	OutCurveKeysMap.Reserve(vmdMotionInfo->keyFaceList.Num());

	// 第一阶段：收集所有曲线数据
	for (int i = 0; i < vmdMotionInfo->keyFaceList.Num(); ++i)
	{
		MMD4UE5::VmdFaceTrackList* vmdFaceTrackPtr = &vmdMotionInfo->keyFaceList[i];

		// 跳过只有一个值为0的关键帧
		if (vmdFaceTrackPtr->keyList.Num() == 1 && vmdFaceTrackPtr->keyList[0].Factor == 0.0f)
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("表情[%d]:[%s]只有一个值为0的关键帧，跳过"),
				i, *vmdFaceTrackPtr->TrackName);
			continue;
		}

		// 获取原始名称
		FName Name = *vmdFaceTrackPtr->TrackName;
		FName* MappedName = RenameMap.Find(Name);
		if (MappedName)
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("表情[%d]:[%s]表情名称从转换表映射: %s -> %s"),
				i, *Name.ToString(), *Name.ToString(), *MappedName->ToString());
			Name = *MappedName;
		}

		// 检查该表情是否存在于模型中
		UMorphTarget* morphTargetPtr = MorphTargetMap.FindRef(Name);
		if (!morphTargetPtr)
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Warning,
				TEXT("表情[%d]:[%s]未找到变形目标...搜索[%s]VMD原始名称[%s]"),
				i, *Name.ToString(), *Name.ToString(), *vmdFaceTrackPtr->TrackName);
			continue;
		}

		// 创建曲线标识符
		FAnimationCurveIdentifier CurveId(Name, ERawCurveTrackTypes::RCT_Float);
		if (!CurveId.IsValid())
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("表情[%d]:[%s]曲线标识符无效"),
				i, *Name.ToString());
			continue;
		}

		// 记录需要添加的曲线
		OutCurvesToAdd.Add(CurveId);

		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("表情[%d]:[%s]添加关键帧，总数: %d"),
			i, *Name.ToString(), vmdFaceTrackPtr->keyList.Num());

		// 预分配关键帧数组空间
		TArray<FRichCurveKey>& keyFrames = OutCurveKeysMap.Add(CurveId);
		keyFrames.Reserve(vmdFaceTrackPtr->keyList.Num());

		// 处理每个关键帧
		for (int s = 0; s < vmdFaceTrackPtr->keyList.Num(); ++s)
		{
			int sortedIndex = vmdFaceTrackPtr->sortIndexList[s];
			if (sortedIndex >= vmdFaceTrackPtr->keyList.Num())
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("表情[%d]:[%s]索引越界: %d/%d"),
					i, *Name.ToString(), sortedIndex, vmdFaceTrackPtr->keyList.Num());
				continue;
			}

			MMD4UE5::VMD_FACE_KEY* faceKeyPtr = &vmdFaceTrackPtr->keyList[sortedIndex];
			float timeCurve = faceKeyPtr->Frame / 30.0f;

			if (timeCurve > SequenceLength)
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("表情[%d]:[%s]关键帧时间 %f 超出序列长度 %f，停止添加"),
					i, *Name.ToString(), timeCurve, SequenceLength);
				break;
			}

			keyFrames.Add(FRichCurveKey(timeCurve, faceKeyPtr->Factor));
		}
	}

	return true;
}

/*******************
 * 准备VMD骨骼动画数据
 * 从VMD文件数据中提取骨骼动画信息并转换为UE可用的格式
 * 生成所有骨骼轨道的原始数据，以便后续批量添加到动画序列中
 **********************/
bool UVmdFactory::PrepareVMDBoneAnimData(
	UAnimSequence* DestSeq,
	USkeleton* Skeleton,
	UDataTable* ReNameTable,
	UIKRigDefinition* IKRig,
	UMMDExtendAsset* mmdExtend,
	MMD4UE5::VmdMotionInfo* vmdMotionInfo,
	TArray<FName>& OutBoneNames,
	TArray<FRawAnimSequenceTrack>& OutRawTracks)
{
	// nullptr check in-param
	if (!DestSeq || !Skeleton || !vmdMotionInfo)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error,
			TEXT("PrepareVMDBoneAnimData : Ref InParam is Null. DestSeq[%x],Skelton[%x],vmdMotionInfo[%x]"),
			DestSeq, Skeleton, vmdMotionInfo);
		// TBD:: ERR in Param...
		return false;
	}

	if (!ReNameTable)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log,
			TEXT("PrepareVMDBoneAnimData : Target ReNameTable is null."));
	}

	if (!mmdExtend)
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log,
			TEXT("PrepareVMDBoneAnimData : Target MMDExtendAsset is null."));
	}

	float ResampleRate = 30.f;

	auto& adc = DestSeq->GetController();
	adc.InitializeModel();

	const FFrameRate ResampleFrameRate(ResampleRate, 1);
	adc.SetFrameRate(ResampleFrameRate);

	const FFrameNumber NumberOfFrames = FGenericPlatformMath::Max<int32>((int32)vmdMotionInfo->maxFrame, 1);
	adc.SetNumberOfFrames(NumberOfFrames.Value, false);

	const int32 NumBones = Skeleton->GetReferenceSkeleton().GetNum();
	const TArray<FTransform>& RefBonePose = Skeleton->GetReferenceSkeleton().GetRefBonePose();

	OutBoneNames.Reset(NumBones);
	OutRawTracks.Reset(NumBones);

	// /* 添加调试输出：打印Skeleton中的所有骨骼名称 */
	// UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== Skeleton骨骼列表开始 ======"));
	// for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
	// {
	//     FName BoneName = Skeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);
	//     UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("Skeleton骨骼[%d]: %s "),
	//            BoneIndex, *BoneName.ToString());
	// }
	// UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== Skeleton骨骼列表结束 ======"));

	// /* 添加调试输出：打印VMD文件中的所有骨骼名称 */
	// UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== VMD骨骼列表开始 ======"));
	// for (int i = 0; i < vmdMotionInfo->keyBoneList.Num(); ++i)
	// {
	//     FString boneName = vmdMotionInfo->keyBoneList[i].TrackName;
	//     UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD骨骼[%d]: %s "),
	//            i, *boneName);
	// }
	// UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== VMD骨骼列表结束 ======"));

	// 添加调试输出：检查VMD中的骨骼是否在骨架中存在
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== 检查VMD骨骼在骨架中的存在情况 ======"));

		// 统计变量
		int32 totalVmdBones = vmdMotionInfo->keyBoneList.Num();
		int32 totalSkeletonBones = Skeleton->GetReferenceSkeleton().GetNum();
		int32 foundBones = 0;

		// 用于记录已找到的VMD骨骼索引
		TArray<int32> foundVmdIndices;

		// 遍历骨架中的每个骨骼
		for (int32 boneIndex = 0; boneIndex < totalSkeletonBones; ++boneIndex)
		{
			FName skeletonBoneName = Skeleton->GetReferenceSkeleton().GetBoneName(boneIndex);
			FString targetName = skeletonBoneName.ToString();

			// 应用可能的名称转换
			if (ReNameTable)
			{
				FMMD2UE5NameTableRow* dataRow;
				FString ContextString;
				dataRow = ReNameTable->FindRow<FMMD2UE5NameTableRow>(skeletonBoneName, ContextString);
				if (dataRow)
				{
					targetName = dataRow->MmdOriginalName;
				}
			}

			// 使用FindKeyTrackName检查骨骼是否存在于VMD数据中
			int32 vmdKeyListIndex = vmdMotionInfo->FindKeyTrackName(targetName,
				MMD4UE5::VmdMotionInfo::EVMD_KEYBONE);
			if (vmdKeyListIndex != -1)
			{
				// 记录找到的VMD骨骼索引
				foundVmdIndices.AddUnique(vmdKeyListIndex);
				foundBones++;
			}
		}

		// 计算VMD中存在但不在骨架中的骨骼数量
		int32 notFoundBones = totalVmdBones - foundVmdIndices.Num();

		// 输出未找到的VMD骨骼名称
		for (int i = 0; i < totalVmdBones; ++i)
		{
			if (!foundVmdIndices.Contains(i))
			{
				FString vmdBoneName = vmdMotionInfo->keyBoneList[i].TrackName;
				UE_LOG(LogMMD4UE5_VMDFactory, Warning,
					TEXT("VMD骨骼在骨架中未找到: [%s]"),
					*vmdBoneName);
			}
		}

		// 输出统计信息
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("骨架中共有 %d 根骨骼，VMD中共有 %d 根骨骼"),
			totalSkeletonBones, totalVmdBones);
		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("VMD骨骼在骨架中找到的有 %d 根，未找到的有 %d 根"),
			foundBones, notFoundBones);

		UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("====== VMD骨骼检查完成 ======"));
	}

	check(RefBonePose.Num() == NumBones);
	// 注册与Skeleton的Bone关系@必要事项
	for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
	{
		OutRawTracks.Add(FRawAnimSequenceTrack());
		check(BoneIndex == OutRawTracks.Num() - 1);
		FRawAnimSequenceTrack& RawTrack = OutRawTracks[BoneIndex];

		auto refTranslation = RefBonePose[BoneIndex].GetTranslation();

		FName targetName = Skeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);
		OutBoneNames.Add(targetName);

		FName* pn = NameMap.Find(targetName);
		if (pn)
			targetName = *pn;

		if (ReNameTable)
		{
			// 如果指定了转换表的资源，则从表中获取转换名称
			FMMD2UE5NameTableRow* dataRow;
			FString ContextString;
			dataRow = ReNameTable->FindRow<FMMD2UE5NameTableRow>(targetName, ContextString);
			if (dataRow)
			{
				targetName = FName(*dataRow->MmdOriginalName);
			}
		}

		// UE_LOG(LogMMD4UE5_VMDFactory, Warning, TEXT("%s"),*targetName.ToString());
		int vmdKeyListIndex = vmdMotionInfo->FindKeyTrackName(targetName.ToString(),
			MMD4UE5::VmdMotionInfo::EVMD_KEYBONE);
		if (vmdKeyListIndex == -1)
		{
			// {
			//     UE_LOG(LogMMD4UE5_VMDFactory, Warning,
			//            TEXT("PrepareVMDBoneAnimData Target Bone Not Found...[%s]"),
			//            *targetName.ToString());
			// }

			// nop
			// 设定与帧相同的值
			for (int32 i = 0; i < DestSeq->GetNumberOfSampledKeys(); i++)
			{
				FTransform nrmTrnc;
				nrmTrnc.SetIdentity();
				RawTrack.PosKeys.Add(FVector3f(nrmTrnc.GetTranslation() + refTranslation));
				RawTrack.RotKeys.Add(FQuat4f(nrmTrnc.GetRotation()));
				RawTrack.ScaleKeys.Add(FVector3f(nrmTrnc.GetScale3D()));
			}
		}
		else
		{
			check(vmdKeyListIndex > -1);
			int sortIndex = 0;
			int preKeyIndex = -1;
			auto& kybone = vmdMotionInfo->keyBoneList[vmdKeyListIndex];
			// if (kybone.keyList.Num() < 2)                    continue;
			int nextKeyIndex = kybone.sortIndexList[sortIndex];
			int nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;
			int baseKeyFrame = 0;

			// {
			//     UE_LOG(LogMMD4UE5_VMDFactory, Log,
			//            TEXT("PrepareVMDBoneAnimData Target Bone Found...Name[%s]-KeyNum[%d]"),
			//            *targetName.ToString(),
			//            kybone.sortIndexList.Num());
			// }

			bool dbg = false;
			if (targetName == L"右ひじ")
				dbg = true;
			// 事先针对各轨迹，在没有父Bone的情况下，在Local坐标下计算预定全部注册的帧（如果有更好的处理……讨论）
			// 如果进入90度以上的轴旋转，则由于四元数的原因或处理有错误而进入多余的旋转。
			// 通过上述方式，仅通过Z旋转（旋转运动），下半身和上半身的轴成为物理上不可能的旋转的组合。臭虫。

			// if (targetName == L"右足ＩＫ")
			// {
			//     UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("右足ＩＫ"));
			// }
			// if (targetName == L"左足ＩＫ")
			// {
			//     UE_LOG(LogMMD4UE5_VMDFactory, Log, TEXT("左足ＩＫ"));
			// }

			for (int32 i = 0; i < DestSeq->GetNumberOfSampledKeys(); i++)
			{
				if (i == 0)
				{
					if (i == nextKeyFrame)
					{
						FTransform tempTranceform(
							FQuat(
								kybone.keyList[nextKeyIndex].Quaternion[0],
								kybone.keyList[nextKeyIndex].Quaternion[2] * (-1),
								kybone.keyList[nextKeyIndex].Quaternion[1],
								kybone.keyList[nextKeyIndex].Quaternion[3]),
							FVector(
								kybone.keyList[nextKeyIndex].Position[0],
								kybone.keyList[nextKeyIndex].Position[2] * (-1),
								kybone.keyList[nextKeyIndex].Position[1])
								* 10.0f,
							FVector(1, 1, 1));
						// 将从引用姿势移动了Key的姿势的值作为初始值
						RawTrack.PosKeys.Add(FVector3f(tempTranceform.GetTranslation() + refTranslation));
						RawTrack.RotKeys.Add(FQuat4f(tempTranceform.GetRotation()));
						RawTrack.ScaleKeys.Add(FVector3f(tempTranceform.GetScale3D()));

						preKeyIndex = nextKeyIndex;
						uint32 lastKF = nextKeyFrame;
						while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[nextKeyIndex].Frame <= lastKF)
						{
							sortIndex++;
							nextKeyIndex = kybone.sortIndexList[sortIndex];
						}
						lastKF = nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;

						while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[kybone.sortIndexList[sortIndex + 1]].Frame == lastKF)
						{
							sortIndex++;
							nextKeyIndex = kybone.sortIndexList[sortIndex];
						}
						nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;
					}
					else
					{
						preKeyIndex = nextKeyIndex;
						// 例外处理。未为初始帧（0）设置关键帧
						FTransform nrmTrnc;
						nrmTrnc.SetIdentity();
						RawTrack.PosKeys.Add(FVector3f(nrmTrnc.GetTranslation() + refTranslation));
						RawTrack.RotKeys.Add(FQuat4f(nrmTrnc.GetRotation()));
						RawTrack.ScaleKeys.Add(FVector3f(nrmTrnc.GetScale3D()));
					}
				}
				else // if (nextKeyFrame == i)
				{
					float blendRate = 1;
					FTransform NextTranc;
					FTransform PreTranc;
					FTransform NowTranc;

					NextTranc.SetIdentity();
					PreTranc.SetIdentity();
					NowTranc.SetIdentity();

					if (nextKeyIndex > 0)
					{
						MMD4UE5::VMD_KEY& PreKey = kybone.keyList[preKeyIndex];
						MMD4UE5::VMD_KEY& NextKey = kybone.keyList[nextKeyIndex];
						if (NextKey.Frame <= (uint32)i)
						{
							blendRate = 1.0f;
						}
						else
						{
							// TBD:：帧间为1的话不以0.5计算吗？
							blendRate = 1.0f - (float)(NextKey.Frame - (uint32)i) / (float)(NextKey.Frame - PreKey.Frame);
						}
						// pose
						NextTranc.SetLocation(
							FVector(
								NextKey.Position[0],
								NextKey.Position[2] * (-1),
								NextKey.Position[1]));
						PreTranc.SetLocation(
							FVector(
								PreKey.Position[0],
								PreKey.Position[2] * (-1),
								PreKey.Position[1]));

						NowTranc.SetLocation(
							FVector(
								interpolateBezier(
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_X] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_X] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_X] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_X] / 127.0f,
									blendRate)
										* (NextTranc.GetTranslation().X - PreTranc.GetTranslation().X)
									+ PreTranc.GetTranslation().X,
								interpolateBezier(
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
									blendRate)
										* (NextTranc.GetTranslation().Y - PreTranc.GetTranslation().Y)
									+ PreTranc.GetTranslation().Y,
								interpolateBezier(
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
									NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
									blendRate)
										* (NextTranc.GetTranslation().Z - PreTranc.GetTranslation().Z)
									+ PreTranc.GetTranslation().Z));
						// rot
						NextTranc.SetRotation(
							FQuat(
								NextKey.Quaternion[0],
								NextKey.Quaternion[2] * (-1),
								NextKey.Quaternion[1],
								NextKey.Quaternion[3]));
						PreTranc.SetRotation(
							FQuat(
								PreKey.Quaternion[0],
								PreKey.Quaternion[2] * (-1),
								PreKey.Quaternion[1],
								PreKey.Quaternion[3]));

						float bezirT = interpolateBezier(
							NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_R] / 127.0f,
							NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_R] / 127.0f,
							NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_R] / 127.0f,
							NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_R] / 127.0f,
							blendRate);
						NowTranc.SetRotation(
							FQuat::Slerp(PreTranc.GetRotation(), NextTranc.GetRotation(), bezirT));
						/*UE_LOG(LogMMD4UE5_VMDFactory, Warning,
							TEXT("interpolateBezier Rot:[%s],F[%d/%d],BLD[%.2f],biz[%.2f]BEZ[%s]"),
							*targetName.ToString(), i, NextKey.Frame, blendRate, bezirT,*NowTranc.GetRotation().ToString()
							);*/
					}
					else
					{
						NowTranc.SetLocation(
							FVector(
								kybone.keyList[nextKeyIndex].Position[0],
								kybone.keyList[nextKeyIndex].Position[2] * (-1),
								kybone.keyList[nextKeyIndex].Position[1]));
						NowTranc.SetRotation(
							FQuat(
								kybone.keyList[nextKeyIndex].Quaternion[0],
								kybone.keyList[nextKeyIndex].Quaternion[2] * (-1),
								kybone.keyList[nextKeyIndex].Quaternion[1],
								kybone.keyList[nextKeyIndex].Quaternion[3]));
						// TBD:需要重新研究该路线存在的花纹、处理
						// check(false);
					}

					FTransform tempTranceform(
						NowTranc.GetRotation(),
						NowTranc.GetTranslation() * 10.0f,
						FVector(1, 1, 1));
					// 将从引用姿势移动了Key的姿势的值作为初始值
					RawTrack.PosKeys.Add(FVector3f(tempTranceform.GetTranslation() + refTranslation));
					RawTrack.RotKeys.Add(FQuat4f(tempTranceform.GetRotation()));
					RawTrack.ScaleKeys.Add(FVector3f(tempTranceform.GetScale3D()));

					if (nextKeyFrame == i)
					{
						preKeyIndex = nextKeyIndex;
						uint32 lastKF = nextKeyFrame;
						while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[nextKeyIndex].Frame <= lastKF)
						{
							sortIndex++;
							nextKeyIndex = kybone.sortIndexList[sortIndex];
						}
						lastKF = nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;

						while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[kybone.sortIndexList[sortIndex + 1]].Frame == lastKF)
						{
							sortIndex++;
							nextKeyIndex = kybone.sortIndexList[sortIndex];
						}
						nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;
					}
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// 创建以X轴为中心的旋转矩阵
void CreateRotationXMatrix(FMatrix* Out, float Angle)
{
	float Sin, Cos;

	//_SINCOS(Angle, &Sin, &Cos);
	//	Sin = sinf( Angle ) ;
	//	Cos = cosf( Angle ) ;
	Sin = FMath::Sin(Angle);
	Cos = FMath::Cos(Angle);

	//_MEMSET(Out, 0, sizeof(MATRIX));
	FMemory::Memzero(Out, sizeof(FMatrix));
	Out->M[0][0] = 1.0f;
	Out->M[1][1] = Cos;
	Out->M[1][2] = Sin;
	Out->M[2][1] = -Sin;
	Out->M[2][2] = Cos;
	Out->M[3][3] = 1.0f;

	// return 0;
}

// 求仅旋转分量矩阵的积（3）×3以外的部分也不代入值）
void MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(FMatrix* Out, FMatrix* In1, FMatrix* In2)
{
	Out->M[0][0] = In1->M[0][0] * In2->M[0][0] + In1->M[0][1] * In2->M[1][0] + In1->M[0][2] * In2->M[2][0];
	Out->M[0][1] = In1->M[0][0] * In2->M[0][1] + In1->M[0][1] * In2->M[1][1] + In1->M[0][2] * In2->M[2][1];
	Out->M[0][2] = In1->M[0][0] * In2->M[0][2] + In1->M[0][1] * In2->M[1][2] + In1->M[0][2] * In2->M[2][2];

	Out->M[1][0] = In1->M[1][0] * In2->M[0][0] + In1->M[1][1] * In2->M[1][0] + In1->M[1][2] * In2->M[2][0];
	Out->M[1][1] = In1->M[1][0] * In2->M[0][1] + In1->M[1][1] * In2->M[1][1] + In1->M[1][2] * In2->M[2][1];
	Out->M[1][2] = In1->M[1][0] * In2->M[0][2] + In1->M[1][1] * In2->M[1][2] + In1->M[1][2] * In2->M[2][2];

	Out->M[2][0] = In1->M[2][0] * In2->M[0][0] + In1->M[2][1] * In2->M[1][0] + In1->M[2][2] * In2->M[2][0];
	Out->M[2][1] = In1->M[2][0] * In2->M[0][1] + In1->M[2][1] * In2->M[1][1] + In1->M[2][2] * In2->M[2][1];
	Out->M[2][2] = In1->M[2][0] * In2->M[0][2] + In1->M[2][1] * In2->M[1][2] + In1->M[2][2] * In2->M[2][2];
}

/////////////////////////////////////
// 判定角度限制的共同函数（subIndexJdg的判定比较不明…）
void CheckLimitAngle(
	const FVector& RotMin,
	const FVector& RotMax,
	FVector* outAngle, // target angle ( in and out param)
	bool subIndexJdg   //(ik link index < ik loop temp):: linkBoneIndex < ikt
)
{
	// #define DEBUG_CheckLimitAngle
#ifdef DEBUG_CheckLimitAngle
	FVector debugVec = *outAngle;
#endif

	*outAngle = ClampVector(
		*outAngle,
		RotMin,
		RotMax);

	// debug
#ifdef DEBUG_CheckLimitAngle
	UE_LOG(LogMMD4UE5_VMDFactory, Log,
		TEXT("CheckLimitAngle::out[%s]<-In[%s]:MI[%s]MX[%s]"),
		*outAngle->ToString(),
		*debugVec.ToString(),
		*RotMin.ToString(),
		*RotMax.ToString());
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

/*****************
 * 从MMD侧的名称检索并取得TableRow的UE侧名称
 * Return :T is Found
 * @param :ue5Name is Found Row Name
 ****************/
bool UVmdFactory::FindTableRowMMD2UEName(
	UDataTable* ReNameTable,
	FName mmdName,
	FName* ue5Name)
{
	if (ReNameTable == NULL || ue5Name == NULL)
	{
		return false;
	}

	TArray<FName> getTableNames = ReNameTable->GetRowNames();

	FMMD2UE5NameTableRow* dataRow;
	FString ContextString;
	for (int i = 0; i < getTableNames.Num(); ++i)
	{
		ContextString = "";
		dataRow = ReNameTable->FindRow<FMMD2UE5NameTableRow>(getTableNames[i], ContextString);
		if (dataRow)
		{
			if (mmdName == FName(*dataRow->MmdOriginalName))
			{
				*ue5Name = getTableNames[i];
				return true;
			}
		}
	}
	return false;
}

/*****************
 * 从Bone名称中搜索并获取与RefSkelton匹配的BoneIndex
 * Return :index, -1 is not found
 * @param :TargetName is Target Bone Name
 ****************/
int32 UVmdFactory::FindRefBoneInfoIndexFromBoneName(
	const FReferenceSkeleton& RefSkelton,
	const FName& TargetName)
{
	for (int i = 0; i < RefSkelton.GetRefBoneInfo().Num(); ++i)
	{
		if (RefSkelton.GetRefBoneInfo()[i].Name == TargetName)
		{
			return i;
		}
	}
	return -1;
}

/*****************
 * 递归计算当前关键帧中指定Bone的Glb坐标
 * Return :trncform
 * @param :TargetName is Target Bone Name
 ****************/
FTransform UVmdFactory::CalcGlbTransformFromBoneIndex(
	UAnimSequence* DestSeq,
	USkeleton* Skeleton,
	int32 BoneIndex,
	int32 keyIndex)
{
	if (DestSeq == NULL || Skeleton == NULL || BoneIndex < 0 || keyIndex < 0)
	{
		// error root
		return FTransform::Identity;
	}

	auto& dat = DestSeq->GetDataModel()->GetBoneAnimationTracks()[BoneIndex].InternalTrackData;

	FTransform resultTrans(
		FQuat(dat.RotKeys[keyIndex]), // qt.X, qt.Y, qt.Z, qt.W),
		FVector(dat.PosKeys[keyIndex]),
		FVector(dat.ScaleKeys[keyIndex]));

	int ParentBoneIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(BoneIndex);
	if (ParentBoneIndex >= 0)
	{
		// found parent bone
		resultTrans *= CalcGlbTransformFromBoneIndex(
			DestSeq,
			Skeleton,
			ParentBoneIndex,
			keyIndex);
	}
	return resultTrans;
}
bool UVmdFactory::ImportVmdFromFile(FString file, USkeletalMesh* SkeletalMesh)
{
	initMmdNameMap();
	MMD4UE5::VmdMotionInfo vmdMotionInfo;

	UVmdFactory* MyFactory = NewObject<UVmdFactory>();
	TArray<uint8> File_Result;

	FString filepath = file;
	int32 indexs = -1;
	if (filepath.FindLastChar('\\', indexs))
	{
		filepath = filepath.Right(filepath.Len() - indexs - 1);
		if (filepath.FindLastChar('.', indexs))
		{
			filepath = filepath.Left(indexs);
			if (FPaths::FileExists(file))
			{

				if (FFileHelper::LoadFileToArray(File_Result, *file))
				{
					const uint8* DataPtr = File_Result.GetData();
					if (vmdMotionInfo.VMDLoaderBinary(DataPtr, NULL) == false)
					{
						UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error::vmd data load faile."));
						return false;
					}
					UAnimSequence* LastCreatedAnim = NULL;
					USkeleton* Skeleton = NULL;
					// UIKRigDefinition* IKRig = NULL;
					VMDImportOptions* ImportOptions = NULL;
					UDataTable* MMD2UE5NameTable = NULL;
					// UMMDExtendAsset* MMDasset = NULL;
					if (SkeletalMesh)
					{
						Skeleton = SkeletalMesh->GetSkeleton();

						LastCreatedAnim = MyFactory->ImportAnimations(
							Skeleton,
							SkeletalMesh,
							NULL,
							filepath,
							NULL,
							MMD2UE5NameTable,
							NULL,
							&vmdMotionInfo);
						return true;
					}
					else
					{
						UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error::SkeletalMesh is null."));
					}
				}
				else
				{
					UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error:: LoadFileToArray."));
				}
			}
			else
			{
				UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error:: FIle is not exist."));
			}
		}
		else
		{
			UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error::filepath type error."));
		}
	}
	else
	{
		UE_LOG(LogMMD4UE5_VMDFactory, Error, TEXT("!!!VMD Import error::filepath error.%d,%s"), indexs, *filepath);
	}

	return false;
}
#undef LOCTEXT_NAMESPACE
