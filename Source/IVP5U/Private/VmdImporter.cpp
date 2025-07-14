// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#include "VmdImporter.h"
#include "IVP5UPrivatePCH.h"
#include "MMDImportHelper.h"

#include "Animation/AnimSequenceBase.h"

namespace MMD4UE5
{

	DEFINE_LOG_CATEGORY(LogMMD4UE5_VmdMotionInfo)

	VmdMotionInfo::VmdMotionInfo()
	{
		maxFrame = 0;
		minFrame = 0;
	}

	VmdMotionInfo::~VmdMotionInfo()
	{
	}

	bool VmdMotionInfo::VMDLoaderBinary(
		const uint8*& Buffer,
		const uint8* BufferEnd)
	{
		VmdReadMotionData readData;
		{
			uint32 memcopySize = 0;
			float modelScale = 8.0f;

			memcopySize = sizeof(readData.vmdHeader);
			FMemory::Memcpy(&readData.vmdHeader, Buffer, memcopySize);
			// 确定是否为VMD文件的临时版本
			if (readData.vmdHeader.header[0] == 'V' && readData.vmdHeader.header[1] == 'o' && readData.vmdHeader.header[2] == 'c')
			{
				UE_LOG(LogMMD4UE5_VmdMotionInfo, Log, TEXT("VMD Import START /Correct Magic[Vocaloid Motion Data 0002]"));
			}
			else
			{
				// UE_LOG(LogMMD4UE5_PmdMeshInfo, Error, TEXT("PMX Import FALSE/Return /UnCorrect Magic[PMX]"));
				return false;
			}
			Buffer += memcopySize;

			// 设置每个数据的起始地址
			{
				// Key VMD
				memcopySize = sizeof(readData.vmdKeyCount);
				FMemory::Memcpy(&readData.vmdKeyCount, Buffer, memcopySize);
				Buffer += memcopySize;

				// 优化：一次性分配所有内存
				readData.vmdKeyList.SetNumZeroed(readData.vmdKeyCount);

				// 优化：批量读取数据
				for (int32 i = 0; i < readData.vmdKeyCount; ++i)
				{
					VMD_KEY* vmdKeyPtr = &readData.vmdKeyList[i];

					// Bone NAME
					memcopySize = sizeof(vmdKeyPtr->Name);
					FMemory::Memcpy(&vmdKeyPtr->Name[0], Buffer, memcopySize);
					Buffer += memcopySize;

					// Frame + Position + Quaternion (一次性读取)
					memcopySize = sizeof(vmdKeyPtr->Frame) + sizeof(vmdKeyPtr->Position) + sizeof(vmdKeyPtr->Quaternion);
					FMemory::Memcpy(&vmdKeyPtr->Frame, Buffer, memcopySize);
					Buffer += memcopySize;

					// 高效讀取64字節插值數據，直接提取關鍵位置的數據
					// 根據MMD Tools的minimum acceptable data格式，只讀取有效位置
					const uint8* interpolationBuffer = Buffer;

					// 使用指針直接訪問，避免複製64字節到臨時數組
					// X軸: 位置 0, 4, 8, 12
					vmdKeyPtr->Bezier[0][0][0] = interpolationBuffer[0];  // x_x1
					vmdKeyPtr->Bezier[0][1][0] = interpolationBuffer[4];  // x_y1
					vmdKeyPtr->Bezier[1][0][0] = interpolationBuffer[8];  // x_x2
					vmdKeyPtr->Bezier[1][1][0] = interpolationBuffer[12]; // x_y2

					// Y軸: 位置 16, 20, 24, 28
					vmdKeyPtr->Bezier[0][0][1] = interpolationBuffer[16]; // y_x1
					vmdKeyPtr->Bezier[0][1][1] = interpolationBuffer[20]; // y_y1
					vmdKeyPtr->Bezier[1][0][1] = interpolationBuffer[24]; // y_x2
					vmdKeyPtr->Bezier[1][1][1] = interpolationBuffer[28]; // y_y2

					// Z軸: 位置 32, 36, 40, 44
					vmdKeyPtr->Bezier[0][0][2] = interpolationBuffer[32]; // z_x1
					vmdKeyPtr->Bezier[0][1][2] = interpolationBuffer[36]; // z_y1
					vmdKeyPtr->Bezier[1][0][2] = interpolationBuffer[40]; // z_x2
					vmdKeyPtr->Bezier[1][1][2] = interpolationBuffer[44]; // z_y2

					// 旋轉軸: 位置 48, 52, 56, 60
					vmdKeyPtr->Bezier[0][0][3] = interpolationBuffer[48]; // r_x1
					vmdKeyPtr->Bezier[0][1][3] = interpolationBuffer[52]; // r_y1
					vmdKeyPtr->Bezier[1][0][3] = interpolationBuffer[56]; // r_x2
					vmdKeyPtr->Bezier[1][1][3] = interpolationBuffer[60]; // r_y2

					Buffer += 64; // 跳過整個64字節區塊
				}
			}
			// 设置每个数据的起始地址
			{
				// Key Fase VMD
				memcopySize = sizeof(readData.vmdFaceCount);
				FMemory::Memcpy(&readData.vmdFaceCount, Buffer, memcopySize);
				Buffer += memcopySize;

				// 优化：一次性分配所有内存
				readData.vmdFaceList.SetNumZeroed(readData.vmdFaceCount);

				// 优化：批量读取数据
				for (int32 i = 0; i < readData.vmdFaceCount; ++i)
				{
					VMD_FACE_KEY* vmdFacePtr = &readData.vmdFaceList[i];

					// Bone NAME
					memcopySize = sizeof(vmdFacePtr->Name);
					FMemory::Memcpy(&vmdFacePtr->Name[0], Buffer, memcopySize);
					Buffer += memcopySize;

					// frame and value (一次性读取)
					memcopySize = sizeof(int32) + sizeof(float);
					FMemory::Memcpy(&vmdFacePtr->Frame, Buffer, memcopySize);
					Buffer += memcopySize;
				}
			}
			// 设置每个数据的起始地址
			{
				// Key Camera VMD
				memcopySize = sizeof(readData.vmdCameraCount);
				FMemory::Memcpy(&readData.vmdCameraCount, Buffer, memcopySize);
				Buffer += memcopySize;

				if (readData.vmdCameraCount > 0)
				{
					// 优化：一次性分配所有内存
					readData.vmdCameraList.SetNumZeroed(readData.vmdCameraCount);

					// 优化：批量读取数据
					for (int32 i = 0; i < readData.vmdCameraCount; ++i)
					{
						VMD_CAMERA* vmdCameraPtr = &readData.vmdCameraList[i];

						// 批量读取 Frame + Length + Location + Rotate
						memcopySize = sizeof(uint32) * (1) + sizeof(float) * (1 + 3 + 3);
						FMemory::Memcpy(&vmdCameraPtr->Frame, Buffer, memcopySize);
						Buffer += memcopySize;

						// 批量读取 Interpolation
						memcopySize = sizeof(uint8) * (6 * 4);
						FMemory::Memcpy(&vmdCameraPtr->Interpolation[0][0][0], Buffer, memcopySize);
						Buffer += memcopySize;

						// 批量读取 ViewingAngle + Perspective
						memcopySize = sizeof(uint32) * (1) + sizeof(uint8) * (1);
						FMemory::Memcpy(&vmdCameraPtr->ViewingAngle, Buffer, memcopySize);
						Buffer += memcopySize;
					}
				}
			}
		}
		//////////////////////////////
		if (!ConvertVMDFromReadData(&readData))
		{
			// convert err
			return false;
		}

		return true;
	}

