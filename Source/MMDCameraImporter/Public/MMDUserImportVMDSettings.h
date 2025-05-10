// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "MMDUserImportVMDSettings.generated.h"

UENUM()
enum class ECameraCutImportType
{
    OneFrameInterval UMETA(DisplayName = "One Frame Interval (Best for Sequencer)"),
    ConstantKey UMETA(DisplayName = "Constant Key (MMD 60 frame animation method)"),
    OneFrameIntervalWithConstantKey UMETA(DisplayNAme = "One Frame Interval with Constant Key (For fps scalable environment e.g. game)"),
    ImportAsIs UMETA(DisplayName = "Import As Is (For 30 frame animation)"),
};

UENUM(BlueprintType)
enum class EVMDAxisMapping : uint8
{
    VMD_X UMETA(DisplayName = "VMD X"),
    VMD_Y UMETA(DisplayName = "VMD Y"),
    VMD_Z UMETA(DisplayName = "VMD Z"),
    VMD_NEG_X UMETA(DisplayName = "-VMD X"),
    VMD_NEG_Y UMETA(DisplayName = "-VMD Y"),
    VMD_NEG_Z UMETA(DisplayName = "-VMD Z")
};

// // 添加相机透视模式枚举
// UENUM(BlueprintType)
// enum class ECameraPerspectiveMode : uint8
// {
//     UseVMDSetting UMETA(DisplayName = "UseVMDSetting"),
//     ForcePerspective UMETA(DisplayName = "ForcePerspective"),
//     ForceOrthographic UMETA(DisplayName = "ForceOrthographic")
// };

USTRUCT()
struct FFilmbackImportSettings
{
    GENERATED_BODY()

    FFilmbackImportSettings();

    /** 数字胶片或传感器的水平尺寸，单位为毫米。 */
    UPROPERTY(EditAnywhere, config, meta = (ClampMin = "0.001", ForceUnits = mm, ToolTip = "Horizontal size of filmback or digital sensor, in mm.\n数字胶片或传感器的水平尺寸，单位为毫米。"))
    float SensorWidth;

    /** 数字胶片或传感器的垂直尺寸，单位为毫米。 */
    UPROPERTY(EditAnywhere, config, meta = (ClampMin = "0.001", ForceUnits = mm, ToolTip = "Vertical size of filmback or digital sensor, in mm.\n数字胶片或传感器的垂直尺寸，单位为毫米。"))
    float SensorHeight;
};

UCLASS(config = EditorSettings, BlueprintType)
class UMmdUserImportVmdSettings final : public UObject
{
public:
    explicit UMmdUserImportVmdSettings(const FObjectInitializer &Initializer);

    GENERATED_BODY()

    /** 导入统一缩放 */
    UPROPERTY(EditAnywhere, config, Category = Transform, meta = (ClampMin = "0.0", ToolTip = "Import Uniform Scale for VMD data.\n导入VMD数据的统一缩放比例。"))
    float ImportUniformScale;

    /** 初始视角偏移（度） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV & Focal Settings", meta = (ClampMin = "-90.0", ClampMax = "90.0", UIMin = "-90.0", UIMax = "90.0", ToolTip = "Initial view angle offset in degrees - Adjusts camera field of view.\n初始视角偏移（度）- 调整相机视野角度。"))
    float FOVOffset;

    /** 焦距缩放比例 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV & Focal Settings", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0", ToolTip = "Focal length scale - Controls the overall multiplier of focal length value calculated from FOV.\n焦距缩放比例 - 控制从FOV计算的焦距值的整体倍数。"))
    float FocalLengthScale;

    /** 相机距离偏移量 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings", meta = (ClampMin = "-1000.0", ClampMax = "1000.0", UIMin = "-100.0", UIMax = "100.0", ToolTip = "Distance offset - Adds a constant offset to camera distance.\n相机距离偏移量 - 为相机距离添加一个常量偏移值。"))
    float DistanceOffset;

    // /** 相机透视模式 */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective Mode Settings", meta = (ToolTip = "Determines whether to use VMD original settings or force a specific perspective mode.\n决定是使用VMD原始设定，还是强制使用特定的透视模式。"))
    // ECameraPerspectiveMode PerspectiveMode;

