// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"
#include "PmxImporter.h"
#include "MMDImportHelper.h"

namespace MMD4UE5
{

    // 宏定义----------------------------------
    // 数据类型定义---------------------------------
    // PMD数据头（283byte）
    struct PMD_HEADER
    {
        /*uint8	Data[283];						// 数据
         */
        char Magic[3];     // "Pmd"
        uint8 Version[4];  // 1.0f ( 0x3f800000 )
        char Name[20];     // 名前
        char Comment[256]; // 批注
    };

    // 每个顶点数据的信息（38byte）
    struct PMD_VERTEX
    {
        float Position[3]; // 坐标
        float Normal[3];   // 法线
        float Uv[2];       // 纹理坐标
        uint16 BoneNo[2];  // 骨骼编号
        uint8 BoneWeight;  // 骨骼在“0”和“0”之间的影响（0-100）骨骼在“1”之间的影响是（100-BoneWeight）
        uint8 Edge;        // 边标志0：启用边1:禁用边
    };

    // 顶点数据
    struct PMD_VERTEX_DATA
    {
        int Count; // 顶点数据数
                   /*这里有PMD_VERTEX只存在Count的数量 */
    };
    // 面列表
    struct PMD_FACE
    {
        uint16 VertexIndx[3]; // 顶点数（面数为VertexCount/3）
                              /* 这里，uint16的顶点索引仅存在VertexCount的数量 */
    };
    // 面列表
    struct PMD_FACE_DATA
    {
        uint32 VertexCount; // 顶点数（面数为 VertexCount / 3 )
                            /* 这里，uint16的顶点索引仅存在VertexCount的数量 */
    };

    // 每种材质的信息（70byte）
    struct PMD_MATERIAL
    {
        /*uint8	Data[70];						// 数据
         */
        float DiffuseR, DiffuseG, DiffuseB;    // 扩散色
        float Alpha;                           // α値
        float Specularity;                     // 镜面反射系数
        float SpecularR, SpecularG, SpecularB; // 镜面反射颜色
        float AmbientR, AmbientG, AmbientB;    // 环境色
        uint8 ToolImage;                       // 用于渲染的纹理索引
        uint8 Edge;                            // 轮廓、阴影
        int FaceVertexCount;                   // 使用此材质的面顶点数
        char TextureFileName[20];              // 纹理文件名
    };

    // 材质信息
    struct PMD_MATERIAL_DATA
    {
        int Count; // 材质数量
                   /*这里有PMD_MATERIAL只存在Count的数量*/
    };

    // 每个骨骼的信息（39byte）
    struct PMD_BONE
    {
        /*uint8	Data[39];						// 数据
         */
        char Name[20];     //  0:骨骼名称
        short Parent;      // 20:父骨骼（如果没有，则为0xffff）
        short TailPosBone; // 22:切线位置的骨骼（对于链末端为0xffff）
        uint8 Type;        // 24:骨骼类型（0:旋转1:旋转和移动2:IK3:未知4:IK影响下5:旋转影响下6:IK连接处7:隐藏8:扭转9:旋转运动）
        short IkParent;    // 25:IK骨骼编号（如果没有影响IK骨骼，则为0xffff）
        float HeadPos[3];  // 27:头部位置
    };

    // 骨骼信息
    struct PMD_BONE_DATA
    {
        uint16 Count; // 骨骼数
                      /* 这里有PMD_BONE仅存在Count数 */
    };

    // IK数据附近的信息（超出ChainBoneIndex的尺寸11byte）
    struct PMD_IK
    {
        /*uint8	Data[11];						// 数据
         */
        uint16 Bone;       // IK骨骼
        uint16 TargetBone; // IK目标骨骼
        uint8 ChainLength; // IK链长度（子代数）
        uint16 Iterations; // 重新计算次数
        // float	ControlWeight ;						// IK影响
        float RotLimit;                 // 单位限制角[dig]（仅适用于PMD系的Bone为膝盖时仅限X轴）
        TArray<uint16> ChainBoneIndexs; // IK影响下的骨骼编号
    };

    // IK信息
    struct PMD_IK_DATA
    {
        uint16 Count; // IK数据数
        /* 这里有PMD_IK仅存在Count数 */
    };