	bool VmdMotionInfo::ConvertVMDFromReadData(
		VmdReadMotionData* readData)
	{
		check(readData);
		if (!readData)
		{
			return false;
		}
		////////////////////////////
		ModelName = ConvertMMDSJISToFString(
			(uint8*)&(readData->vmdHeader.modelName),
			sizeof(readData->vmdHeader.modelName));
		///////////////////////////////

		int arrayIndx = -1;
		FString trackName;

		// 优化：使用哈希映射跟踪骨骼和表情名称，避免线性搜索
		TMap<FString, int32> boneTrackNameMap;
		TMap<FString, int32> faceTrackNameMap;

		//////////////////////////////////
		{
			// Keys
			TArray<VmdKeyTrackList> tempKeyBoneList;
			VmdKeyTrackList* vmdKeyTrackPtr = NULL;

			// VMD Key
			boneTrackNameMap.Empty(readData->vmdKeyCount / 10); // 预估轨道数量

			for (int32 i = 0; i < readData->vmdKeyCount; i++)
			{
				// get ptr
				VMD_KEY* vmdKeyPtr = &(readData->vmdKeyList[i]);
				//
				trackName = ConvertMMDSJISToFString(
					(uint8*)&(vmdKeyPtr->Name),
					sizeof(vmdKeyPtr->Name));

				// 优化：使用哈希映射查找轨道索引
				int32* foundIndex = boneTrackNameMap.Find(trackName);
				if (foundIndex == nullptr)
				{
					// 新轨道
					arrayIndx = tempKeyBoneList.Add(VmdKeyTrackList());
					vmdKeyTrackPtr = &(tempKeyBoneList[arrayIndx]);
					vmdKeyTrackPtr->TrackName = trackName;
					boneTrackNameMap.Add(trackName, arrayIndx);
				}
				else
				{
					// 已存在的轨道
					arrayIndx = *foundIndex;
					vmdKeyTrackPtr = &(tempKeyBoneList[arrayIndx]);
				}

				check(vmdKeyTrackPtr);
				///
				arrayIndx = vmdKeyTrackPtr->keyList.Add(*vmdKeyPtr);
				//
				vmdKeyTrackPtr->maxFrameCount = FMath::Max(vmdKeyPtr->Frame, vmdKeyTrackPtr->maxFrameCount);
				vmdKeyTrackPtr->minFrameCount = FMath::Min(vmdKeyPtr->Frame, vmdKeyTrackPtr->minFrameCount);
			}

			// 优化排序：使用内建快速排序代替手动排序
			for (int i = 0; i < tempKeyBoneList.Num(); i++)
			{
				// 首先对关键帧按Frame排序
				if (tempKeyBoneList[i].keyList.Num() > 0)
				{
					// 使用标准排序
					tempKeyBoneList[i].keyList.Sort([](const VMD_KEY& A, const VMD_KEY& B) {
						return A.Frame < B.Frame;
					});

					// 优化：直接预分配内存并填充排序索引
					const int32 keyCount = tempKeyBoneList[i].keyList.Num();
					tempKeyBoneList[i].sortIndexList.Empty(keyCount);
					tempKeyBoneList[i].sortIndexList.Reserve(keyCount);
					for (int32 j = 0; j < keyCount; j++)
					{
						tempKeyBoneList[i].sortIndexList.Add(j);
					}
				}
			}
			keyBoneList = tempKeyBoneList;
		}
		{
			// Skins
			TArray<VmdFaceTrackList> tempKeyFaceList;
			VmdFaceTrackList* vmdFaceTrackPtr = NULL;

			// 优化：使用哈希映射跟踪表情名称
			faceTrackNameMap.Empty(readData->vmdFaceCount / 5); // 预估轨道数量

			for (int32 i = 0; i < readData->vmdFaceCount; i++)
			{
				// get ptr
				VMD_FACE_KEY* vmdFacePtr = &(readData->vmdFaceList[i]);
				//
				trackName = ConvertMMDSJISToFString(
					(uint8*)&(vmdFacePtr->Name),
					sizeof(vmdFacePtr->Name));

				// 优化：使用哈希映射查找轨道索引
				int32* foundIndex = faceTrackNameMap.Find(trackName);
				if (foundIndex == nullptr)
				{
					// 新轨道
					arrayIndx = tempKeyFaceList.Add(VmdFaceTrackList());
					vmdFaceTrackPtr = &(tempKeyFaceList[arrayIndx]);
					vmdFaceTrackPtr->TrackName = trackName;
					faceTrackNameMap.Add(trackName, arrayIndx);
				}
				else
				{
					// 已存在的轨道
					arrayIndx = *foundIndex;
					vmdFaceTrackPtr = &(tempKeyFaceList[arrayIndx]);
				}

				check(vmdFaceTrackPtr);
				///
				arrayIndx = vmdFaceTrackPtr->keyList.Add(*vmdFacePtr);
				//
				vmdFaceTrackPtr->maxFrameCount = FMath::Max(vmdFacePtr->Frame, vmdFaceTrackPtr->maxFrameCount);
				vmdFaceTrackPtr->minFrameCount = FMath::Min(vmdFacePtr->Frame, vmdFaceTrackPtr->minFrameCount);
			}

			// 优化排序：使用内建快速排序代替手动排序
			for (int i = 0; i < tempKeyFaceList.Num(); i++)
			{
				// 首先对关键帧按Frame排序
				if (tempKeyFaceList[i].keyList.Num() > 0)
				{
					// 使用标准排序
					tempKeyFaceList[i].keyList.Sort([](const VMD_FACE_KEY& A, const VMD_FACE_KEY& B) {
						return A.Frame < B.Frame;
					});

					// 优化：直接预分配内存并填充排序索引
					const int32 keyCount = tempKeyFaceList[i].keyList.Num();
					tempKeyFaceList[i].sortIndexList.Empty(keyCount);
					tempKeyFaceList[i].sortIndexList.Reserve(keyCount);
					for (int32 j = 0; j < keyCount; j++)
					{
						tempKeyFaceList[i].sortIndexList.Add(j);
					}
				}
			}
			keyFaceList = tempKeyFaceList;
		}
		if (readData->vmdCameraCount > 0)
		{
			// Keys
			TArray<VmdCameraTrackList> tempKeyCamList;
			VmdCameraTrackList* vmdCamKeyTrackPtr = NULL;
			VMD_CAMERA* vmdCamKeyPtr = NULL;

			// Camera Key - 相机轨道只有一个，无需使用映射
			tempKeyCamList.Empty();
			tempKeyCamList.Add(VmdCameraTrackList()); // VMD中相机只有一个
			trackName = "MMDCamera000";				  // 固定值
			// New Track
			vmdCamKeyTrackPtr = &(tempKeyCamList[0]); // VMD中相机只有一个
			vmdCamKeyTrackPtr->TrackName = trackName;

			// 优化：预分配内存
			vmdCamKeyTrackPtr->keyList.Reserve(readData->vmdCameraCount);

			for (int32 i = 0; i < readData->vmdCameraCount; i++)
			{
				// get ptr
				vmdCamKeyPtr = &(readData->vmdCameraList[i]);
				//
				check(vmdCamKeyTrackPtr);
				///
				arrayIndx = vmdCamKeyTrackPtr->keyList.Add(*vmdCamKeyPtr);
				//
				vmdCamKeyTrackPtr->maxFrameCount = FMath::Max(vmdCamKeyPtr->Frame, vmdCamKeyTrackPtr->maxFrameCount);
				vmdCamKeyTrackPtr->minFrameCount = FMath::Min(vmdCamKeyPtr->Frame, vmdCamKeyTrackPtr->minFrameCount);
			}

			// 优化排序：使用内建快速排序代替手动排序
			for (int i = 0; i < tempKeyCamList.Num(); i++)
			{
				// 首先对关键帧按Frame排序
				if (tempKeyCamList[i].keyList.Num() > 0)
				{
					// 使用标准排序
					tempKeyCamList[i].keyList.Sort([](const VMD_CAMERA& A, const VMD_CAMERA& B) {
						return A.Frame < B.Frame;
					});

					// 优化：直接预分配内存并填充排序索引
					const int32 keyCount = tempKeyCamList[i].keyList.Num();
					tempKeyCamList[i].sortIndexList.Empty(keyCount);
					tempKeyCamList[i].sortIndexList.Reserve(keyCount);
					for (int32 j = 0; j < keyCount; j++)
					{
						tempKeyCamList[i].sortIndexList.Add(j);
					}
				}
			}
			keyCameraList = tempKeyCamList;
		}

		// 优化：一次性计算全局最小/最大帧
		UpdateMinMaxFrames();

		// 构建名称映射以加速查找
		BuildNameMaps();

		return true;
	}

