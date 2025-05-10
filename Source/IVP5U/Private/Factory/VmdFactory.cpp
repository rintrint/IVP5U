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

#define LOCTEXT_NAMESPACE "VMDImportFactory"

DEFINE_LOG_CATEGORY(LogMMD4UE4_VMDFactory)

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
void CreateRotationXMatrix(FMatrix *Out, float Angle);
// 求仅旋转分量矩阵的积（3）×3以外的部分也不代入值）
void MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(FMatrix *Out, FMatrix *In1, FMatrix *In2);
// 判定角度限制的共同函数（subIndexJdg的判定比较不明…）
void CheckLimitAngle(
    const FVector &RotMin,
    const FVector &RotMax,
    FVector *outAngle, // target angle ( in and out param)
    bool subIndexJdg   //(ik link index < ik loop temp):: linkBoneIndex < ikt
);
///////////////////////////////////////////////////////

UVmdFactory::UVmdFactory(const FObjectInitializer &ObjectInitializer)
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

bool UVmdFactory::DoesSupportClass(UClass *Class)
{
    return (Class == UVmdFactory::StaticClass());
}

UClass *UVmdFactory::ResolveSupportedClass()
{
    return UVmdFactory::StaticClass();
}

UObject *UVmdFactory::FactoryCreateBinary(
    UClass *Class,
    UObject *InParent,
    FName Name,
    EObjectFlags Flags,
    UObject *Context,
    const TCHAR *Type,
    const uint8 *&Buffer,
    const uint8 *BufferEnd,
    FFeedbackContext *Warn,
    bool &bOutOperationCanceled)
{
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始FactoryCreateBinary: Name=%s"), *Name.ToString());

    MMD4UE4::VmdMotionInfo vmdMotionInfo;

    if (vmdMotionInfo.VMDLoaderBinary(Buffer, BufferEnd) == false)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("VMD导入取消:: vmd数据加载失败"));
        return NULL;
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("VMD解析成功，keyBoneList=%d，keyFaceList=%d，keyCameraList=%d"),
           vmdMotionInfo.keyBoneList.Num(), vmdMotionInfo.keyFaceList.Num(), vmdMotionInfo.keyCameraList.Num());

    /////////////////////////////////////////
    UAnimSequence *LastCreatedAnim = NULL;
    USkeleton *Skeleton = NULL;
    USkeletalMesh *SkeletalMesh = NULL;
    UIKRigDefinition *IKRig = NULL;
    VMDImportOptions *ImportOptions = NULL;

    // 检查是否为相机动画
    if (vmdMotionInfo.keyCameraList.Num() == 0)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("非相机动画，准备导入骨骼和变形动画"));
        // 如果不是摄影机动画
        FVmdImporter *VmdImporter = FVmdImporter::GetInstance();

        EVMDImportType ForcedImportType = VMDIT_Animation;
        bool bOperationCanceled = false;
        bool bIsPmxFormat = true;
        // show Import Option Slate
        bool bImportAll = false;
        ImportUI->bIsObjImport = false; // anim mode
        ImportUI->OriginalImportType = EVMDImportType::VMDIT_Animation;
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

        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("获取导入选项结果: ImportOptions=%p, bOperationCanceled=%d"),
               ImportOptions, bOperationCanceled);

        /* 第一次判定 */
        if (ImportOptions)
        {
            Skeleton = ImportUI->Skeleton;
            SkeletalMesh = ImportUI->SkeletonMesh;

            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("从导入选项获取: Skeleton=%p, SkeletalMesh=%p"),
                   Skeleton, SkeletalMesh);

            /* 最低限度的参数设置检查 */
            if ((!Skeleton) || (!SkeletalMesh) || (Skeleton != SkeletalMesh->GetSkeleton()))
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Warning, TEXT("导入选项参数检查不通过，重新获取导入选项"));

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
            }
        }

        if (ImportOptions)
        {
            Skeleton = ImportUI->Skeleton;
            SkeletalMesh = ImportUI->SkeletonMesh;
            IKRig = ImportUI->IKRig;

            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("最终导入选项: Skeleton=%p, SkeletalMesh=%p, IKRig=%p"),
                   Skeleton, SkeletalMesh, IKRig);

            bool preParamChk = true;
            /*包含关系检查*/
            if (SkeletalMesh)
            {
                if (Skeleton != SkeletalMesh->GetSkeleton())
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("错误: Skeleton不等于SkeletalMesh->GetSkeleton()"));
                    preParamChk = false;
                }
            }

            if (preParamChk)
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("参数检查通过，开始导入动画"));

                ////////////////////////////////////
                if (!ImportOptions->AnimSequenceAsset)
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("创建新的AnimSequence资产"));
                    // create AnimSequence Asset from VMD
                    LastCreatedAnim = ImportAnimations(
                        Skeleton,
                        SkeletalMesh,
                        InParent,
                        Name.ToString(),
                        IKRig,
                        ImportUI->MMD2UE4NameTableRow,
                        ImportUI->MmdExtendAsset,
                        &vmdMotionInfo);
                }
                else
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("向现有AnimSequence添加变形曲线，AnimSequenceAsset=%p"),
                           ImportOptions->AnimSequenceAsset);

                    // add morph curve only to exist ainimation
                    LastCreatedAnim = AddtionalMorphCurveImportToAnimations(
                        SkeletalMesh,
                        ImportOptions->AnimSequenceAsset, // UAnimSequence* exsistAnimSequ,
                        ImportUI->MMD2UE4NameTableRow,
                        &vmdMotionInfo);
                }
            }
            else
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("参数检查失败，导入错误"));
            }
        }
        else
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Warning, TEXT("VMD导入取消"));
        }
    }
    else
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Warning, TEXT("相机动画导入未实现"));
        LastCreatedAnim = NULL;
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("FactoryCreateBinary结束，LastCreatedAnim=%p"), LastCreatedAnim);
    return LastCreatedAnim;
};

