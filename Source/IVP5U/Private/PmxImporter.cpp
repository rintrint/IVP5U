// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "PmxImporter.h"
#include "IVP5UPrivatePCH.h"

#include "MMDImportHelper.h"
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace MMD4UE5
{

#define LOCTEXT_NAMESPACE "PMXLoader"

    DEFINE_LOG_CATEGORY(LogMMD4UE5_PmxMeshInfo)

    PmxMeshInfo::PmxMeshInfo()
    {
    }

    PmxMeshInfo::~PmxMeshInfo()
    {
    }

    bool PmxMeshInfo::PMXLoaderBinary(
        const uint8 *&Buf,
        const uint8 *BufferEnd)
    {
        Buffer = Buf;
        ////////////////////////////////////////////

        PMXEncodeType pmxEncodeType = PMXEncodeType_ERROR;
        uint32 memcopySize = 0;
        float modelScale = 8.0f;

        FMemory::Memcpy(this->magic, Buffer, sizeof(this->magic));
        if (this->magic[0] == 'P' && this->magic[1] == 'M' && this->magic[2] == 'X')
        {
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import START /Correct Magic[PMX]"));
        }
        else
        {
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Import FALSE/Return /UnCorrect Magic[PMX]"));
            return false;
        }
        Buffer += sizeof(this->magic);

        FMemory::Memcpy(&this->formatVer, Buffer, sizeof(this->formatVer));
        Buffer += sizeof(this->formatVer);

        Buffer += sizeof(uint8);
        /*
        字节列-byte
        [0]-编码方法=0:UTF16:UTF8
        [1]-附加UV数=0-4详细信息请参见顶点
        [2]-顶点索引大小=1,2,4之一
        [3]-纹理索引大小=1,2,4之一
        [4]-材质索引尺寸=1,2,4中的任一种
        [5]-骨骼索引大小=1,2,4之一
        [6]-变形索引大小=1,2,4之一
        [7]-刚体索引大小=1,2,4之一
        */
        FMemory::Memcpy(&this->baseHeader, Buffer, sizeof(this->baseHeader));
        // NewMyAsset->MyStruct.ModelName.Append((char*)BufferPMXHeaderPtr__Impl.byteDate);
        Buffer += sizeof(this->baseHeader);

        if (0 == this->baseHeader.EncodeType)
        {
            pmxEncodeType = PMXEncodeType_UTF16LE;
        }
        else if (1 == this->baseHeader.EncodeType)
        {
            pmxEncodeType = PMXEncodeType_UTF8;
        }
        else
        {
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Import FALSE/Return /UnCorrect EncodeType"));
            return false;
        }
        this->modelNameJP = PMXTexBufferToFString(&Buffer, pmxEncodeType);
        this->modelNameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);
        this->modelCommentJP = PMXTexBufferToFString(&Buffer, pmxEncodeType);
        this->modelCommentEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);

        modelNameJP = modelNameJP.Replace(TEXT("."), TEXT("_")); // [.] is broken filepath for UE5
        ////////////////////////////////////////////
        {
            // 統計
            uint32 statics_bdef1 = 0;
            uint32 statics_bdef2 = 0;
            uint32 statics_bdef4 = 0;
            uint32 statics_sdef = 0;

            uint32 PmxVertexNum = 0;
            memcopySize = sizeof(PmxVertexNum);
            FMemory::Memcpy(&PmxVertexNum, Buffer, memcopySize);
            Buffer += memcopySize;

            vertexList.AddZeroed(PmxVertexNum);
            for (uint32 VertexIndex = 0; VertexIndex < PmxVertexNum; ++VertexIndex)
            {
                PMX_VERTEX &pmxVertexPtr = vertexList[VertexIndex];
                ///
                // 位置(x,y,z)
                memcopySize = sizeof(pmxVertexPtr.Position);
                FMemory::Memcpy(&pmxVertexPtr.Position, Buffer, memcopySize);
                pmxVertexPtr.Position = ConvertVectorAsixToUE5FromMMD(pmxVertexPtr.Position) * modelScale;
                Buffer += memcopySize;
                // 法線(x,y,z)
                memcopySize = sizeof(pmxVertexPtr.Normal);
                FMemory::Memcpy(&pmxVertexPtr.Normal, Buffer, memcopySize);
                pmxVertexPtr.Normal = ConvertVectorAsixToUE5FromMMD(pmxVertexPtr.Normal);
                Buffer += memcopySize;
                // UV(u,v)
                memcopySize = sizeof(pmxVertexPtr.UV);
                FMemory::Memcpy(&pmxVertexPtr.UV, Buffer, memcopySize);
                Buffer += memcopySize;
                /*
                float tempUV = pmxVertexPtr.UV.X;//UE5座標系反転
                pmxVertexPtr.UV.X = 1 - pmxVertexPtr.UV.Y;
                pmxVertexPtr.UV.Y = 1 - tempUV;
                */
                // 追加UV(x,y,z,w)  PMXヘッダの追加UV数による	n:追加UV数 0～4
                for (int ExUVNum = 0; ExUVNum < this->baseHeader.UVNum; ++ExUVNum)
                {
                    //
                    memcopySize = sizeof(pmxVertexPtr.AddUV[ExUVNum]);
                    FMemory::Memcpy(&pmxVertexPtr.AddUV[ExUVNum], Buffer, memcopySize);
                    Buffer += memcopySize;
                }
                // 配重变形方式 0:BDEF1 1:BDEF2 2:BDEF4 3:SDEF
                memcopySize = sizeof(pmxVertexPtr.WeightType);
                FMemory::Memcpy(&pmxVertexPtr.WeightType, Buffer, memcopySize);
                Buffer += memcopySize;
                //
                if (pmxVertexPtr.WeightType == 0)
                {
                    statics_bdef1++;
                    // BDEF1
                    pmxVertexPtr.BoneIndex[0] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[0]++;
                    pmxVertexPtr.BoneWeight[0] = 1.0f;
                }
                else if (pmxVertexPtr.WeightType == 1)
                {
                    statics_bdef2++;
                    // BDEF2
                    pmxVertexPtr.BoneIndex[0] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[0]++;
                    pmxVertexPtr.BoneIndex[1] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[1]++;
                    //
                    memcopySize = sizeof(pmxVertexPtr.BoneWeight[0]);
                    FMemory::Memcpy(&pmxVertexPtr.BoneWeight[0], Buffer, memcopySize);
                    Buffer += memcopySize;
                    //
                    pmxVertexPtr.BoneWeight[1] = 1.0f - pmxVertexPtr.BoneWeight[0];
                }
                else if (pmxVertexPtr.WeightType == 2)
                {
                    statics_bdef4++;
                    // BDEF4
                    pmxVertexPtr.BoneIndex[0] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[0]++;
                    pmxVertexPtr.BoneIndex[1] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[1]++;
                    pmxVertexPtr.BoneIndex[2] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[2]++;
                    pmxVertexPtr.BoneIndex[3] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[3]++;
                    for (int bw = 0; bw < 4; ++bw)
                    {
                        //
                        memcopySize = sizeof(pmxVertexPtr.BoneWeight[bw]);
                        FMemory::Memcpy(&pmxVertexPtr.BoneWeight[bw], Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                }
                else if (pmxVertexPtr.WeightType == 3)
                {
                    statics_sdef++;
                    // SDEF
                    pmxVertexPtr.BoneIndex[0] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[0]++;
                    pmxVertexPtr.BoneIndex[1] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    pmxVertexPtr.BoneIndex[1]++;
                    //
                    memcopySize = sizeof(pmxVertexPtr.BoneWeight[0]);
                    FMemory::Memcpy(&pmxVertexPtr.BoneWeight[0], Buffer, memcopySize);
                    Buffer += memcopySize;
                    pmxVertexPtr.BoneWeight[1] = 1.0f - pmxVertexPtr.BoneWeight[0];

                    for (int bw = 0; bw < 1; ++bw)
                    {
                        //
                        memcopySize = sizeof(pmxVertexPtr.SDEF_C);
                        FMemory::Memcpy(&pmxVertexPtr.SDEF_C, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    for (int bw = 0; bw < 1; ++bw)
                    {
                        //
                        memcopySize = sizeof(pmxVertexPtr.SDEF_R0);
                        FMemory::Memcpy(&pmxVertexPtr.SDEF_R0, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    for (int bw = 0; bw < 1; ++bw)
                    {
                        //
                        memcopySize = sizeof(pmxVertexPtr.SDEF_R1);
                        FMemory::Memcpy(&pmxVertexPtr.SDEF_R1, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                }
                else
                {
                    UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Import FALSE/Return /UnCorrect EncodeType"));
                }
                // 边缘倍率材质相对于边缘尺寸的倍率值
                memcopySize = sizeof(pmxVertexPtr.ToonEdgeScale);
                FMemory::Memcpy(&pmxVertexPtr.ToonEdgeScale, Buffer, memcopySize);
                Buffer += memcopySize;
            }
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning,
                   TEXT("PMX Import [Vertex:: statics bone type, bdef1 = %u] Complete"), statics_bdef1);
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning,
                   TEXT("PMX Import [Vertex:: statics bone type, bdef2 = %u] Complete"), statics_bdef2);
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning,
                   TEXT("PMX Import [Vertex:: statics bone type, bdef3 = %u] Complete"), statics_bdef4);
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning,
                   TEXT("PMX Import [Vertex:: statics bone type, sdef = %u] Complete"), statics_sdef);
        }
        UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [Vertex] Complete"));
        ////////////////////////////////////////////
        {
            /*
            ●面
            n：顶点索引大小
            ※3点（3顶点Index）1面
            每个材质的面数由材质内的面（顶点）数管理（同PMD方式）
            */
            uint32 PmxFaceNum = 0;
            memcopySize = sizeof(PmxFaceNum);
            FMemory::Memcpy(&PmxFaceNum, Buffer, memcopySize);
            Buffer += memcopySize;

            PmxFaceNum /= 3;

            faseList.AddZeroed(PmxFaceNum);
            for (uint32 FaceIndex = 0; FaceIndex < PmxFaceNum; ++FaceIndex)
            {
                PMX_FACE &pmxFaseListPtr = faseList[FaceIndex];
                //
                for (int SubNum = 0; SubNum < 3; ++SubNum)
                {
                    pmxFaseListPtr.VertexIndex[SubNum] = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.VertexIndexSize);
                }
            }
        }
        UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [FaceList] Complete"));
        ////////////////////////////////////////////
        {

            // 获取纹理数
            uint32 PmxTextureNum = 0;
            //
            memcopySize = sizeof(PmxTextureNum);
            FMemory::Memcpy(&PmxTextureNum, Buffer, memcopySize);
            Buffer += memcopySize;

            // 确保存储纹理数据的存储区域
            textureList.AddZeroed(PmxTextureNum);

            // 获取纹理信息
            for (uint32 i = 0; i < PmxTextureNum; i++)
            {
                textureList[i].TexturePath = PMXTexBufferToFString(&Buffer, pmxEncodeType);
            }
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [textureList] Complete"));
        }
        {
            // 获取材质数量
            uint32 PmxMaterialNum = 0;
            //
            memcopySize = sizeof(PmxMaterialNum);
            FMemory::Memcpy(&PmxMaterialNum, Buffer, memcopySize);
            Buffer += memcopySize;

            // 确保存储材质数据的存储空间
            materialList.AddZeroed(PmxMaterialNum);

            // 读取材质
            for (uint32 i = 0; i < PmxMaterialNum; i++)
            {
                // 材质名称的取得
                // materialList[i].Name = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                materialList[i].Name = FString::Printf(TEXT("%d_%s"), i, *PMXTexBufferToFString(&Buffer, pmxEncodeType));
                materialList[i].NameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);

                // Diffuse (R,G,B,A)
                memcopySize = sizeof(materialList[i].Diffuse);
                FMemory::Memcpy(materialList[i].Diffuse, Buffer, memcopySize);
                Buffer += memcopySize;
                // Specular (R,G,B)
                memcopySize = sizeof(materialList[i].Specular);
                FMemory::Memcpy(materialList[i].Specular, Buffer, memcopySize);
                Buffer += memcopySize;
                // Specular係数
                memcopySize = sizeof(materialList[i].SpecularPower);
                FMemory::Memcpy(&materialList[i].SpecularPower, Buffer, memcopySize);
                Buffer += memcopySize;
                // Ambient (R,G,B)
                memcopySize = sizeof(materialList[i].Ambient);
                FMemory::Memcpy(materialList[i].Ambient, Buffer, memcopySize);
                Buffer += memcopySize;

                /*
                描绘标志（8bit）-各bit0:OFF 1:ON
                0x01:双面绘制，0x02:地面阴影，0x04:绘制自阴影贴图，0x08:绘制自阴影，0x10：边绘制
                */
                uint8 tempByte = 0;
                memcopySize = sizeof(tempByte);
                FMemory::Memcpy(&tempByte, Buffer, memcopySize);
                Buffer += memcopySize;
                materialList[i].CullingOff = (tempByte & 0x01) ? 1 : 0;
                materialList[i].GroundShadow = (tempByte & 0x02) ? 1 : 0;
                materialList[i].SelfShadowMap = (tempByte & 0x04) ? 1 : 0;
                materialList[i].SelfShadowDraw = (tempByte & 0x08) ? 1 : 0;
                materialList[i].EdgeDraw = (tempByte & 0x10) ? 1 : 0;

                // 边缘颜色 (R,G,B,A)
                memcopySize = sizeof(materialList[i].EdgeColor);
                FMemory::Memcpy(materialList[i].EdgeColor, Buffer, memcopySize);
                Buffer += memcopySize;

                // 边缘大小
                memcopySize = sizeof(materialList[i].EdgeSize);
                FMemory::Memcpy(&materialList[i].EdgeSize, Buffer, memcopySize);
                Buffer += memcopySize;
                // 纹理，纹理表参考索引
                materialList[i].TextureIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.TextureIndexSize);
                // 球形纹理，纹理表参照Index※纹理扩展名无限制
                materialList[i].SphereTextureIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.TextureIndexSize);
                // 球形模式0:无效1:乘法（sph）2:加法（spa）
                // 3:子纹理（通过参考UV1中的x，y进行常规纹理绘制）
                memcopySize = sizeof(materialList[i].SphereMode);
                FMemory::Memcpy(&materialList[i].SphereMode, Buffer, memcopySize);
                Buffer += memcopySize;
                // 共享Toon标志0：单独Toon1:共享连续值Toon
                memcopySize = sizeof(materialList[i].ToonFlag);
                FMemory::Memcpy(&materialList[i].ToonFlag, Buffer, memcopySize);
                Buffer += memcopySize;

                if (materialList[i].ToonFlag == 0)
                { // Toon纹理，纹理表参考索引
                    materialList[i].ToonTextureIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.TextureIndexSize);
                }
                else
                { // 共享Toon纹理[0-9]->分别支持toon01.bmp～toon10.bmp
                    memcopySize = sizeof(uint8);
                    FMemory::Memcpy(&materialList[i].ToonTextureIndex, Buffer, memcopySize);
                    Buffer += memcopySize;
                }

                // 跳过注释
                PMXTexBufferToFString(&Buffer, pmxEncodeType);
                // 材质对应的面（顶点）数（一定是3的倍数）
                memcopySize = sizeof(materialList[i].MaterialFaceNum);
                FMemory::Memcpy(&materialList[i].MaterialFaceNum, Buffer, memcopySize);
                Buffer += memcopySize;
            }
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [materialList] Complete"));
        }
        ////////////////////////////////////////////

        uint32 offsetBoneIndex = 1;
        {
            // 为UE5添加所有父Bone

            // 获取骨骼数
            uint32 PmxBoneNum = 0;

#if 1
            // 确保存储骨骼数据的存储空间
            boneList.AddZeroed(1);
            // 获取骨骼信息
            {
                boneList[PmxBoneNum].Name = TEXT("Root");
                boneList[PmxBoneNum].NameEng = "AllTopRootBone";
                boneList[PmxBoneNum].Position = FVector3f(0);
                boneList[PmxBoneNum].ParentBoneIndex = INDEX_NONE;
            }
#endif
            //
            memcopySize = sizeof(PmxBoneNum);
            FMemory::Memcpy(&PmxBoneNum, Buffer, memcopySize);
            Buffer += memcopySize;

            // 确保存储骨骼数据的存储空间
            boneList.AddZeroed(PmxBoneNum);

            // 获取骨骼信息
            uint32 PmxIKNum = 0;
            for (uint32 i = 1; i < PmxBoneNum + offsetBoneIndex; i++)
            {

                boneList[i].Name = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                boneList[i].NameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                //
                memcopySize = sizeof(boneList[i].Position);
                FMemory::Memcpy(&boneList[i].Position, Buffer, memcopySize);
                boneList[i].Position = ConvertVectorAsixToUE5FromMMD(boneList[i].Position) * modelScale;
                Buffer += memcopySize;

                boneList[i].ParentBoneIndex = MMDExtendBufferSizeToInt32(&Buffer, this->baseHeader.BoneIndexSize) + offsetBoneIndex;
                //
                memcopySize = sizeof(boneList[i].TransformLayer);
                FMemory::Memcpy(&boneList[i].TransformLayer, Buffer, memcopySize);
                Buffer += memcopySize;

                uint16 Flag;
                //
                memcopySize = sizeof(Flag);
                FMemory::Memcpy(&Flag, Buffer, memcopySize);
                Buffer += memcopySize;

                boneList[i].Flag_LinkDest = (Flag & 0x0001) != 0 ? 1 : 0;
                boneList[i].Flag_EnableRot = (Flag & 0x0002) != 0 ? 1 : 0;
                boneList[i].Flag_EnableMov = (Flag & 0x0004) != 0 ? 1 : 0;
                boneList[i].Flag_Disp = (Flag & 0x0008) != 0 ? 1 : 0;
                boneList[i].Flag_EnableControl = (Flag & 0x0010) != 0 ? 1 : 0;
                boneList[i].Flag_IK = (Flag & 0x0020) != 0 ? 1 : 0;
                boneList[i].Flag_AddRot = (Flag & 0x0100) != 0 ? 1 : 0;
                boneList[i].Flag_AddMov = (Flag & 0x0200) != 0 ? 1 : 0;
                boneList[i].Flag_LockAxis = (Flag & 0x0400) != 0 ? 1 : 0;
                boneList[i].Flag_LocalAxis = (Flag & 0x0800) != 0 ? 1 : 0;
                boneList[i].Flag_AfterPhysicsTransform = (Flag & 0x1000) != 0 ? 1 : 0;
                boneList[i].Flag_OutParentTransform = (Flag & 0x2000) != 0 ? 1 : 0;

                if (boneList[i].Flag_LinkDest == 0)
                {
                    //
                    memcopySize = sizeof(boneList[i].OffsetPosition);
                    FMemory::Memcpy(&boneList[i].OffsetPosition, Buffer, memcopySize);
                    boneList[i].OffsetPosition = ConvertVectorAsixToUE5FromMMD(boneList[i].OffsetPosition) * modelScale;
                    Buffer += memcopySize;
                }
                else
                {
                    boneList[i].LinkBoneIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                }

                if (boneList[i].Flag_AddRot == 1 || boneList[i].Flag_AddMov == 1)
                {
                    boneList[i].AddParentBoneIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize);
                    //
                    memcopySize = sizeof(boneList[i].AddRatio);
                    FMemory::Memcpy(&boneList[i].AddRatio, Buffer, memcopySize);
                    Buffer += memcopySize;
                }

                if (boneList[i].Flag_LockAxis == 1)
                {
                    //
                    memcopySize = sizeof(boneList[i].LockAxisVector);
                    FMemory::Memcpy(&boneList[i].LockAxisVector, Buffer, memcopySize);
                    Buffer += memcopySize;
                }

                if (boneList[i].Flag_LocalAxis == 1)
                {

                    //
                    memcopySize = sizeof(boneList[i].LocalAxisXVector);
                    FMemory::Memcpy(&boneList[i].LocalAxisXVector, Buffer, memcopySize);
                    Buffer += memcopySize;

                    //
                    memcopySize = sizeof(boneList[i].LocalAxisZVector);
                    FMemory::Memcpy(&boneList[i].LocalAxisZVector, Buffer, memcopySize);
                    Buffer += memcopySize;
                }

                if (boneList[i].Flag_OutParentTransform == 1)
                {
                    //
                    memcopySize = sizeof(boneList[i].OutParentTransformKey);
                    FMemory::Memcpy(&boneList[i].OutParentTransformKey, Buffer, memcopySize);
                    Buffer += memcopySize;
                }

                if (boneList[i].Flag_IK == 1)
                {
                    PmxIKNum++;

                    boneList[i].IKInfo.TargetBoneIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize) + offsetBoneIndex;
                    //
                    memcopySize = sizeof(boneList[i].IKInfo.LoopNum);
                    FMemory::Memcpy(&boneList[i].IKInfo.LoopNum, Buffer, memcopySize);
                    Buffer += memcopySize;

                    //
                    memcopySize = sizeof(boneList[i].IKInfo.RotLimit);
                    FMemory::Memcpy(&boneList[i].IKInfo.RotLimit, Buffer, memcopySize);
                    Buffer += memcopySize;
                    // fix to Dig From rad(pmx) for vmd ik
                    boneList[i].IKInfo.RotLimit = FMath::RadiansToDegrees(boneList[i].IKInfo.RotLimit);

                    //
                    memcopySize = sizeof(boneList[i].IKInfo.LinkNum);
                    FMemory::Memcpy(&boneList[i].IKInfo.LinkNum, Buffer, memcopySize);
                    Buffer += memcopySize;
                    if (boneList[i].IKInfo.LinkNum >= PMX_MAX_IKLINKNUM)
                    {
                        return false;
                    }

                    for (int32 j = 0; j < boneList[i].IKInfo.LinkNum; j++)
                    {
                        boneList[i].IKInfo.Link[j].BoneIndex = MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize) + offsetBoneIndex;
                        //
                        memcopySize = sizeof(boneList[i].IKInfo.Link[j].RotLockFlag);
                        FMemory::Memcpy(&boneList[i].IKInfo.Link[j].RotLockFlag, Buffer, memcopySize);
                        Buffer += memcopySize;

                        if (boneList[i].IKInfo.Link[j].RotLockFlag == 1)
                        {
                            FVector3f tempVecRot;
                            //
                            memcopySize = sizeof(boneList[i].IKInfo.Link[j].RotLockMin);
                            FMemory::Memcpy(&boneList[i].IKInfo.Link[j].RotLockMin[0], Buffer, memcopySize);
                            Buffer += memcopySize;
                            tempVecRot = FVector3f(
                                boneList[i].IKInfo.Link[j].RotLockMin[0],
                                boneList[i].IKInfo.Link[j].RotLockMin[1],
                                boneList[i].IKInfo.Link[j].RotLockMin[2]);
                            tempVecRot = ConvertVectorAsixToUE5FromMMD(tempVecRot);
                            // fix to Dig From rad(pmx) for vmd ik
                            boneList[i].IKInfo.Link[j].RotLockMin[0] = FMath::RadiansToDegrees(tempVecRot.X);
                            boneList[i].IKInfo.Link[j].RotLockMin[1] = FMath::RadiansToDegrees(tempVecRot.Y);
                            boneList[i].IKInfo.Link[j].RotLockMin[2] = FMath::RadiansToDegrees(tempVecRot.Z);
                            //
                            memcopySize = sizeof(boneList[i].IKInfo.Link[j].RotLockMax);
                            FMemory::Memcpy(&boneList[i].IKInfo.Link[j].RotLockMax[0], Buffer, memcopySize);
                            Buffer += memcopySize;
                            tempVecRot = FVector3f(
                                boneList[i].IKInfo.Link[j].RotLockMax[0],
                                boneList[i].IKInfo.Link[j].RotLockMax[1],
                                boneList[i].IKInfo.Link[j].RotLockMax[2]);
                            tempVecRot = ConvertVectorAsixToUE5FromMMD(tempVecRot);
                            // 已修复vmd-ik从rad（pmx）挖掘的问题
                            boneList[i].IKInfo.Link[j].RotLockMax[0] = FMath::RadiansToDegrees(tempVecRot.X);
                            boneList[i].IKInfo.Link[j].RotLockMax[1] = FMath::RadiansToDegrees(tempVecRot.Y);
                            boneList[i].IKInfo.Link[j].RotLockMax[2] = FMath::RadiansToDegrees(tempVecRot.Z);
                        }
                    }
                }
            }
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [BoneList] Complete"));
        }
        {
            int32 i, j;
            // 获取变形信息的数量
            int32 PmxMorphNum = 0;

            //
            memcopySize = sizeof(PmxMorphNum);
            FMemory::Memcpy(&PmxMorphNum, Buffer, memcopySize);
            Buffer += memcopySize;

            // 确保存储变形数据的存储区域
            morphList.AddZeroed(PmxMorphNum);

            // 导入变形信息
            int32 PmxSkinNum = 0;
            for (i = 0; i < PmxMorphNum; i++)
            {
                morphList[i].Name = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                morphList[i].NameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);

                //
                memcopySize = sizeof(morphList[i].ControlPanel);
                FMemory::Memcpy(&morphList[i].ControlPanel, Buffer, memcopySize);
                Buffer += memcopySize;
                //
                memcopySize = sizeof(morphList[i].Type);
                FMemory::Memcpy(&morphList[i].Type, Buffer, memcopySize);
                Buffer += memcopySize;
                //
                memcopySize = sizeof(morphList[i].DataNum);
                FMemory::Memcpy(&morphList[i].DataNum, Buffer, memcopySize);
                Buffer += memcopySize;

                switch (morphList[i].Type)
                {
                case 0: // 组合变形
                    morphList[i].Group.AddZeroed(morphList[i].DataNum);

                    for (j = 0; j < morphList[i].DataNum; j++)
                    {
                        morphList[i].Group[j].Index =
                            MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.MorphIndexSize);
                        //
                        memcopySize = sizeof(morphList[i].Group[j].Ratio);
                        FMemory::Memcpy(&morphList[i].Group[j].Ratio, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    break;

                case 1: // 頂点
                    PmxSkinNum++;
                    morphList[i].Vertex.AddZeroed(morphList[i].DataNum);

                    for (j = 0; j < morphList[i].DataNum; j++)
                    {
                        morphList[i].Vertex[j].Index =
                            MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.VertexIndexSize);
                        //
                        memcopySize = sizeof(morphList[i].Vertex[j].Offset);
                        FMemory::Memcpy(&morphList[i].Vertex[j].Offset, Buffer, memcopySize);
                        morphList[i].Vertex[j].Offset = ConvertVectorAsixToUE5FromMMD(morphList[i].Vertex[j].Offset) * modelScale;
                        Buffer += memcopySize;
                    }
                    break;

                case 2: // 骨骼变形
                    morphList[i].Bone.AddZeroed(morphList[i].DataNum);

                    for (j = 0; j < morphList[i].DataNum; j++)
                    {
                        morphList[i].Bone[j].Index =
                            MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.BoneIndexSize) + offsetBoneIndex;
                        //
                        memcopySize = sizeof(morphList[i].Bone[j].Offset);
                        FMemory::Memcpy(&morphList[i].Bone[j].Offset, Buffer, memcopySize);
                        morphList[i].Bone[j].Offset = ConvertVectorAsixToUE5FromMMD(morphList[i].Bone[j].Offset) * modelScale;
                        Buffer += memcopySize;
                        //
                        memcopySize = sizeof(morphList[i].Bone[j].Quat);
                        FMemory::Memcpy(&morphList[i].Bone[j].Quat, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    break;

                case 3: // UV变形
                case 4: // 追加UV1变形
                case 5: // 追加UV2变形
                case 6: // 追加UV3变形
                case 7: // 追加UV4变形
                    morphList[i].UV.AddZeroed(morphList[i].DataNum);

                    for (j = 0; j < morphList[i].DataNum; j++)
                    {
                        morphList[i].UV[j].Index =
                            MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.VertexIndexSize);
                        //
                        memcopySize = sizeof(morphList[i].UV[j].Offset);
                        FMemory::Memcpy(&morphList[i].UV[j].Offset, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    break;

                case 8: // 材质变形
                    morphList[i].Material.AddZeroed(morphList[i].DataNum);

                    for (j = 0; j < morphList[i].DataNum; j++)
                    {
                        morphList[i].Material[j].Index =
                            MMDExtendBufferSizeToUint32(&Buffer, this->baseHeader.MaterialIndexSize);
                        //
                        memcopySize = sizeof(morphList[i].Material[j].CalcType);
                        FMemory::Memcpy(&morphList[i].Material[j].CalcType, Buffer, memcopySize);
                        Buffer += memcopySize;

                        //
                        memcopySize = sizeof(morphList[i].Material[j].Diffuse);
                        FMemory::Memcpy(&morphList[i].Material[j].Diffuse, Buffer, memcopySize);
                        Buffer += memcopySize;
                        //
                        memcopySize = sizeof(morphList[i].Material[j].Specular);
                        FMemory::Memcpy(&morphList[i].Material[j].Specular, Buffer, memcopySize);
                        Buffer += memcopySize;
                        //
                        memcopySize = sizeof(morphList[i].Material[j].SpecularPower);
                        FMemory::Memcpy(&morphList[i].Material[j].SpecularPower, Buffer, memcopySize);
                        Buffer += memcopySize;
                        //
                        memcopySize = sizeof(morphList[i].Material[j].Ambient);
                        FMemory::Memcpy(&morphList[i].Material[j].Ambient, Buffer, memcopySize);
                        Buffer += memcopySize;

                        //
                        memcopySize = sizeof(morphList[i].Material[j].EdgeColor);
                        FMemory::Memcpy(&morphList[i].Material[j].EdgeColor, Buffer, memcopySize);
                        Buffer += memcopySize;

                        //
                        memcopySize = sizeof(morphList[i].Material[j].EdgeSize);
                        FMemory::Memcpy(&morphList[i].Material[j].EdgeSize, Buffer, memcopySize);
                        Buffer += memcopySize;

                        //
                        memcopySize = sizeof(morphList[i].Material[j].TextureScale);
                        FMemory::Memcpy(&morphList[i].Material[j].TextureScale, Buffer, memcopySize);
                        Buffer += memcopySize;

                        //
                        memcopySize = sizeof(morphList[i].Material[j].SphereTextureScale);
                        FMemory::Memcpy(&morphList[i].Material[j].SphereTextureScale, Buffer, memcopySize);
                        Buffer += memcopySize;
                        //
                        memcopySize = sizeof(morphList[i].Material[j].ToonTextureScale);
                        FMemory::Memcpy(&morphList[i].Material[j].ToonTextureScale, Buffer, memcopySize);
                        Buffer += memcopySize;
                    }
                    break;
                }
            }
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [MorphList] Complete"));
        }

        { // Displayframe
            int32 i, j;

            int32 PmxMorphNum = 0;

            memcopySize = sizeof(PmxMorphNum);
            FMemory::Memcpy(&PmxMorphNum, Buffer, memcopySize);
            Buffer += memcopySize;

            morphList.AddZeroed(PmxMorphNum);

            int32 PmxSkinNum = 0;
            for (i = 0; i < PmxMorphNum; i++)
            {
                FString name = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                FString nameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);
                Buffer += 1; // flag
                int32_t targetCount;
                readBuffer(targetCount);
                for (j = 0; j < targetCount; j++)
                {
                    uint8_t targetType;
                    readBuffer(targetType);
                    if (targetType == 0)
                        Buffer += this->baseHeader.BoneIndexSize;
                    else if (targetType == 1)
                        Buffer += this->baseHeader.MorphIndexSize;
                    else
                        throw "Dispframe";
                }
            }
        }

        { // RigidBody
            int32 i;

            int32 rbNum = 0;
            readBuffer(rbNum);

            rigidList.AddZeroed(rbNum);

            int32 PmxSkinNum = 0;
            for (i = 0; i < rbNum; i++)
            {
                PMX_RIGIDBODY &rb = rigidList[i];
                rb.Name = PMXTexBufferToFString(&Buffer, pmxEncodeType);

                rb.NameEng = PMXTexBufferToFString(&Buffer, pmxEncodeType);

                // UE_LOG print this->baseHeader.BoneIndexSize
                rb.BoneIndex = MMDExtendBufferSizeToInt32(&Buffer, this->baseHeader.BoneIndexSize) + offsetBoneIndex;
                // UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Import [RigidBody] BoneIndexSize %d"), rb.BoneIndex);

                auto bone = boneList[rb.BoneIndex];
                rb.fnName = FName(bone.Name);
                readBuffer(rb.RigidBodyGroupIndex);
                readBuffer(rb.RigidBodyGroupTarget);
                readBuffer(rb.ShapeType);
                readBuffer(rb.Size);
                rb.Size *= modelScale;
                readBuffer(rb.Position);
                rb.Position = ConvertVectorAsixToUE5FromMMD(rb.Position) * modelScale - bone.Position;
                readBuffer(rb.Rotation); // rb.Rotation = ConvertVectorAsixToUE5FromMMD(rb.Rotation);
                auto rx = glm::rotate(glm::mat4(1), rb.Rotation.X, glm::vec3(1, 0, 0));
                auto ry = glm::rotate(glm::mat4(1), rb.Rotation.Y, glm::vec3(0, 1, 0));
                auto rz = glm::rotate(glm::mat4(1), rb.Rotation.Z, glm::vec3(0, 0, 1));
                glm::mat4 rotMat = ry * rx * rz;
                glm::quat q(rotMat);
                rb.Quat = FQuat(q.x, -q.z, q.y, q.w);
                readBuffer(rb.Mass);
                readBuffer(rb.PosDim);
                readBuffer(rb.RotDim);
                readBuffer(rb.Recoil);
                readBuffer(rb.Friction);
                readBuffer(rb.OpType);
            }
        }

        /*骨骼索引修改*/
        if (false == FixSortParentBoneIndex())
        {
            /*BoneIndexSort NG*/
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Importer Class FAULT: Bone Index NG?"));

            // 读取模型后的警告文显示：注释栏
            const FText MessageDbg = FText(LOCTEXT("PMX_FormatNG_Dbg_sg", "[Restriction]:IVP5U Plugin / Bone Index Sort NG.\n\
					要读取的PMX数据的Bone结构可能超出预期。\n\
					当前PluginVer很有可能发生编辑器崩溃，因此建议中断读取处理。\n\
					（原因例：Bone的父子构成的Index宣言顺序颠倒了？etc.）\n\
					\n\
					是否中断（是）？\n\
					是？“是否”？"));
            if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, MessageDbg))
            {
                UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Importer Class STOP"));
                return false;
            }
        }
        //////////////////////////////////////////////
        UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Importer Class Complete: PMXLoaderBinary"));

        Buf = Buffer;
        return true;
    }

    bool PmxMeshInfo::FixSortParentBoneIndex()
    {
        bool bRet = true;
        //////////////////////////////////////////////
        UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Importer Class Start: FixSortParentBoneIndex"));

        /////
        int32 i; // , j;
        // 获取变形信息的数量
        int32 boneNum = boneList.Num();

        // init
        fixedHierarchyBone.AddZeroed(boneNum);

        for (i = 0; i < boneNum; i++)
        {
            // 保留基本的BoneIndex
            fixedHierarchyBone[i].originalBoneIndex = i;

            // 确定父骨骼的顺序：ok is, parent index < self
            if (boneList[i].ParentBoneIndex < i)
            {
                fixedHierarchyBone[i].fixFlag_Parent = false;
            }
            else
            {
                // NG
                fixedHierarchyBone[i].fixFlag_Parent = true;
                UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning,
                       TEXT("PMX Importer : FixSortParentBoneIndex : parent ng. P:%u > S:%u"), boneList[i].ParentBoneIndex, i);
                bRet = false;
            }
        }
        if (bRet == false)
        {
            UE_LOG(LogMMD4UE5_PmxMeshInfo, Error, TEXT("PMX Importer Class Error: FixSortParentBoneIndex NG?"));
        }
        //////////////////////////////////////////////
        UE_LOG(LogMMD4UE5_PmxMeshInfo, Warning, TEXT("PMX Importer Class Complete: FixSortParentBoneIndex"));
        return bRet;
    }

#undef LOCTEXT_NAMESPACE
}