    // 表情顶点信息
    struct PMD_SKIN_VERT
    {
        int TargetVertexIndex; // 目标顶点索引
        float Position[3];     // 坐标（PMD_SKIN的SkinType为0时为绝对坐标，除此之外为相对于base的相对坐标）
    };

    // 一个表情附近的信息（超过顶点的大小25byte）
    struct PMD_SKIN
    {
        /*uint8	Data[25];						//数据
         */
        char Name[20];                // 表情名
        int VertexCount;              // 頂点の数
        uint8 SkinType;               // 表情の種類( 0:base 1：まゆ、2：目、3：リップ、4：その他 )
        TArray<PMD_SKIN_VERT> Vertex; // 表情用の頂点データ
    };

    // 表情の情報
    struct PMD_SKIN_DATA
    {
        uint16 Count; // 表情データの数
                      /* ここに PMD_SKIN が Count の数だけ存在する */
    };

#if 0
	// 物理演算データ一つ辺りの情報( 83byte )
	struct PMD_PHYSICS
	{
		uint8	Data[83];
		/*
		char	RigidBodyName[ 20 ] ;				//  0 : 剛体名
		uint16	RigidBodyRelBoneIndex ;				// 20 : 剛体関連ボーン番号
		uint8	RigidBodyGroupIndex ;				// 22 : 剛体グループ番号
		uint16	RigidBodyGroupTarget ;				// 23 : 剛体グループ対象
		uint8	ShapeType ;							// 25 : 形状( 0:球  1:箱  2:カプセル )
		float	ShapeW ;							// 26 : 幅
		float	ShapeH ;							// 30 : 高さ
		float	ShapeD ;							// 34 : 奥行
		float	Position[ 3 ] ;						// 38 : 位置
		float	Rotation[ 3 ] ;						// 50 : 回転( ラジアン )
		float	RigidBodyWeight ;					// 62 : 質量
		float	RigidBodyPosDim ;					// 66 : 移動減
		float	RigidBodyRotDim ;					// 70 : 回転減
		float	RigidBodyRecoil ;					// 74 : 反発力
		float	RigidBodyFriction ;					// 78 : 摩擦力
		uint8	RigidBodyType ;						// 82 : 剛体タイプ( 0:Bone追従  1:物理演算  2:物理演算(Bone位置合わせ) )
		*/
	};

	// 物理演算データの情報
	struct PMD_PHYSICS_DATA
	{
		int	Count;								// 剛体データの数
		/* ここに PMD_PHYSICS が Count の数だけ存在する */
	};

	// 物理演算用ジョイントデータ一つ辺りの情報( 124byte )
	struct PMD_PHYSICS_JOINT
	{
		char	Name[20];						// 名前
		int	RigidBodyA;						// 接続先剛体Ａ
		int	RigidBodyB;						// 接続先剛体Ｂ
		float	Position[3];						// 位置
		float	Rotation[3];						// 回転( ラジアン )
		float	ConstrainPosition1[3];			// 移動制限値１
		float	ConstrainPosition2[3];			// 移動制限値２
		float	ConstrainRotation1[3];			// 回転制限値１
		float	ConstrainRotation2[3];			// 回転制限値２
		float	SpringPosition[3];				// ばね移動値
		float	SpringRotation[3];				// ばね回転値
	};

	// 物理演算用ジョイント情報
	struct PMD_PHYSICS_JOINT_DATA
	{
		int	Count;								// ジョイントデータの数
		/* ここに PMD_PHYSICS_JOINT が Count の数だけ存在する */
	};

	// VMD読み込み処理用PMDボーンデータ
	struct PMD_READ_BONE_INFO
	{
		PMD_BONE			*Base;					// ボーン基データ
		MV1_ANIM_R			*Anim;					// このボーン用のアニメーション情報へのポインタ
		MV1_FRAME_R			*Frame;				// このボーン用のフレーム情報へのポインタ

