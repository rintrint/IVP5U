// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"

#include "MMDImportHelper.h"
#include "PmxImportUI.h"
#include "MMDExtendAsset.h"

#include "MMDStaticMeshImportData.h"

/////////////////////////////////////////////////
// Copy From DxLib DxModelLoader4.h
// DX Library Copyright (C) 2001-2008 Takumi Yamada.

// #define uint8 (unsigned char)

namespace MMD4UE5
{

	// 宏定义 -----------------------------------

#define PMX_MAX_IKLINKNUM (64) // 对应的IK链路的最大数目

	// 存储PMX文件信息的结构体
	struct PMX_BASEINFO
	{
		uint8 EncodeType;		 // 字符代码编码类型0:UTF16:UTF8
		uint8 UVNum;			 // 追加UV数（0～4）
		uint8 VertexIndexSize;	 // 顶点索引大小（1或2或4）
		uint8 TextureIndexSize;	 // 纹理索引大小（1或2或4）
		uint8 MaterialIndexSize; // 材质索引大小（1或2或4）
		uint8 BoneIndexSize;	 // 骨骼索引大小（1或2或4）
		uint8 MorphIndexSize;	 // 变形索引大小（1或2或4）
		uint8 RigidIndexSize;	 // 刚体索引大小（1或2或4）
	};

	// 顶点数据
	struct PMX_VERTEX
	{
		FVector3f Position;	 // 坐标
		FVector3f Normal;	 // 法线
		FVector2f UV;		 // 标准UV值
		FVector4f AddUV[4];	 // 追加UV值
		uint8 WeightType;	 // 权重类型（0:BDEF1:BDEF2:BDEF4:SDEF）
		uint32 BoneIndex[4]; // 骨骼索引
		float BoneWeight[4]; // 骨骼重量
		FVector3f SDEF_C;	 // SDEF-C
		FVector3f SDEF_R0;	 // SDEF-R0
		FVector3f SDEF_R1;	 // SDEF-R1
		float ToonEdgeScale; // 缩放图文边
	};

	// 面列表
	struct PMX_FACE
	{
		uint32 VertexIndex[3]; // 頂点IndexList
	};

	// 纹理信息
	struct PMX_TEXTURE
	{
		FString TexturePath;
	};

	// 材质信息
	struct PMX_MATERIAL
	{
		FString Name;	 // 名
		FString NameEng; // 名e

		float Diffuse[4];	 // 扩散色
		float Specular[3];	 // 镜面反射颜色
		float SpecularPower; // 镜面反射常数
		float Ambient[3];	 // 环境色

		uint8 CullingOff;	  // 双面绘制
		uint8 GroundShadow;	  // 地面阴影影
		uint8 SelfShadowMap;  // 在自阴影贴图中绘制
		uint8 SelfShadowDraw; // 绘制自阴影
		uint8 EdgeDraw;		  // 绘制边

		float EdgeColor[4]; // 边缘颜色
		float EdgeSize;		// 边缘大小

		int TextureIndex;		// 常规纹理索引
		int SphereTextureIndex; // 球形纹理索引
		uint8 SphereMode;		// 球形模式（0:无效1:乘法2:加法3:子纹理（通过UV参照追加UV1的x，y进行通常纹理描绘）

		uint8 ToonFlag;		  // 共享标记（0：单个Toon1:共享Toon）
		int ToonTextureIndex; // 纹理索引

		int MaterialFaceNum; // 材质适应的面数
	};

	// IK链接信息
	struct PMX_IKLINK
	{
		int BoneIndex;		 // リンクボーンのインデックス
		uint8 RotLockFlag;	 // 回転制限( 0:OFF 1:ON )
		float RotLockMin[3]; // 回転制限、下限
		float RotLockMax[3]; // 回転制限、上限
	};

	// ＩＫ情報
	struct PMX_IK
	{
		int TargetBoneIndex; // IKターゲットのボーンインデックス
		int LoopNum;		 // IK計算のループ回数
		float RotLimit;		 // 計算一回辺りの制限角度

