// Copyright Epic Games, Inc. All Rights Reserved.

#include "MmdUserImportVmdSettings.h"
#include "UObject/UnrealType.h"

FFilmbackImportSettings::FFilmbackImportSettings()
{
	SensorWidth = 24.0f;
	SensorHeight = 13.5f;
}

UMmdUserImportVmdSettings::UMmdUserImportVmdSettings(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	ImportUniformScale = 0.08f;

	// 預設相機參數調整
	FOVOffset = 0.0f;		 // 初始視角偏移
	FocalLengthScale = 1.0f; // 焦距縮放比例

	// 相机距离控制参数的默认值
	DistanceOffset = 0.0f; // 默认不偏移距离

	// // 是否覆蓋相機透視模式
	// PerspectiveMode = ECameraPerspectiveMode::UseVMDSetting;
	// // 注意：在UE中，正交投影模式僅由基本CameraActor完全支持，而本插件使用的是CineCameraActor，暫時透過拉遠相機模擬正交投影
	// // 參見：https://forums.unrealengine.com/t/orthographic-projection-mode-in-camera-settings-ue5-4/1820866
	// OrthographicStrength = 10.0f; // 默認拉遠10倍來模擬正交
	// PerspectiveStrength = 1.0f;   // 默認保持原始透視強度

	CameraCutImportType = ECameraCutImportType::AdaptiveConstantKey; // 從AdaptiveOneFrameInterval改為AdaptiveConstantKey
	CameraCount = 1;												 // 從2改為1
	bAddMotionBlurKey = false;
	MotionBlurAmount = 0.5f;

	// 預設座標映射
	DistanceAxisMapping = EVMDAxisMapping::VMD_X;
	AxisMappingX = EVMDAxisMapping::VMD_X;
	AxisMappingY = EVMDAxisMapping::VMD_NEG_Z;
	AxisMappingZ = EVMDAxisMapping::VMD_Y;

	// 添加坐標系統調整參數 預設旋轉偏移
	RotationOffsetX = 0.0f;	  // X軸旋轉偏移
	RotationOffsetY = 0.0f;	  // Y軸旋轉偏移
	RotationOffsetZ = -90.0f; // Z軸旋轉偏移

	// 其他可添加參數
	// 插值曲線強度 (InterpolationStrength) 插值曲線調整 - VMD 使用貝塞爾曲線插值，可以提供全局參數來調整插值強度
	// 歐拉角旋轉順序 (RotationOrder) Roll Pitch Yaw 相機旋轉順序 - 允許用戶指定歐拉角旋轉順序(XYZ, ZXY 等)
}