	// 优化：一次性计算全局最小/最大帧
	void VmdMotionInfo::UpdateMinMaxFrames()
	{
		maxFrame = 0;
		minFrame = TNumericLimits<uint32>::Max();

		// 检查骨骼帧
		for (const auto& track : keyBoneList)
		{
			maxFrame = FMath::Max(maxFrame, track.maxFrameCount);
			if (track.minFrameCount < TNumericLimits<uint32>::Max())
			{
				minFrame = FMath::Min(minFrame, track.minFrameCount);
			}
		}

		// 检查表情帧
		for (const auto& track : keyFaceList)
		{
			maxFrame = FMath::Max(maxFrame, track.maxFrameCount);
			if (track.minFrameCount < TNumericLimits<uint32>::Max())
			{
				minFrame = FMath::Min(minFrame, track.minFrameCount);
			}
		}

		// 检查相机帧
		for (const auto& track : keyCameraList)
		{
			maxFrame = FMath::Max(maxFrame, track.maxFrameCount);
			if (track.minFrameCount < TNumericLimits<uint32>::Max())
			{
				minFrame = FMath::Min(minFrame, track.minFrameCount);
			}
		}
	}

	// 优化：构建名称映射以加速查找
	void VmdMotionInfo::BuildNameMaps()
	{
		// 构建骨骼名称映射
		BoneNameToIndexMap.Empty(keyBoneList.Num());
		for (int i = 0; i < keyBoneList.Num(); i++)
		{
			BoneNameToIndexMap.Add(keyBoneList[i].TrackName, i);
		}

		// 构建表情名称映射
		FaceNameToIndexMap.Empty(keyFaceList.Num());
		for (int i = 0; i < keyFaceList.Num(); i++)
		{
			FaceNameToIndexMap.Add(keyFaceList[i].TrackName, i);
		}
	}