UAnimSequence *UVmdFactory::ImportAnimations(
    USkeleton *Skeleton,
    USkeletalMesh *SkeletalMesh,
    UObject *Outer,
    const FString &Name,
    UIKRigDefinition *IKRig,
    UDataTable *ReNameTable,
    UMMDExtendAsset *mmdExtend,
    MMD4UE4::VmdMotionInfo *vmdMotionInfo)
{
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始ImportAnimations: Name=%s"), *Name);

    UAnimSequence *LastCreatedAnim = NULL;

    // we need skeleton to create animsequence
    if (Skeleton == NULL)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("ImportAnimations: Skeleton为空"));
        return NULL;
    }

    {
        FString SequenceName = Name;

        SequenceName += "_";
        SequenceName += Skeleton->GetName();

        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("序列名称: %s"), *SequenceName);
        SequenceName = ObjectTools::SanitizeObjectName(SequenceName);
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("清理后序列名称: %s"), *SequenceName);

        FString animpath = SkeletalMesh->GetPathName();
        int32 indexs = -1;
        if (animpath.FindLastChar('/', indexs))
        {
            animpath = animpath.Left(indexs);
        }
        else
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("获取动画路径失败"));
            return NULL;
        }

        FString ParentPath;
        if (Outer)
        {
            ParentPath = FString::Printf(TEXT("%s/%s"), *FPackageName::GetLongPackagePath(*Outer->GetName()), *SequenceName);
        }
        else
        {
            ParentPath = FString::Printf(TEXT("%s/%s"), *animpath, *SequenceName);
        }

        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("动画路径: %s"), *ParentPath);

        UObject *ParentPackage = CreatePackage(*ParentPath);
        UObject *Object = LoadObject<UObject>(ParentPackage, *SequenceName, NULL, LOAD_None, NULL);
        UAnimSequence *DestSeq = Cast<UAnimSequence>(Object);

        if (Object && !DestSeq)
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("同名资产已存在但不是AnimSequence类型"));
            return LastCreatedAnim;
        }

        // If not, create new one now.
        if (!DestSeq)
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("创建新的AnimSequence: %s"), *SequenceName);
            DestSeq = NewObject<UAnimSequence>(ParentPackage, *SequenceName, RF_Public | RF_Standalone);

            // Notify the asset registry
            FAssetRegistryModule::AssetCreated(DestSeq);
        }
        else
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("重置现有AnimSequence: %s"), *SequenceName);
            DestSeq->ResetAnimation();
        }

        DestSeq->SetSkeleton(Skeleton);
        LastCreatedAnim = DestSeq;
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("AnimSequence创建结果: %p"), LastCreatedAnim);

    ///////////////////////////////////
    // Create RawCurve -> Track Curve Key
    //////////////////////

    if (LastCreatedAnim)
    {
        bool importSuccessFlag = true;

        /* vmd animation regist*/
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始导入VMD骨骼动画"));
        if (!ImportVMDBoneToAnimSequence(LastCreatedAnim, Skeleton, ReNameTable, IKRig, mmdExtend, vmdMotionInfo))
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("ImportVMDBoneToAnimSequence失败"));
            importSuccessFlag = false;
        }

        /* morph animation regist*/
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始导入VMD变形动画"));
        if (!ImportMorphCurveToAnimSequence(LastCreatedAnim, Skeleton, SkeletalMesh, ReNameTable, vmdMotionInfo))
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("ImportMorphCurveToAnimSequence失败"));
            importSuccessFlag = false;
        }

        /*Import正常時PreviewMesh更新*/
        if ((importSuccessFlag) && (SkeletalMesh))
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("设置预览网格"));
            LastCreatedAnim->SetPreviewMesh(SkeletalMesh);
        }
    }

    /////////////////////////////////////////
    // end process?
    ////////////////////////////////////////
    if (LastCreatedAnim)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("完成动画资产处理"));

        auto &adc = LastCreatedAnim->GetController();
        adc.OpenBracket(LOCTEXT("ImportAsSkeletalMesh", "Importing VMD Animation"));

        adc.UpdateCurveNamesFromSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
        adc.NotifyPopulated();
        adc.CloseBracket();

        // mark package as dirty
        MarkPackageDirty();
        SkeletalMesh->MarkPackageDirty();

        // 添加后处理以确保动画资料正确 (compress)
        // otherwise just compress
        // LastCreatedAnim->PostProcessSequence();
        // 使用默认的压缩设置
        LastCreatedAnim->BoneCompressionSettings = FAnimationUtils::GetDefaultAnimationBoneCompressionSettings();
        LastCreatedAnim->CurveCompressionSettings = FAnimationUtils::GetDefaultAnimationCurveCompressionSettings();
        // 尝试重新压缩动画数据
        LastCreatedAnim->RequestSyncAnimRecompression(false);
        // 确保初始化正确
        LastCreatedAnim->Modify();

        LastCreatedAnim->PostEditChange();
        LastCreatedAnim->SetPreviewMesh(SkeletalMesh);
        LastCreatedAnim->MarkPackageDirty();

        Skeleton->SetPreviewMesh(SkeletalMesh);
        Skeleton->PostEditChange();
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("ImportAnimations结束，LastCreatedAnim=%p"), LastCreatedAnim);
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
UAnimSequence *UVmdFactory::AddtionalMorphCurveImportToAnimations(
    USkeletalMesh *SkeletalMesh,
    UAnimSequence *exsistAnimSequ,
    UDataTable *ReNameTable,
    MMD4UE4::VmdMotionInfo *vmdMotionInfo)
{
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始AddtionalMorphCurveImportToAnimations，exsistAnimSequ=%p"), exsistAnimSequ);

    USkeleton *Skeleton = NULL;
    // we need skeleton to create animsequence
    if (exsistAnimSequ == NULL)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("exsistAnimSequ为空"));
        return NULL;
    }

    {
        Skeleton = exsistAnimSequ->GetSkeleton();
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("获取Skeleton=%p"), Skeleton);
    }

    ///////////////////////////////////
    // Create RawCurve -> Track Curve Key
    //////////////////////
    if (exsistAnimSequ)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("准备导入变形曲线"));
        /////////////////////////////////
        if (!ImportMorphCurveToAnimSequence(
                exsistAnimSequ,
                Skeleton,
                SkeletalMesh,
                ReNameTable,
                vmdMotionInfo))
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("ImportMorphCurveToAnimSequence失败"));
        }
    }

    /////////////////////////////////////////
    // end process?
    ////////////////////////////////////////
    if (exsistAnimSequ)
    {
        bool existAsset = true;
        /***********************/
        // refresh TrackToskeletonMapIndex
        // exsistAnimSequ->RefreshTrackMapFromAnimTrackNames();
        if (existAsset)
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("更新已存在的资产"));
            // exsistAnimSequ->BakeTrackCurvesToRawAnimation();
        }
        else
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("更新控制器"));
            // otherwise just compress
            // exsistAnimSequ->PostProcessSequence();

            auto &adc = exsistAnimSequ->GetController();
            adc.OpenBracket(LOCTEXT("ImportAsSkeletalMesh", "Importing VMD Animation"));

            adc.UpdateCurveNamesFromSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
            adc.NotifyPopulated();

            adc.CloseBracket();

            // mark package as dirty
            MarkPackageDirty();
            SkeletalMesh->MarkPackageDirty();

            exsistAnimSequ->PostEditChange();
            exsistAnimSequ->SetPreviewMesh(SkeletalMesh);
            exsistAnimSequ->MarkPackageDirty();

            Skeleton->SetPreviewMesh(SkeletalMesh);
            Skeleton->PostEditChange();
        }
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("AddtionalMorphCurveImportToAnimations结束，返回exsistAnimSequ=%p"), exsistAnimSequ);
    return exsistAnimSequ;
}
/*******************
 *导入Morph目标AnimCurve
 *将Morphtarget FloatCurve从VMD文件数据导入AnimSeq
 **********************/