		VECTOR				*KeyPos;				// 座標アニメーションキー配列の先頭
		FLOAT4				*KeyRot;				// 回転アニメーションキー配列の先頭
		float				*KeyPosTime;			// 座標アニメーションタイムキー配列の先頭
		float				*KeyRotTime;			// 回転アニメーションタイムキー配列の先頭
		MATRIX				*KeyMatrix;			// 行列アニメーションキー配列の先頭
		MATRIX				*KeyMatrix2;			// 行列アニメーションキー配列の先頭( IsIKChild が TRUE のボーン用 )

		int					Type;					// タイプ
		int					IsPhysics;				// 物理を使用しているかどうか
		int					IsIK;					// ＩＫボーンかどうか
		int					IsIKAnim;				// ＩＫアニメーションをつけるかどうか
		int					IsIKChild;				// ＩＫアニメーションするボーンの子ボーン( 且つＩＫの影響を受けないボーン )かどうか
		int					IKLimitAngle;			// ＩＫ時に角度制限をするかどうか
		MATRIX				LocalWorldMatrix;		// 構築したローカル→ワールド行列
		VECTOR				Translate;				// 平行移動値
		FLOAT4				Rotate;				// 回転値
		VECTOR				OrgTranslate;			// 原点からの初期位置

		MATRIX				IKmat;					// IK処理で使用する行列構造体
		FLOAT4				IKQuat;				// IK処理で使用するクォータニオン

		VECTOR				InitTranslate;			// 平行移動値の初期値
		FLOAT4				InitRotate;			// 回転値の初期値
#ifndef DX_NON_BULLET_PHYSICS
		int					PhysicsIndex;			// 物理演算用情報の番号
		int					SetupPhysicsAnim;		// ボーンアニメーションを付けたかどうか
#endif

		struct VMD_READ_NODE_INFO	*Node;			// ボーンと関連付けられているノード
		struct VMD_READ_KEY_INFO	*NowKey;		// 現在再生しているキー
	};

	// VMD読み込み処理用PMDIKボーンデータ
	struct PMD_READ_IK_INFO
	{
		PMD_IK				*Base;					// IKボーン基データ
		PMD_READ_BONE_INFO	*Bone;					// IKターゲットボーン
		PMD_READ_BONE_INFO	*TargetBone;			// IK先端ボーン

		uint16				LimitAngleIK;			// 角度制限ボーンが含まれたＩＫかどうか( 0:違う 1:そう ) 
		uint16				Iterations;			// 再起演算回数
		float				ControlWeight;			// IKの影響度
		float				IKTotalLength;			// IK処理の根元からチェインの最後までの距離

		int					ChainBoneNum;			// IKチェーンの長さ(子の数)
		uint16				*ChainBone;			// IK影響下のボーンへのインデックス配列へのポインタ

		PMD_READ_IK_INFO	*Prev;					// リストの前のデータへのアドレス
		PMD_READ_IK_INFO	*Next;					// リストの次のデータへのアドレス
	};
	//////////////////////////////////////////////////////////////

	struct PMX_BONE_HIERARCKY
	{

	};
#endif
    //////////////////////////////////////////////////////////////

    DECLARE_LOG_CATEGORY_EXTERN(LogMMD4UE5_PmdMeshInfo, Log, All)

    // Inport用 meta data 格納クラス
    class PmdMeshInfo : public MMDImportHelper
    {
    public:
        PmdMeshInfo();
        ~PmdMeshInfo();

        ///////////////////////////////////////
        bool PMDLoaderBinary(
            const uint8 *&Buffer,
            const uint8 *BufferEnd);
        //////////////////////////////////////////
        bool ConvertToPmxFormat(
            PmxMeshInfo *pmxImportPtr);
        //////////////////////////////////////////
        PMD_HEADER header;
        PMD_VERTEX_DATA vertexData;
        TArray<PMD_VERTEX> vertexList;
        PMD_FACE_DATA faceData;
        TArray<PMD_FACE> faceList;
        PMD_MATERIAL_DATA materialData;
        TArray<PMD_MATERIAL> materialList;

        PMD_BONE_DATA boneData;
        TArray<PMD_BONE> boneList;

        PMD_IK_DATA ikData;
        TArray<PMD_IK> ikList;

        PMD_SKIN_DATA skinData;
        TArray<PMD_SKIN> skinList;
    };

}