	// 优化：使用哈希映射加速名称查找
	int32 VmdMotionInfo::FindKeyTrackName(
		FString targetName,
		EVMDKEYFRAMETYPE listType)
	{
		if (listType == EVMDKEYFRAMETYPE::EVMD_KEYBONE)
		{
			// 1. 直接查找
			int32* FoundIndex = BoneNameToIndexMap.Find(targetName);
			if (FoundIndex)
			{
				return *FoundIndex;
			}

			// 2. 尝试替换 "_" 为 "."
			if (targetName.Contains(TEXT("_")))
			{
				FString modifiedName = targetName;
				modifiedName.ReplaceInline(TEXT("_"), TEXT("."));

				FoundIndex = BoneNameToIndexMap.Find(modifiedName);
				if (FoundIndex)
				{
					return *FoundIndex;
				}

				// 3. 尝试替换 "_" 为 "+"
				modifiedName = targetName;
				modifiedName.ReplaceInline(TEXT("_"), TEXT("+"));

				FoundIndex = BoneNameToIndexMap.Find(modifiedName);
				if (FoundIndex)
				{
					return *FoundIndex;
				}
			}
		}
		else if (listType == EVMDKEYFRAMETYPE::EVMD_KEYFACE)
		{
			// 直接查找
			int32* FoundIndex = FaceNameToIndexMap.Find(targetName);
			if (FoundIndex)
			{
				return *FoundIndex;
			}
		}

		return -1;
	}
} // namespace MMD4UE5