bool UVmdFactory::ImportMorphCurveToAnimSequence(
    UAnimSequence *DestSeq,
    USkeleton *Skeleton,
    USkeletalMesh *SkeletalMesh,
    UDataTable *ReNameTable,
    MMD4UE4::VmdMotionInfo *vmdMotionInfo)
{
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始ImportMorphCurveToAnimSequence: DestSeq=%p, Skeleton=%p"), DestSeq, Skeleton);

    if (!DestSeq || !Skeleton || !vmdMotionInfo)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("参数错误：DestSeq=%p, Skeleton=%p, vmdMotionInfo=%p"),
               DestSeq, Skeleton, vmdMotionInfo);
        return false;
    }

    USkeletalMesh *mesh = SkeletalMesh;
    if (!mesh)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("ImportMorphCurveToAnimSequence GetAssetPreviewMesh未找到"));
        return false;
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始处理Morph曲线，获取动画控制器"));
    auto &adc = DestSeq->GetController();

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("开始遍历表情关键帧列表，总数: %d"), vmdMotionInfo->keyFaceList.Num());

    for (int i = 0; i < vmdMotionInfo->keyFaceList.Num(); ++i)
    {
        MMD4UE4::VmdFaceTrackList *vmdFaceTrackPtr = &vmdMotionInfo->keyFaceList[i];

        // 获取原始名称
        FName Name = *vmdFaceTrackPtr->TrackName;
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("处理表情[%d]: %s"), i, *Name.ToString());

        if (ReNameTable)
        {
            // 如果指定了转换表，则获取转换后的名称
            FMMD2UE4NameTableRow *dataRow;
            FString ContextString;
            dataRow = ReNameTable->FindRow<FMMD2UE4NameTableRow>(Name, ContextString);
            if (dataRow)
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  表情名称从转换表映射: %s -> %s"),
                       *Name.ToString(), *dataRow->MmdOriginalName);
                Name = FName(*dataRow->MmdOriginalName);
            }
        }

        if (mesh != NULL)
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  查找变形目标: %s"), *Name.ToString());
            UMorphTarget *morphTargetPtr = mesh->FindMorphTarget(Name);
            if (!morphTargetPtr)
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Warning,
                       TEXT("  未找到变形目标...搜索[%s]VMD原始名称[%s]"),
                       *Name.ToString(), *vmdFaceTrackPtr->TrackName);
            }
            else
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  找到变形目标: %s"), *Name.ToString());

                if (vmdFaceTrackPtr->keyList.Num() > 0 &&
                    !(vmdFaceTrackPtr->keyList.Num() == 1 && vmdFaceTrackPtr->keyList[0].Factor == 0.0f))
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  创建曲线标识符，名称=%s"), *Name.ToString());

                    // 直接创建曲线标识符的方式在UE5.5中发生了变化
                    FAnimationCurveIdentifier CurveId(Name, ERawCurveTrackTypes::RCT_Float);

                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  曲线标识符: CurveName=%s, CurveType=%d"),
                           *CurveId.CurveName.ToString(), (int)CurveId.CurveType);

                    // 检查曲线名称是否有效
                    if (CurveId.IsValid())
                    {
                        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  曲线标识符有效，尝试添加曲线"));
                        TArray<FRichCurveKey> keyarrys;

                        bool addResult = adc.AddCurve(CurveId);
                        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  AddCurve结果: %s"),
                               addResult ? TEXT("成功") : TEXT("失败"));

                        if (addResult)
                        {
                            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  开始添加关键帧，总数: %d"),
                                   vmdFaceTrackPtr->keyList.Num());

                            MMD4UE4::VMD_FACE_KEY *faceKeyPtr = NULL;
                            for (int s = 0; s < vmdFaceTrackPtr->keyList.Num(); ++s)
                            {
                                check(vmdFaceTrackPtr->sortIndexList[s] < vmdFaceTrackPtr->keyList.Num());
                                faceKeyPtr = &vmdFaceTrackPtr->keyList[vmdFaceTrackPtr->sortIndexList[s]];
                                check(faceKeyPtr);

                                float SequenceLength = DestSeq->GetPlayLength();
                                float timeCurve = faceKeyPtr->Frame / 30.0f;

                                if (timeCurve > SequenceLength)
                                {
                                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  关键帧时间 %f 超出序列长度 %f，停止添加"),
                                           timeCurve, SequenceLength);
                                    break;
                                }

                                keyarrys.Add(FRichCurveKey(timeCurve, faceKeyPtr->Factor));
                            }

                            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  设置曲线关键帧，数量: %d"), keyarrys.Num());
                            adc.SetCurveKeys(CurveId, keyarrys);

                            UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  成功添加曲线[%s]关键帧数[%d]"),
                                   *Name.ToString(), vmdFaceTrackPtr->keyList.Num());
                        }
                        else
                        {
                            UE_LOG(LogMMD4UE4_VMDFactory, Warning, TEXT("  添加曲线失败[%s]关键帧数[%d]"),
                                   *Name.ToString(), vmdFaceTrackPtr->keyList.Num());
                        }
                    }
                    else
                    {
                        UE_LOG(LogMMD4UE4_VMDFactory, Warning, TEXT("  曲线标识符无效[%s]"), *Name.ToString());
                    }
                }
                else
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  关键帧数量不足或只有一个值为0的关键帧，跳过: %d"),
                           vmdFaceTrackPtr->keyList.Num());
                }
            }
        }

        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("  标记动画序列已修改"));
        DestSeq->Modify();
    }

    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("表情曲线导入完成"));
    return true;
}