    // /** 正交模擬強度 - 控制相機拉遠距離的倍數 */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective Mode Settings",
    //           meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "50.0",
    //                   EditCondition = "PerspectiveMode != ECameraPerspectiveMode::ForcePerspective",
    //                   ToolTip = "正交模式使用拉遠相機模擬，當使用正交模式時，相機距離的倍數倍率。數值越大，正交效果越強"))
    // float OrthographicStrength;

    // /** 透視強度調整 - 控制透視模式下的視場角度縮放 */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective Mode Settings",
    //           meta = (ClampMin = "0.1", ClampMax = "5.0", UIMin = "0.1", UIMax = "3.0",
    //                   EditCondition = "PerspectiveMode != ECameraPerspectiveMode::ForceOrthographic",
    //                   ToolTip = "調整透視模式下的視場角度。值小於1時減弱透視效果，值大於1時加強透視效果"))
    // float PerspectiveStrength;

    /** 相机切换导入类型 */
    UPROPERTY(EditAnywhere, config, Category = KeyFrame, meta = (ToolTip = "Specifies how camera cuts should be imported.\n指定相机切换应如何导入。"))
    ECameraCutImportType CameraCutImportType;

    /** 相机数量 */
    UPROPERTY(EditAnywhere, config, Category = KeyFrame, meta = (ClampMin = "1", ClampMax = "4", ToolTip = "Number of cameras to import.\n要导入的相机数量。"))
    int CameraCount;

    /** 添加运动模糊关键帧 */
    UPROPERTY(EditAnywhere, config, Category = KeyFrame, meta = (ToolTip = "Whether to add motion blur keyframes.\n是否添加运动模糊关键帧。"))
    bool bAddMotionBlurKey;

    /** 运动模糊量 */
    UPROPERTY(EditAnywhere, config, Category = KeyFrame, meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bAddMotionBlurKey", ToolTip = "Amount of motion blur to apply.\n应用的运动模糊量。"))
    float MotionBlurAmount;

    /** 数字胶片设置 */
    UPROPERTY(EditAnywhere, config, Category = Camera, meta = (ShowOnlyInnerProperties, ToolTip = "Filmback settings for the camera.\n相机的数字胶片设置。"))
    FFilmbackImportSettings CameraFilmback;

    /** 距离轴映射 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate Mapping", meta = (DisplayName = "Distance Axis Mapping", ToolTip = "Distance axis mapping for VMD data.\n相机距离参数的轴向映射。"))
    EVMDAxisMapping DistanceAxisMapping;

    /** 坐标轴映射设定 X */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate Mapping", meta = (ToolTip = "Coordinate mapping for X axis.\n相机X轴坐标映射设定。"))
    EVMDAxisMapping AxisMappingX;

    /** 坐标轴映射设定 Y */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate Mapping", meta = (ToolTip = "Coordinate mapping for Y axis.\n相机Y轴坐标映射设定。"))
    EVMDAxisMapping AxisMappingY;

    /** 坐标轴映射设定 Z */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate Mapping", meta = (ToolTip = "Coordinate mapping for Z axis.\n相机Z轴坐标映射设定。"))
    EVMDAxisMapping AxisMappingZ;

    /** 相机旋转偏移量 X */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Adjustments", meta = (ToolTip = "Camera rotation offset for X axis.\n相机X轴旋转偏移量。"))
    float RotationOffsetX;

    /** 相机旋转偏移量 Y */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Adjustments", meta = (ToolTip = "Camera rotation offset for Y axis.\n相机Y轴旋转偏移量。"))
    float RotationOffsetY;

    /** 相机旋转偏移量 Z */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Adjustments", meta = (ToolTip = "Camera rotation offset for Z axis.\n相机Z轴旋转偏移量。"))
    float RotationOffsetZ;
};