// Copyright 2015-2026 IVP5U contributors

#include "MMDImportHelper.h"
#include "SjisToUnicode.h"

namespace MMD4UE5
{
	DEFINE_LOG_CATEGORY(LogMMD4UE5_MMDImportHelper);

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
		if (encodeType == PMXEncodeType_UTF16LE)
		{
			FMemory::Memcpy(&size, *buffer, sizeof(uint32));
			*buffer += sizeof(uint32);
			if (size > 0)
			{
				const int32 NumChars = static_cast<int32>(size / sizeof(UTF16CHAR));
				// Copy to an aligned TArray; raw buffer may not be 2-byte aligned (unaligned read fault on ARM)
				TArray<UTF16CHAR> Utf16Buffer;
				Utf16Buffer.AddUninitialized(NumChars);
				FMemory::Memcpy(Utf16Buffer.GetData(), *buffer, size);
				// On Windows TCHAR == UTF16CHAR (no-op); on platforms with 32-bit TCHAR this performs real conversion
				const auto Converted = StringCast<TCHAR>(Utf16Buffer.GetData(), NumChars);
				NewString = FString(Converted.Length(), Converted.Get());
			}
			*buffer += size;
		}
		else if (encodeType == PMXEncodeType_UTF8)
		{
			FMemory::Memcpy(&size, *buffer, sizeof(uint32));
			*buffer += sizeof(uint32);
			if (size > 0)
			{
				const FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(*buffer), size);
				NewString = FString(Conv.Length(), Conv.Get());
			}
			*buffer += size;
		}
		else
		{
			UE_LOG(LogMMD4UE5_MMDImportHelper, Warning, TEXT("PMXTexBufferToFString: unsupported encoding type %d, skipping field"), static_cast<int32>(encodeType));
			// Still consume the field (4-byte size + payload) to keep buffer in sync; otherwise all subsequent parsing desyncs
			FMemory::Memcpy(&size, *buffer, sizeof(uint32));
			*buffer += sizeof(uint32);
			*buffer += size;
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