/*******************
 * Import VMD Animation
 * 从VMD文件的数据将运动数据导入AnimSeq
 **********************/
bool UVmdFactory::ImportVMDBoneToAnimSequence(
    UAnimSequence *DestSeq,
    USkeleton *Skeleton,
    UDataTable *ReNameTable,
    UIKRigDefinition *IKRig,
    UMMDExtendAsset *mmdExtend,
    MMD4UE4::VmdMotionInfo *vmdMotionInfo)
{
    // nullptr check in-param
    if (!DestSeq || !Skeleton || !vmdMotionInfo)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error,
               TEXT("ImportVMDBoneToAnimSequence : Ref InParam is Null. DestSeq[%x],Skelton[%x],vmdMotionInfo[%x]"),
               DestSeq, Skeleton, vmdMotionInfo);
        // TBD:: ERR in Param...
        return false;
    }
    if (!ReNameTable)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Warning,
               TEXT("ImportVMDBoneToAnimSequence : Target ReNameTable is null."));
    }
    if (!mmdExtend)
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Warning,
               TEXT("ImportVMDBoneToAnimSequence : Target MMDExtendAsset is null."));
    }

    float ResampleRate = 30.f;

    auto &adc = DestSeq->GetController();

    adc.InitializeModel();
    // adc.OpenBracket(LOCTEXT("AddNewRawTrack_Bracket", "Adding new Bone Animation Track"));

    const FFrameRate ResampleFrameRate(ResampleRate, 1);
    adc.SetFrameRate(ResampleFrameRate);

    const FFrameNumber NumberOfFrames = FGenericPlatformMath::Max<int32>((int32)vmdMotionInfo->maxFrame, 1);
    adc.SetNumberOfFrames(NumberOfFrames.Value, false);

    const int32 NumBones = Skeleton->GetReferenceSkeleton().GetNum();

    const TArray<FTransform> &RefBonePose = Skeleton->GetReferenceSkeleton().GetRefBonePose();

    TArray<FRawAnimSequenceTrack> TempRawTrackList;

    /* 添加调试输出：打印Skeleton中的所有骨骼名称 */
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("====== Skeleton骨骼列表开始 ======"));
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        FName BoneName = Skeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("Skeleton骨骼[%d]: %s"),
               BoneIndex, *BoneName.ToString());
    }
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("====== Skeleton骨骼列表结束 ======"));

    /* 添加调试输出：打印VMD文件中的所有骨骼名称 */
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("====== VMD骨骼列表开始 ======"));
    for (int i = 0; i < vmdMotionInfo->keyBoneList.Num(); ++i)
    {
        FString boneName = vmdMotionInfo->keyBoneList[i].TrackName;
        UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("VMD骨骼[%d]: %s"),
               i, *boneName);
    }
    UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("====== VMD骨骼列表结束 ======"));

    check(RefBonePose.Num() == NumBones);
    // 注册与Skeleton的Bone关系@必要事项
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        TempRawTrackList.Add(FRawAnimSequenceTrack());
        check(BoneIndex == TempRawTrackList.Num() - 1);
        FRawAnimSequenceTrack &RawTrack = TempRawTrackList[BoneIndex];

        auto refTranslation = RefBonePose[BoneIndex].GetTranslation();

        FName targetName = Skeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);
        ;
        FName *pn = NameMap.Find(targetName);
        if (pn)
            targetName = *pn;

        if (ReNameTable)
        {
            // 如果指定了转换表的资源，则从表中获取转换名称
            FMMD2UE4NameTableRow *dataRow;
            FString ContextString;
            dataRow = ReNameTable->FindRow<FMMD2UE4NameTableRow>(targetName, ContextString);
            if (dataRow)
            {
                targetName = FName(*dataRow->MmdOriginalName);
            }
        }

        // UE_LOG(LogTemp, Warning, TEXT("%s"),*targetName.ToString());
        int vmdKeyListIndex = vmdMotionInfo->FindKeyTrackName(targetName.ToString(),
                                                              MMD4UE4::VmdMotionInfo::EVMD_KEYBONE);
        if (vmdKeyListIndex == -1)
        {
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Warning,
                       TEXT("ImportVMDBoneToAnimSequence Target Bone Not Found...[%s]"),
                       *targetName.ToString());
            }
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
            auto &kybone = vmdMotionInfo->keyBoneList[vmdKeyListIndex];
            // if (kybone.keyList.Num() < 2)					continue;
            int nextKeyIndex = kybone.sortIndexList[sortIndex];
            int nextKeyFrame = kybone.keyList[nextKeyIndex].Frame;
            int baseKeyFrame = 0;

            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log,
                       TEXT("ImportVMDBoneToAnimSequence Target Bone Found...Name[%s]-KeyNum[%d]"),
                       *targetName.ToString(),
                       kybone.sortIndexList.Num());
            }
            bool dbg = false;
            if (targetName == L"右ひじ")
                dbg = true;
            // 事先针对各轨迹，在没有父Bone的情况下，在Local坐标下计算预定全部注册的帧（如果有更好的处理……讨论）
            // 如果进入90度以上的轴旋转，则由于四元数的原因或处理有错误而进入多余的旋转。
            // 通过上述方式，仅通过Z旋转（旋转运动），下半身和上半身的轴成为物理上不可能的旋转的组合。臭虫。

            if (targetName == L"右足ＩＫ")
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("右足ＩＫ"));
            }
            if (targetName == L"左足ＩＫ")
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Log, TEXT("左足ＩＫ"));
            }

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
                                kybone.keyList[nextKeyIndex].Position[1]) *
                                10.0f,
                            FVector(1, 1, 1));
                        // 将从引用姿势移动了Key的姿势的值作为初始值
                        RawTrack.PosKeys.Add(FVector3f(tempTranceform.GetTranslation() + refTranslation));
                        RawTrack.RotKeys.Add(FQuat4f(tempTranceform.GetRotation()));
                        RawTrack.ScaleKeys.Add(FVector3f(tempTranceform.GetScale3D()));

                        preKeyIndex = nextKeyIndex;
                        uint32 lastKF = nextKeyFrame;
                        while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[nextKeyIndex].Frame <=
                                                                                 lastKF)
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
                        MMD4UE4::VMD_KEY &PreKey = kybone.keyList[preKeyIndex];
                        MMD4UE4::VMD_KEY &NextKey = kybone.keyList[nextKeyIndex];
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
                                    blendRate) *
                                        (NextTranc.GetTranslation().X - PreTranc.GetTranslation().X) +
                                    PreTranc.GetTranslation().X,
                                interpolateBezier(
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Z] / 127.0f,
                                    blendRate) *
                                        (NextTranc.GetTranslation().Y - PreTranc.GetTranslation().Y) +
                                    PreTranc.GetTranslation().Y,
                                interpolateBezier(
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_0][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_X][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
                                    NextKey.Bezier[D_VMD_KEY_BEZIER_AR_0_BEZ_1][D_VMD_KEY_BEZIER_AR_1_BEZ_Y][D_VMD_KEY_BEZIER_AR_2_KND_Y] / 127.0f,
                                    blendRate) *
                                        (NextTranc.GetTranslation().Z - PreTranc.GetTranslation().Z) +
                                    PreTranc.GetTranslation().Z));
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
                        /*UE_LOG(LogMMD4UE4_VMDFactory, Warning,
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
                        while (sortIndex + 1 < kybone.sortIndexList.Num() && kybone.keyList[nextKeyIndex].Frame <=
                                                                                 lastKF)
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
    adc.OpenBracket(LOCTEXT("AddNewRawTrack_Bracket", "Adding new Bone Animation Track"));
    /* AddTrack */
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        FName BoneName = Skeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);

        FRawAnimSequenceTrack &RawTrack = TempRawTrackList[BoneIndex];

        // DestSeq->AddNewRawTrack(BoneName, &RawTrack);

        int32 NewTrackIndex = INDEX_NONE;
        if (RawTrack.PosKeys.Num() > 1)
        {
            bool is_sucess = false; // 写法改变

            is_sucess = adc.AddBoneCurve(BoneName);

            if (is_sucess)
            {
                if (BoneName == L"腰")
                {
                    for (int32 ix = 0; ix < RawTrack.PosKeys.Num(); ix++)
                    {
                        // RawTrack.PosKeys[ix] = FVector3f(0.0f,0.0f,0.0f);
                        RawTrack.PosKeys[ix].X = 0.0f;
                        RawTrack.PosKeys[ix].Y = 0.0f;
                        RawTrack.RotKeys[ix] = FQuat4f(0.0f, 0.0f, 0.0f, 1.0f);
                    }
                    // UE_LOG(LogTemp, Warning, TEXT("%f,%f,%f"), RawTrack.PosKeys[1].X, RawTrack.PosKeys[1].Y, RawTrack.PosKeys[1].Z);//看看有多少个
                }

                adc.SetBoneTrackKeys(BoneName, RawTrack.PosKeys, RawTrack.RotKeys, RawTrack.ScaleKeys);
            }
        }
    }

    adc.UpdateCurveNamesFromSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
    adc.NotifyPopulated();

    adc.CloseBracket();
    // GWarn->EndSlowTask();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// 创建以X轴为中心的旋转矩阵
