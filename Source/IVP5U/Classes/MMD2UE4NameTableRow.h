// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine/DataTable.h"

#include "MMD2UE4NameTableRow.generated.h"

/**
 *从MMD系统转换为面向UE4的数据时的表定义
 *->VMD导入时追加指定该表作为MMD->UE4的翻译使用
 *->CSV导入时使用日语时请用UTF-8保存
 *@param Name（默认Tag）：UE4侧名称（记载骨骼名称、Morphtarget名称）
 *@param MMDOriginalName:MMD侧名称（输入骨骼名称、表情名称）
 */
USTRUCT()
struct FMMD2UE4NameTableRow : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMD")
    FString MmdOriginalName;
};