		int LinkNum;						// ＩＫリンクの数
		PMX_IKLINK Link[PMX_MAX_IKLINKNUM]; // ＩＫリンク情報
	};

	// 骨骼信息
	struct PMX_BONE
	{
		FString Name;		 // 名前
		FString NameEng;	 // 名前
		FVector3f Position;	 // 座標
		int ParentBoneIndex; // 父骨骼索引
		int TransformLayer;	 // 变形层次

		uint8 Flag_LinkDest;			  // 连接地址
		uint8 Flag_EnableRot;			  // 能否旋转
		uint8 Flag_EnableMov;			  // 能否移动
		uint8 Flag_Disp;				  // 显示
		uint8 Flag_EnableControl;		  // 是否可操作
		uint8 Flag_IK;					  // IK
		uint8 Flag_AddRot;				  // 赋予旋转
		uint8 Flag_AddMov;				  // 赋予移动
		uint8 Flag_LockAxis;			  // 轴固定
		uint8 Flag_LocalAxis;			  // 局部轴
		uint8 Flag_AfterPhysicsTransform; // 物理后变形
		uint8 Flag_OutParentTransform;	  // 外部父变形

		FVector3f OffsetPosition;	// 偏移坐标
		int LinkBoneIndex;			// 目标骨骼索引
		int AddParentBoneIndex;		// 授予的父骨骼索引
		float AddRatio;				// 授予率
		FVector3f LockAxisVector;	// 轴固定时轴的方向矢量
		FVector3f LocalAxisXVector; // 本地轴的X轴
		FVector3f LocalAxisZVector; // 局部轴的Z轴
		int OutParentTransformKey;	// 外部父代变形时的Key值

		PMX_IK IKInfo; // ＩＫ情報

		FTransform3f absTF;
	};

	// 顶点变形信息
	struct PMX_MORPH_VERTEX
	{
		int Index;		  // 頂点インデックス
		FVector3f Offset; // 頂点座標オフセット
	};

	// UV变形信息
	struct PMX_MORPH_UV
	{
		int Index;		 // 頂点インデックス
		float Offset[4]; // 頂点ＵＶオフセット
	};

	// 骨骼变形信息
	struct PMX_MORPH_BONE
	{
		int Index;		  // ボーンインデックス
		FVector3f Offset; // 座標オフセット
		float Quat[4];	  // 回転クォータニオン
	};

	// 材质变形信息
	struct PMX_MORPH_MATERIAL
	{
		int Index;					 // マテリアルインデックス
		uint8 CalcType;				 // 計算タイプ( 0:乗算  1:加算 )
		float Diffuse[4];			 // ディフューズカラー
		float Specular[3];			 // スペキュラカラー
		float SpecularPower;		 // スペキュラ係数
		float Ambient[3];			 // アンビエントカラー
		float EdgeColor[4];			 // エッジカラー
		float EdgeSize;				 // エッジサイズ
		float TextureScale[4];		 // テクスチャ係数
		float SphereTextureScale[4]; // スフィアテクスチャ係数
		float ToonTextureScale[4];	 // トゥーンテクスチャ係数
	};

	// 组合变形
	struct PMX_MORPH_GROUP
	{
		int Index;	 // モーフインデックス
		float Ratio; // モーフ率
	};

	// 变形信息
	struct PMX_MORPH
	{
		FString Name;	 // 名前
		FString NameEng; // 名前

		uint8 ControlPanel; // 操作パネル
		uint8 Type;			// モーフの種類  0:グループ 1:頂点 2:ボーン 3:UV 4:追加UV1 5:追加UV2 6:追加UV3 7:追加UV4 8:材質

		int DataNum; // モーフ情報の数

		TArray<PMX_MORPH_VERTEX> Vertex;	 // 頂点モーフ
		TArray<PMX_MORPH_UV> UV;			 // UVモーフ
		TArray<PMX_MORPH_BONE> Bone;		 // ボーンモーフ
		TArray<PMX_MORPH_MATERIAL> Material; // マテリアルモーフ
		TArray<PMX_MORPH_GROUP> Group;		 // グループモーフ
	};

