// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"

namespace MMD4UE5
{
    // ToDo:MMD（PMX、PMD、VMD）的共同函数重构
    // 现状，PMX系、PMD/VMD系分别安装。

    ///////////////
    // encode type
    ///////////////
    enum PMXEncodeType
    {
        PMXEncodeType_UTF16LE = 0, // for PMX
        PMXEncodeType_UTF8,        // for PMX
        PMXEncodeType_SJIS,        // for PMD, VMD
        PMXEncodeType_ERROR
    };
    class MMDImportHelper
    {
    public:
        //////////////////////////////////////
        // from MMD Asix To UE5 Asix
        // param  : vec , in vector
        // return : convert vec ,out
        //////////////////////////////////////
        FVector3f ConvertVectorAsixToUE5FromMMD(
            FVector3f vec);
        //////////////////////////////////////
        // from PMX Binary Buffer To String @ TextBuf
        // 4 + n: TexBuf
        // buf : top string (top data)
        // encodeType : 0 utf-16, 1 utf-8
        //////////////////////////////////////
        FString PMXTexBufferToFString(
            const uint8 **buffer,
            PMXEncodeType encodeType);
        //////////////////////////////////////
        // from MMD char (SJIS) To FString
        // 4 + n: TexBuf
        // buf : top string (top data)
        // encodeType : SJIS
        // for Pmd , Vmd format
        //////////////////////////////////////
        FString ConvertMMDSJISToFString(
            uint8 *buffer,
            const uint32 size);

        /////////////////////////////////////
        // import uint
        //////////////////////////////////////
        uint32 MMDExtendBufferSizeToUint32(
            const uint8 **buffer,
            const uint8 blockSize);
        /////////////////////////////////////
        // import int
        //////////////////////////////////////
        int32 MMDExtendBufferSizeToInt32(
            const uint8 **buffer,
            const uint8 blockSize);
    };
}