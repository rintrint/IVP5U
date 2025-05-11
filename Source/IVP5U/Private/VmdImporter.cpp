// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#include "VmdImporter.h"
#include "IVP5UPrivatePCH.h"
#include "MMDImportHelper.h"

#include "Animation/AnimSequenceBase.h"

namespace MMD4UE4
{

	DEFINE_LOG_CATEGORY(LogMMD4UE4_VmdMotionInfo)

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
				UE_LOG(LogMMD4UE4_VmdMotionInfo, Log, TEXT("VMD Import START /Correct Magic[Vocaloid Motion Data 0002]"));
			}
			else
			{
				// UE_LOG(LogMMD4UE4_PmdMeshInfo, Error, TEXT("PMX Import FALSE/Return /UnCorrect Magic[PMX]"));
				return false;
			}
			Buffer += memcopySize;

			// 设置每个数据的起始地址
			{
				// Key VMD
				memcopySize = sizeof(readData.vmdKeyCount);
				FMemory::Memcpy(&readData.vmdKeyCount, Buffer, memcopySize);
				Buffer += memcopySize;

				// memcopySize = sizeof(VMD_KEY);//111
				readData.vmdKeyList.AddZeroed(readData.vmdKeyCount);

				for (int32 i = 0; i < readData.vmdKeyCount; ++i)
				{
					VMD_KEY* vmdKeyPtr = &readData.vmdKeyList[i];

					// Bone NAME
					memcopySize = sizeof(vmdKeyPtr->Name);
					FMemory::Memcpy(&vmdKeyPtr->Name[0], Buffer, memcopySize);
					Buffer += memcopySize;
					// posandQurt
					memcopySize = sizeof(int32) + sizeof(float) * (4 + 3);
					FMemory::Memcpy(&vmdKeyPtr->Frame, Buffer, memcopySize);
					Buffer += memcopySize;
					// bezier
					memcopySize = sizeof(vmdKeyPtr->Bezier);
					FMemory::Memcpy(&vmdKeyPtr->Bezier, Buffer, memcopySize);
					Buffer += memcopySize;
					//
					// dummy bezier
					Buffer += 48 * sizeof(uint8);
				}
			}
			// 设置每个数据的起始地址
			{
				// Key Fase VMD
				memcopySize = sizeof(readData.vmdFaceCount);
				FMemory::Memcpy(&readData.vmdFaceCount, Buffer, memcopySize);
				Buffer += memcopySize;

				// memcopySize = sizeof(VMD_FACE_KEY);//111
				readData.vmdFaceList.AddZeroed(readData.vmdFaceCount);
				for (int32 i = 0; i < readData.vmdFaceCount; ++i)
				{
					VMD_FACE_KEY* vmdFacePtr = &readData.vmdFaceList[i];

					// Bone NAME
					memcopySize = sizeof(vmdFacePtr->Name);
					FMemory::Memcpy(&vmdFacePtr->Name[0], Buffer, memcopySize);
					Buffer += memcopySize;
					// frame and value
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
					// memcopySize = sizeof(VMD_CAMERA);//61
					readData.vmdCameraList.AddZeroed(readData.vmdCameraCount);
					for (int32 i = 0; i < readData.vmdCameraCount; ++i)
					{
						VMD_CAMERA* vmdCameraPtr = &readData.vmdCameraList[i];

						// Freme No
						memcopySize = sizeof(uint32) * (1) // FrameNo
							+ sizeof(float) * (1 + 3 + 3); // Length + Location + Rotate
						FMemory::Memcpy(&vmdCameraPtr->Frame, Buffer, memcopySize);
						Buffer += memcopySize;

						// Interpolation[6][4]
						memcopySize = sizeof(uint8) * (6 * 4);
						FMemory::Memcpy(&vmdCameraPtr->Interpolation[0][0][0], Buffer, memcopySize);
						Buffer += memcopySize;

						// ViewingAngle + Perspective
						memcopySize = sizeof(uint32) * (1) // ViewingAngle
							+ sizeof(uint8) * (1);		   // Perspective
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
		int arrayIndxPre;
		FString trackName;

		TArray<FString> tempTrackNameList;
		//////////////////////////////////
		{
			// Keys
			TArray<VmdKeyTrackList> tempKeyBoneList;
			VmdKeyTrackList* vmdKeyTrackPtr = NULL;
			// VMD_KEY * vmdKeyPtr = NULL;

			// VMD Key
			tempTrackNameList.Empty();
			arrayIndx = -1;
			for (int32 i = 0; i < readData->vmdKeyCount; i++)
			{
				// get ptr
				VMD_KEY* vmdKeyPtr = &(readData->vmdKeyList[i]);
				//
				trackName = ConvertMMDSJISToFString(
					(uint8*)&(vmdKeyPtr->Name),
					sizeof(vmdKeyPtr->Name));

				arrayIndxPre = tempTrackNameList.Num();
				arrayIndx = tempTrackNameList.AddUnique(trackName);
				if (tempTrackNameList.Num() > arrayIndxPre)
				{
					// New Track
					arrayIndx = tempKeyBoneList.Add(VmdKeyTrackList());
					vmdKeyTrackPtr = &(tempKeyBoneList[arrayIndx]);
					vmdKeyTrackPtr->TrackName = trackName;
				}
				else
				{
					vmdKeyTrackPtr = &(tempKeyBoneList[arrayIndx]);
					/*
					//exist
					for (int k = 0; k < tempKeyBoneList.Num(); k++)
					{
					vmdKeyTrackPtr = &(tempKeyBoneList[k]);
					if (vmdKeyTrackPtr->TrackName.Equals(trackName))
					{
					break;
					}
					vmdKeyTrackPtr = NULL;
					}*/
				}
				check(vmdKeyTrackPtr);
				///
				arrayIndx = vmdKeyTrackPtr->keyList.Add(*vmdKeyPtr);
				//
				vmdKeyTrackPtr->maxFrameCount = FMath::Max(vmdKeyPtr->Frame, vmdKeyTrackPtr->maxFrameCount);
				vmdKeyTrackPtr->minFrameCount = FMath::Min(vmdKeyPtr->Frame, vmdKeyTrackPtr->minFrameCount);
				///
				maxFrame = FMath::Max(vmdKeyPtr->Frame, maxFrame);
				minFrame = FMath::Min(vmdKeyPtr->Frame, minFrame);
			}

			// 優化排序：使用內建快速排序代替手動排序
			for (int i = 0; i < tempKeyBoneList.Num(); i++)
			{
				// 首先對關鍵幀按Frame排序
				if (tempKeyBoneList[i].keyList.Num() > 0)
				{
					// 使用標準排序
					tempKeyBoneList[i].keyList.Sort([](const VMD_KEY& A, const VMD_KEY& B) {
						return A.Frame < B.Frame;
					});

					// 生成排序索引列表 - 這裡索引就直接是數組索引了
					tempKeyBoneList[i].sortIndexList.SetNum(tempKeyBoneList[i].keyList.Num());
					for (int j = 0; j < tempKeyBoneList[i].keyList.Num(); j++)
					{
						tempKeyBoneList[i].sortIndexList[j] = j;
					}
				}
			}
			keyBoneList = tempKeyBoneList;
		}
		{
			// Skins
			TArray<VmdFaceTrackList> tempKeyFaceList;
			VmdFaceTrackList* vmdFaceTrackPtr = NULL;
			// VMD_FACE_KEY * vmdFacePtr = NULL;
			// 坡口
			// Facc
			tempTrackNameList.Empty();
			arrayIndx = -1;
			for (int32 i = 0; i < readData->vmdFaceCount; i++)
			{
				// get ptr
				VMD_FACE_KEY* vmdFacePtr = &(readData->vmdFaceList[i]);
				//
				trackName = ConvertMMDSJISToFString(
					(uint8*)&(vmdFacePtr->Name),
					sizeof(vmdFacePtr->Name));
				arrayIndxPre = tempTrackNameList.Num();
				arrayIndx = tempTrackNameList.AddUnique(trackName);
				if (tempTrackNameList.Num() > arrayIndxPre)
				{
					// New Track
					arrayIndx = tempKeyFaceList.Add(VmdFaceTrackList());
					vmdFaceTrackPtr = &(tempKeyFaceList[arrayIndx]);
					vmdFaceTrackPtr->TrackName = trackName;
				}
				else
				{
					vmdFaceTrackPtr = &(tempKeyFaceList[arrayIndx]);
					/*
					//exist
					for (int k = 0; k < tempKeyBoneList.Num(); k++)
					{
					vmdKeyTrackPtr = &(tempKeyBoneList[k]);
					if (vmdKeyTrackPtr->TrackName.Equals(trackName))
					{
					break;
					}
					vmdKeyTrackPtr = NULL;
					}*/
				}
				check(vmdFaceTrackPtr);
				///
				arrayIndx = vmdFaceTrackPtr->keyList.Add(*vmdFacePtr);
				//
				vmdFaceTrackPtr->maxFrameCount = FMath::Max(vmdFacePtr->Frame, vmdFaceTrackPtr->maxFrameCount);
				vmdFaceTrackPtr->minFrameCount = FMath::Min(vmdFacePtr->Frame, vmdFaceTrackPtr->minFrameCount);
				///
				maxFrame = FMath::Max(vmdFacePtr->Frame, maxFrame);
				minFrame = FMath::Min(vmdFacePtr->Frame, minFrame);
			}

			// 優化排序：使用內建快速排序代替手動排序
			for (int i = 0; i < tempKeyFaceList.Num(); i++)
			{
				// 首先對關鍵幀按Frame排序
				if (tempKeyFaceList[i].keyList.Num() > 0)
				{
					// 使用標準排序
					tempKeyFaceList[i].keyList.Sort([](const VMD_FACE_KEY& A, const VMD_FACE_KEY& B) {
						return A.Frame < B.Frame;
					});

					// 生成排序索引列表 - 這裡索引就直接是數組索引了
					tempKeyFaceList[i].sortIndexList.SetNum(tempKeyFaceList[i].keyList.Num());
					for (int j = 0; j < tempKeyFaceList[i].keyList.Num(); j++)
					{
						tempKeyFaceList[i].sortIndexList[j] = j;
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

			// Camera Key
			tempTrackNameList.Empty();
			arrayIndx = -1;
			tempKeyCamList.Empty();
			tempKeyCamList.Add(VmdCameraTrackList()); // VMDにcamはひとつしかない為
			trackName = "MMDCamera000";				  // 固定値
			// New Track
			vmdCamKeyTrackPtr = &(tempKeyCamList[0]); // VMDにcamはひとつしかない為
			vmdCamKeyTrackPtr->TrackName = trackName;
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
				///
				maxFrame = FMath::Max(vmdCamKeyPtr->Frame, maxFrame);
				minFrame = FMath::Min(vmdCamKeyPtr->Frame, minFrame);
			}

			// 優化排序：使用內建快速排序代替手動排序
			for (int i = 0; i < tempKeyCamList.Num(); i++)
			{
				// 首先對關鍵幀按Frame排序
				if (tempKeyCamList[i].keyList.Num() > 0)
				{
					// 使用標準排序
					tempKeyCamList[i].keyList.Sort([](const VMD_CAMERA& A, const VMD_CAMERA& B) {
						return A.Frame < B.Frame;
					});

					// 生成排序索引列表 - 這裡索引就直接是數組索引了
					tempKeyCamList[i].sortIndexList.SetNum(tempKeyCamList[i].keyList.Num());
					for (int j = 0; j < tempKeyCamList[i].keyList.Num(); j++)
					{
						tempKeyCamList[i].sortIndexList[j] = j;
					}
				}
			}
			keyCameraList = tempKeyCamList;
		}
		//////////////////////////////
		// end phase
		maxFrame++; // min 1frame ?
		return true;
	}

	// 如果在指定列表中有相应的Frame名称，则返回该索引值。异常值=-1。
	int32 VmdMotionInfo::FindKeyTrackName(
		FString targetName,
		EVMDKEYFRAMETYPE listType)
	{
		int32 index = -1;
		// ★TBD:VMD的骨骼名称为15Byte限制，应按前方一致检索
		// 由于麻烦，以完全一致型检索骨骼名称尾（帧名）
		// 導入UE5後骨骼名稱有!"#$%&'()+,./:;=@[]^_`{|}~的都會被轉成底線
		// 空格會被轉成-
		// 只有-*<>?\會維持原樣
		if (listType == EVMDKEYFRAMETYPE::EVMD_KEYBONE)
		{
			// 1. 第一次尝试：完全匹配
			for (int i = 0; i < keyBoneList.Num(); i++)
			{
				if (keyBoneList[i].TrackName.Equals(targetName))
				{
					index = i;
					break;
				}
			}

			// 2. 第二次尝试：将"_"替换为"."后再匹配
			if (index == -1 && targetName.Contains(TEXT("_")))
			{
				FString modifiedName = targetName;
				modifiedName.ReplaceInline(TEXT("_"), TEXT("."));

				for (int i = 0; i < keyBoneList.Num(); i++)
				{
					if (keyBoneList[i].TrackName.Equals(modifiedName))
					{
						index = i;
						break;
					}
				}
			}

			// 3. 第三次尝试：将"_"替换为"+"后再匹配
			if (index == -1 && targetName.Contains(TEXT("_")))
			{
				FString modifiedName = targetName;
				modifiedName.ReplaceInline(TEXT("_"), TEXT("+"));

				for (int i = 0; i < keyBoneList.Num(); i++)
				{
					if (keyBoneList[i].TrackName.Equals(modifiedName))
					{
						index = i;
						break;
					}
				}
			}
		}
		else if (listType == EVMDKEYFRAMETYPE::EVMD_KEYFACE)
		{
			// 1. 第一次尝试：完全匹配
			for (int i = 0; i < keyFaceList.Num(); i++)
			{
				if (keyFaceList[i].TrackName.Equals(targetName))
				{
					index = i;
					break;
				}
			}
		}
		else
		{
			// 操作错误
		}

		return index;
	}
} // namespace MMD4UE4