	// 剛体情報
	struct PMX_RIGIDBODY
	{
		FString Name; // 名前
		FName fnName;
		FString NameEng;
		uint32 BoneIndex; // 対象ボーン番号

		uint8 RigidBodyGroupIndex;	 // 剛体グループ番号
		uint16 RigidBodyGroupTarget; // 剛体グループ対象

		uint8 ShapeType; // 形状( 0:球  1:箱  2:カプセル )
		// float	ShapeW;							// 幅
		// float	ShapeH;							// 高さ
		// float	ShapeD;							// 奥行
		FVector3f Size;

		FVector3f Position; // 位置
		FVector3f Rotation; // 回転( ラジアン )
		FQuat Quat;
		float Mass;		// 質量
		float PosDim;	// 移動減衰
		float RotDim;	// 回転減衰
		float Recoil;	// 反発力
		float Friction; // 摩擦力

		uint8 OpType; // 剛体タイプ( 0:Bone追従  1:物理演算  2:物理演算(Bone位置合わせ) )
	};

	// 关节信息
	struct PMX_JOINT
	{
		FString Name; // 名前

		uint8 Type; // 種類( 0:スプリング6DOF ( PMX2.0 では 0 のみ )

		int RigidBodyAIndex; // 接続先剛体Ａ
		int RigidBodyBIndex; // 接続先剛体Ｂ

		FVector3f Position; // 位置
		float Rotation[3];	// 回転( ラジアン )

		float ConstrainPositionMin[3]; // 移動制限-下限
		float ConstrainPositionMax[3]; // 移動制限-上限
		float ConstrainRotationMin[3]; // 回転制限-下限
		float ConstrainRotationMax[3]; // 回転制限-上限

		float SpringPosition[3]; // バネ定数-移動
		float SpringRotation[3]; // バネ定数-回転
	};
	//////////////////////////////////////////////////////////////

	struct PMX_BONE_HIERARCHY
	{
		int originalBoneIndex;
		int fixedBoneIndex;
		bool fixFlag_Parent;
		// bool	fixFlag_Target;
	};
	//////////////////////////////////////////////////////////////

	DECLARE_LOG_CATEGORY_EXTERN(LogMMD4UE5_PmxMeshInfo, Log, All)

	////////////////////////////////////////////////////////////////////
	// Inport用 meta data 格納クラス
	class PmxMeshInfo : public MMDImportHelper
	{
		//////////////////////////////////////
		// Sort Parent Bone ( sort tree bones)
		// memo: ボーンの配列で子->親の順の場合、
		//       ProcessImportMeshSkeleton内部のcheckに引っかかりクラッシュするため。
		// how to: after PMXLoaderBinary func.
		//////////////////////////////////////
		bool FixSortParentBoneIndex();

		TArray<PMX_BONE_HIERARCHY> fixedHierarchyBone;

	public:
		PmxMeshInfo();
		~PmxMeshInfo();

		///////////////////////////////////////
		bool PMXLoaderBinary(
			const uint8*& Buffer,
			const uint8* BufferEnd);
		template <typename T>
		void readBuffer(T& var)
		{
			size_t memcopySize = sizeof(var);
			var = *(T*)Buffer;
			// FMemory::Memcpy(&var, Buffer, memcopySize);
			Buffer += memcopySize;
		}

		TArray<PMX_RIGIDBODY> findRigid(FName bname)
		{
			TArray<PMX_RIGIDBODY> list;
			for (int i = 0; i < rigidList.Num(); i++)
				if (rigidList[i].fnName == bname)
					list.Add(rigidList[i]);
			return list;
		}
		///////////////////////////////////////
		const uint8* Buffer;
		char magic[4];
		float formatVer;
		PMX_BASEINFO baseHeader;
		FString modelNameJP;
		FString modelNameEng;
		FString modelCommentJP;
		FString modelCommentEng;
		//
		TArray<PMX_VERTEX> vertexList;
		TArray<PMX_FACE> faseList;

