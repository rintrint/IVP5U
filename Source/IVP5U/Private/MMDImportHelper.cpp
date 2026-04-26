// Copyright 2015-2026 IVP5U contributors

#include "MMDImportHelper.h"
#include "SjisToUnicode.h"

namespace MMD4UE5
{
	FVector3f MMDImportHelper::ConvertVectorAxisToUE5FromMMD(const FVector3f& vec)
	{
		return FVector3f(vec.X, -vec.Z, vec.Y);
	}

	//////////////////////////////////////
	// from PMD/PMX Binary Buffer To String @ TextBuf
	// 4 + n: TexBuf
	// buf : top string (top data)
	// encodeType : 0 utf-16, 1 utf-8
	//////////////////////////////////////
	FString MMDImportHelper::PMXTexBufferToFString(const uint8** buffer, const PMXEncodeType encodeType)
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

	FString MMDImportHelper::ConvertMMDSJISToFString(const uint8* buffer, const uint32 size)
	{
		FString NewString;
		TArray<char> RawModData;
		{
			RawModData.Empty(size);
			RawModData.AddUninitialized(size);
			FMemory::Memcpy(RawModData.GetData(), buffer, RawModData.Num());
			RawModData.Add(0);
			RawModData.Add(0);
			NewString.Append((wchar_t*)saba::ConvertSjisToU16String(RawModData.GetData()).c_str());
		}
		return NewString;
	}

	uint32 MMDImportHelper::MMDExtendBufferSizeToUint32(const uint8** buffer, const uint8 blockSize)
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
	int32 MMDImportHelper::MMDExtendBufferSizeToInt32(const uint8** buffer, const uint8 blockSize)
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
