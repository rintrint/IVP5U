// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "MMDImportHelper.h"
#include "IVP5UPrivatePCH.h"
// #include "EncodeHelper.h"
#include "SjisToUnicode.h"
namespace MMD4UE5
{
	FVector3f MMDImportHelper::ConvertVectorAsixToUE5FromMMD(
		FVector3f vec)
	{
		FVector3f temp;
		temp.Y = vec.Z * (-1);
		temp.X = vec.X * (1);
		temp.Z = vec.Y * (1);
		return temp;
	}

	//////////////////////////////////////
	// from PMD/PMX Binary Buffer To String @ TextBuf
	// 4 + n: TexBuf
	// buf : top string (top data)
	// encodeType : 0 utf-16, 1 utf-8
	//////////////////////////////////////
	FString MMDImportHelper::PMXTexBufferToFString(const uint8** buffer, PMXEncodeType encodeType)
	{
		FString NewString;
		uint32 size = 0;
		// temp data
		TArray<uint8> RawModData;

		if (encodeType == PMXEncodeType_UTF16LE)
		{
			FMemory::Memcpy(&size, *buffer, sizeof(uint32));
			*buffer += sizeof(uint32);
			RawModData.Empty(size);
			RawModData.AddUninitialized(size);
			FMemory::Memcpy(RawModData.GetData(), *buffer, RawModData.Num());
			RawModData.Add(0);
			RawModData.Add(0);
			NewString.Append((TCHAR*)RawModData.GetData());
			*buffer += size;
		}
		else
		{
			// this plugin  unsuported encodetype ( utf-8 etc.)
			// Error ...
			// UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Encode Type : not UTF-16 LE , unload"));
		}
		return NewString;
	}

	FString MMDImportHelper::ConvertMMDSJISToFString(
		uint8* buffer,
		const uint32 size)
	{
		FString NewString;
		// uint32 size = 0;
		// temp data
		TArray<char> RawModData;

		{
			// FMemory::Memcpy(&size, *buffer, sizeof(uint32));
			//*buffer += sizeof(uint32);
			// size = 20;
			RawModData.Empty(size);
			RawModData.AddUninitialized(size);
			FMemory::Memcpy(RawModData.GetData(), buffer, RawModData.Num());
			RawModData.Add(0);
			RawModData.Add(0);
			NewString.Append((wchar_t*)saba::ConvertSjisToU16String(RawModData.GetData()).c_str());
			// NewString.Append(UTF8_TO_TCHAR(encodeHelper.convert_encoding(RawModData.GetData(), "shift-jis", "utf-8").c_str()));
			// NewString.Append(RawModData.GetData());
		}
		return NewString;
	}

	/////////////////////////////////////
	//
	//////////////////////////////////////
	uint32 MMDImportHelper::MMDExtendBufferSizeToUint32(
		const uint8** buffer,
		const uint8 blockSize)
	{
		uint32 retValue = 0;

		switch (blockSize)
		{
			case 1:
				retValue = (uint8)((*buffer)[0]);
				*buffer += blockSize;
				break;

			case 2:
				retValue = (uint16)(((uint16*)*buffer)[0]);
				*buffer += blockSize;
				break;

			case 4:
				retValue = (uint32)((uint32*)*buffer)[0];
				*buffer += blockSize;
				break;
		}

		return retValue;
	}
	int32 MMDImportHelper::MMDExtendBufferSizeToInt32(
		const uint8** buffer,
		const uint8 blockSize)
	{
		int32 retValue = 0;

		switch (blockSize)
		{
			case 1:
				retValue = (int8)((*buffer)[0]);
				*buffer += blockSize;
				break;

			case 2:
				retValue = (int16)(((int16*)*buffer)[0]);
				*buffer += blockSize;
				break;

			case 4:
				retValue = (int32)((int32*)*buffer)[0];
				*buffer += blockSize;
				break;
		}

		return retValue;
	}
} // namespace MMD4UE5