		TArray<PMX_TEXTURE> textureList;
		TArray<PMX_MATERIAL> materialList;

		TArray<PMX_BONE> boneList;
		TArray<PMX_MORPH> morphList;
		TArray<PMX_RIGIDBODY> rigidList;
	};

} // namespace MMD4UE5

///////////////////////////////////////////////////////////////////
//	Compy Refafct FBImporter.h
/////////////////////////////////////////////

struct PMXImportOptions
{
	// General options
	bool bImportMaterials;
	bool bInvertNormalMap;
	bool bImportTextures;
	bool bCreateMaterialInstMode;
	bool bUnlitMaterials;
	bool bImportLOD;
	bool bUsedAsFullName;
	bool bConvertScene;
	bool bRemoveNameSpace;
	FVector ImportTranslation;
	FRotator ImportRotation;
	float ImportUniformScale;
	EMMDNormalImportMethod NormalImportMethod;
	// Static Mesh options
	bool bCombineToSingle;
	EVertexColorImportOptionMMD::Type VertexColorImportOption;
	FColor VertexOverrideColor;
	bool bRemoveDegenerates;
	bool bGenerateLightmapUVs;
	bool bOneConvexHullPerUCX;
	bool bAutoGenerateCollision;
	FName StaticMeshLODGroup;
	// Skeletal Mesh options
	bool bImportMorph;
	bool bImportAnimations;
	bool bUpdateSkeletonReferencePose;
	bool bResample;
	bool bImportRigidMesh;
	bool bUseT0AsRefPose;
	bool bPreserveSmoothingGroups;
	bool bKeepOverlappingVertices;
	bool bImportMeshesInBoneHierarchy;
	bool bCreatePhysicsAsset;
	UPhysicsAsset* PhysicsAsset;
	// Animation option
	USkeleton* SkeletonForAnimation;
	// EFBXAnimationLengthImportType AnimationLengthImportType;
	FIntPoint AnimationRange;
	FString AnimationName;
	bool bPreserveLocalTransform;
	bool bImportCustomAttribute;

	/*bool ShouldImportNormals()
	{
	return NormalImportMethod == FBXNIM_ImportNormals || NormalImportMethod == FBXNIM_ImportNormalsAndTangents;
	}

	bool ShouldImportTangents()
	{
	return NormalImportMethod == FBXNIM_ImportNormalsAndTangents;
	}

	void ResetForReimportAnimation()
	{
	bImportMorph = true;
	AnimationLengthImportType = FBXALIT_ExportedTime;
	}*/
	UAnimSequence* AnimSequenceAsset;
	UDataTable* MMD2UE5NameTableRow;
	UMMDExtendAsset* MmdExtendAsset;
};

PMXImportOptions* GetImportOptions(
	class FPmxImporter* PmxImporter,
	UPmxImportUI* ImportUI,
	bool bShowOptionDialog,
	const FString& FullPath,
	bool& bOutOperationCanceled,
	bool& bOutImportAll,
	bool bIsObjFormat,
	bool bForceImportType = false,
	EPMXImportType ImportType = PMXIT_StaticMesh);

void ApplyImportUIToImportOptions(
	UPmxImportUI* ImportUI,
	PMXImportOptions& InOutImportOptions);

/**
 * Main PMX Importer class.
 */
class FPmxImporter
{
public:
	~FPmxImporter();
	/**
	 * Returns the importer singleton. It will be created on the first request.
	 */
	IVP5U_API static FPmxImporter* GetInstance();
	static void DeleteInstance();

	/**
	 * Get the object of import options
	 *
	 * @return FBXImportOptions
	 */
	IVP5U_API PMXImportOptions* GetImportOptions() const;
public:
	PMXImportOptions* ImportOptions;

protected:
	static TSharedPtr<FPmxImporter> StaticInstance;

	FPmxImporter();

	/**
	 * Clean up for destroy the Importer.
	 */
	void CleanUp();
};