void CreateRotationXMatrix(FMatrix *Out, float Angle)
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
void MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(FMatrix *Out, FMatrix *In1, FMatrix *In2)
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
    const FVector &RotMin,
    const FVector &RotMax,
    FVector *outAngle, // target angle ( in and out param)
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
    UE_LOG(LogMMD4UE4_VMDFactory, Log,
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
 * @param :ue4Name is Found Row Name
 ****************/
bool UVmdFactory::FindTableRowMMD2UEName(
    UDataTable *ReNameTable,
    FName mmdName,
    FName *ue4Name)
{
    if (ReNameTable == NULL || ue4Name == NULL)
    {
        return false;
    }

    TArray<FName> getTableNames = ReNameTable->GetRowNames();

    FMMD2UE4NameTableRow *dataRow;
    FString ContextString;
    for (int i = 0; i < getTableNames.Num(); ++i)
    {
        ContextString = "";
        dataRow = ReNameTable->FindRow<FMMD2UE4NameTableRow>(getTableNames[i], ContextString);
        if (dataRow)
        {
            if (mmdName == FName(*dataRow->MmdOriginalName))
            {
                *ue4Name = getTableNames[i];
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
    const FReferenceSkeleton &RefSkelton,
    const FName &TargetName)
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
    UAnimSequence *DestSeq,
    USkeleton *Skeleton,
    int32 BoneIndex,
    int32 keyIndex)
{
    if (DestSeq == NULL || Skeleton == NULL || BoneIndex < 0 || keyIndex < 0)
    {
        // error root
        return FTransform::Identity;
    }

    auto &dat = DestSeq->GetDataModel()->GetBoneAnimationTracks()[BoneIndex].InternalTrackData;

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
bool UVmdFactory::ImportVmdFromFile(FString file, USkeletalMesh *SkeletalMesh)
{
    initMmdNameMap();
    MMD4UE4::VmdMotionInfo vmdMotionInfo;

    UVmdFactory *MyFactory = NewObject<UVmdFactory>();
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
                    const uint8 *DataPtr = File_Result.GetData();
                    if (vmdMotionInfo.VMDLoaderBinary(DataPtr, NULL) == false)
                    {
                        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error::vmd data load faile."));
                        return false;
                    }
                    UAnimSequence *LastCreatedAnim = NULL;
                    USkeleton *Skeleton = NULL;
                    // UIKRigDefinition* IKRig = NULL;
                    VMDImportOptions *ImportOptions = NULL;
                    UDataTable *MMD2UE4NameTable = NULL;
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
                            MMD2UE4NameTable,
                            NULL,
                            &vmdMotionInfo);
                        return true;
                    }
                    else
                    {
                        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error::SkeletalMesh is null."));
                    }
                }
                else
                {
                    UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error:: LoadFileToArray."));
                }
            }
            else
            {
                UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error:: FIle is not exist."));
            }
        }
        else
        {
            UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error::filepath type error."));
        }
    }
    else
    {
        UE_LOG(LogMMD4UE4_VMDFactory, Error, TEXT("!!!VMD Import error::filepath error.%d,%s"), indexs, *filepath);
    }

    return false;
}
#undef LOCTEXT_NAMESPACE
