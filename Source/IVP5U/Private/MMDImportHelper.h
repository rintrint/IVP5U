// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "Engine.h"

namespace MMD4UE5
{
	// ToDo:MMD（PMX、PMD、VMD）的共同函数重构
	// 现状，PMX系、PMD/VMD系分别安装。

	// encode type
	enum PMXEncodeType
	{
		PMXEncodeType_UTF16LE = 0, // for PMX
		PMXEncodeType_UTF8,		   // for PMX
		PMXEncodeType_SJIS,		   // for PMD, VMD
		PMXEncodeType_ERROR
	};

	DECLARE_LOG_CATEGORY_EXTERN(LogMMD4UE5_MMDImportHelper, Log, All);

	class MMDImportHelper
	{
	public:
		// from MMD Axis To UE5 Axis
		// param  : vec , in vector
		// return : convert vec ,out
		static FVector3f ConvertVectorAxisToUE5FromMMD(const FVector3f& vec);

		// from PMX Binary Buffer To String @ TextBuf
		// 4 + n: TexBuf
		// buf : top string (top data)
		// encodeType : 0 utf-16, 1 utf-8
		static FString PMXTexBufferToFString(const uint8** buffer, const PMXEncodeType encodeType);

		// from MMD char (SJIS) To FString
		// 4 + n: TexBuf
		// buf : top string (top data)
		// encodeType : SJIS
		// for Pmd , Vmd format
		static FString ConvertMMDSJISToFString(const uint8* buffer, const uint32 size);

		// import uint
		static uint32 MMDExtendBufferSizeToUint32(const uint8** buffer, const uint8 blockSize);

		// import int
		static int32 MMDExtendBufferSizeToInt32(const uint8** buffer, const uint8 blockSize);
	};
} // namespace MMD4UE5
