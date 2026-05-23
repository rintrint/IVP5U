// Copyright 2015-2026 IVP5U contributors

#include "CoreMinimal.h"

#include "Animation/MorphTarget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Developer/MeshUtilities/Public/MeshUtilities.h"
#include "Engine.h"
#include "Factories.h"
#include "ImportUtils/SkelImport.h"
#include "LODUtilities.h"
#include "Misc/FbxErrors.h"
#include "Rendering/SkeletalMeshModel.h"

#include "PmxFactory.h"

#define LOCTEXT_NAMESPACE "PMXSkeletalMeshImporter"
////////////////////////////////////////////////////////////////////////////////////////////////
// FMorphMeshRawSource is removed after version 4.16. So added for only this plugin here.
// Converts a mesh to raw vertex data used to generate a morph target mesh

/** compare based on base mesh source vertex indices */
struct FCompareMorphTargetDeltas
{
	FORCEINLINE bool operator()(const FMorphTargetDelta& A, const FMorphTargetDelta& B) const
	{
		return ((int32)A.SourceIdx - (int32)B.SourceIdx) < 0 ? true : false;
	}
};

class FMorphMeshRawSource
{
public:
	struct FMorphMeshVertexRaw
	{
		FVector3f Position;

		// Tangent, U-direction
		FVector3f TangentX;
		// Binormal, V-direction
		FVector3f TangentY;
		// Normal
		FVector4f TangentZ;
	};

	/** vertex data used for comparisons */
	TArray<FMorphMeshVertexRaw> Vertices;

	/** index buffer used for comparison */
	TArray<uint32> Indices;

	/** indices to original imported wedge points */
	TArray<uint32> WedgePointIndices;

	/** Constructor (default) */
	FMorphMeshRawSource() {}
	FMorphMeshRawSource(USkeletalMesh* SrcMesh, int32 LODIndex = 0);
	FMorphMeshRawSource(FSkeletalMeshLODModel& LODModel);

	static void CalculateMorphTargetLODModel(const FMorphMeshRawSource& BaseSource,
		const FMorphMeshRawSource& TargetSource, FMorphTargetLODModel& MorphModel);

private:
	void Initialize(FSkeletalMeshLODModel& LODModel);
};

/**
 * Constructor.
 * Converts a skeletal mesh to raw vertex data
 * needed for creating a morph target mesh
 *
 * @param	SrcMesh - source skeletal mesh to convert
 * @param	LODIndex - level of detail to use for the geometry
 */
FMorphMeshRawSource::FMorphMeshRawSource(USkeletalMesh* SrcMesh, int32 LODIndex)
{
	check(SrcMesh);
	check(SrcMesh->GetImportedModel());
	check(SrcMesh->GetImportedModel()->LODModels.IsValidIndex(LODIndex));

	// get the mesh data for the given lod
	FSkeletalMeshLODModel& LODModel = SrcMesh->GetImportedModel()->LODModels[LODIndex];

	Initialize(LODModel);
}

FMorphMeshRawSource::FMorphMeshRawSource(FSkeletalMeshLODModel& LODModel)
{
	Initialize(LODModel);
}

void FMorphMeshRawSource::Initialize(FSkeletalMeshLODModel& LODModel)
{
	// iterate over the chunks for the skeletal mesh
	for (int32 SectionIdx = 0; SectionIdx < LODModel.Sections.Num(); SectionIdx++)
	{
		const FSkelMeshSection& Section = LODModel.Sections[SectionIdx];
		for (int32 VertexIdx = 0; VertexIdx < Section.SoftVertices.Num(); VertexIdx++)
		{
			const FSoftSkinVertex& SourceVertex = Section.SoftVertices[VertexIdx];
			FMorphMeshVertexRaw RawVertex = {
				SourceVertex.Position,
				SourceVertex.TangentX,
				SourceVertex.TangentY,
				SourceVertex.TangentZ
			};
			Vertices.Add(RawVertex);
		}
	}

	// Copy the indices manually, since the LODModel's index buffer may have a different alignment.
	Indices.Empty(LODModel.IndexBuffer.Num());
	for (int32 Index = 0; Index < LODModel.IndexBuffer.Num(); Index++)
	{
		Indices.Add(LODModel.IndexBuffer[Index]);
	}

	// copy the wedge point indices
	int idxNum = LODModel.GetRawPointIndices().Num();
	if (idxNum)
	{
		WedgePointIndices.Empty(idxNum);
		WedgePointIndices.AddUninitialized(idxNum);
		FMemory::Memcpy(WedgePointIndices.GetData(), LODModel.GetRawPointIndices().GetData(), LODModel.GetRawPointIndices().Num() * LODModel.GetRawPointIndices().GetTypeSize());
		// LODModel.RawPointIndices.Unlock();
	}
}

void FMorphMeshRawSource::CalculateMorphTargetLODModel(const FMorphMeshRawSource& BaseSource,
	const FMorphMeshRawSource& TargetSource, FMorphTargetLODModel& MorphModel)
{
	// set the original number of vertices
	MorphModel.NumBaseMeshVerts = BaseSource.Vertices.Num();

	// empty morph mesh vertices first
	MorphModel.Vertices.Empty();

	// array to mark processed base vertices
	TArray<bool> WasProcessed;
	WasProcessed.Empty(BaseSource.Vertices.Num());
	WasProcessed.AddZeroed(BaseSource.Vertices.Num());

	TMap<uint32, uint32> WedgePointToVertexIndexMap;
	// Build a mapping of wedge point indices to vertex indices for fast lookup later.
	for (int32 Idx = 0; Idx < TargetSource.WedgePointIndices.Num(); Idx++)
	{
		WedgePointToVertexIndexMap.Add(TargetSource.WedgePointIndices[Idx], Idx);
	}

	// iterate over all the base mesh indices
	for (int32 Idx = 0; Idx < BaseSource.Indices.Num(); Idx++)
	{
		uint32 BaseVertIdx = BaseSource.Indices[Idx];

		// check for duplicate processing
		if (!WasProcessed[BaseVertIdx])
		{
			// mark this base vertex as already processed
			WasProcessed[BaseVertIdx] = true;

			// get base mesh vertex using its index buffer
			const FMorphMeshVertexRaw& VBase = BaseSource.Vertices[BaseVertIdx];

			// clothing can add extra verts, and we won't have source point, so we ignore those
			if (BaseSource.WedgePointIndices.IsValidIndex(BaseVertIdx))
			{
				// get the base mesh's original wedge point index
				uint32 BasePointIdx = BaseSource.WedgePointIndices[BaseVertIdx];

				// find the matching target vertex by searching for one
				// that has the same wedge point index
				uint32* TargetVertIdx = WedgePointToVertexIndexMap.Find(BasePointIdx);

				// only add the vertex if the source point was found
				if (TargetVertIdx != nullptr)
				{
					// get target mesh vertex using its index buffer
					const FMorphMeshVertexRaw& VTarget = TargetSource.Vertices[*TargetVertIdx];

					// change in position from base to target
					FVector3f PositionDelta(VTarget.Position - VBase.Position);
					FVector3f NormalDeltaZ(VTarget.TangentZ - VBase.TangentZ);

					// check if position actually changed much
					if (PositionDelta.SizeSquared() > FMath::Square(THRESH_POINTS_ARE_NEAR) ||
						// since we can't get imported morphtarget normal from FBX
						// we can't compare normal unless it's calculated
						// this is special flag to ignore normal diff
						(true && NormalDeltaZ.SizeSquared() > 0.01f))
					{
						// create a new entry
						FMorphTargetDelta NewVertex;
						// position delta
						NewVertex.PositionDelta = PositionDelta;
						// normal delta
						NewVertex.TangentZDelta = NormalDeltaZ;
						// index of base mesh vert this entry is to modify
						NewVertex.SourceIdx = BaseVertIdx;

						// add it to the list of changed verts
						MorphModel.Vertices.Add(NewVertex);
					}
				}
			}
		}
	}

	// sort the array of vertices for this morph target based on the base mesh indices
	// that each vertex is associated with. This allows us to sequentially traverse the list
	// when applying the morph blends to each vertex.
	MorphModel.Vertices.Sort(FCompareMorphTargetDeltas());

	// remove array slack
	MorphModel.Vertices.Shrink();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// UPmxFactory

bool UPmxFactory::ImportBone(
	MMD4UE5::PmxMeshInfo* PmxMeshInfo,
	FSkeletalMeshImportData& ImportData,
	bool& bOutDiffPose,
	bool bDisableMissingBindPoseWarning,
	bool& bUseTime0AsRefPose)
{
	int32 NumberOfRoot = 0;

	for (int LinkIndex = 0; LinkIndex < PmxMeshInfo->boneList.Num(); LinkIndex++)
	{
		// Add a bone for each FBX Link
		ImportData.RefBonesBinary.Add(SkeletalMeshImportData::FBone());

		// PMX format guarantees parent bone index < child bone index,
		// so ParentBoneIndex can be used directly. INDEX_NONE means root.
		const int32 LinkParent = PmxMeshInfo->boneList[LinkIndex].ParentBoneIndex;
		const int32 ParentIndex = (LinkParent >= 0 && LinkParent < LinkIndex) ? LinkParent : INDEX_NONE;

		// see how many root this has
		// if more than
		if (ParentIndex == INDEX_NONE)
		{
			++NumberOfRoot;
			if (NumberOfRoot > 1)
			{
				AddTokenizedErrorMessage(
					FTokenizedMessage::Create(
						EMessageSeverity::Error,
						LOCTEXT("MultipleRootsFound", "Multiple roots are found in the bone hierarchy. We only support single root bone.")),
					FFbxErrors::SkeletalMesh_MultipleRoots);
				return false;
			}
		}

		// set bone
		SkeletalMeshImportData::FBone& Bone = ImportData.RefBonesBinary[LinkIndex];
		FString BoneName;

		BoneName = PmxMeshInfo->boneList[LinkIndex].Name;
		Bone.Name = BoneName;

		SkeletalMeshImportData::FJointPos& JointMatrix = Bone.BonePos;

		JointMatrix.Length = 1.;
		JointMatrix.XSize = 100.;
		JointMatrix.YSize = 100.;
		JointMatrix.ZSize = 100.;

		// get the link parent and children
		Bone.ParentIndex = ParentIndex;
		Bone.NumChildren = 0;

		// For MMD
		FQuat4f ftr = FQuat4f(0, 0, 0, 1.0);
		for (int32 ChildIndex = 0; ChildIndex < PmxMeshInfo->boneList.Num(); ChildIndex++)
		{
			if (LinkIndex == PmxMeshInfo->boneList[ChildIndex].ParentBoneIndex)
			{
				Bone.NumChildren++;
			}
		}

		// test MMD , not rot axis and LocalAxis
		FVector3f TransTemp = PmxMeshInfo->boneList[LinkIndex].Position;
		bool hasParent = ParentIndex != INDEX_NONE;
		if (hasParent)
		{
			FTransform3f it = PmxMeshInfo->boneList[ParentIndex].absTF.Inverse();
			TransTemp = it.TransformPosition(
							PmxMeshInfo->boneList[LinkIndex].Position)
				- it.TransformPosition(PmxMeshInfo->boneList[ParentIndex].Position)

				;
			// TransTemp *= -1;
		}

		ftr = FRotator3f(0, 0, 0).Quaternion();
		JointMatrix.Transform.SetTranslation(TransTemp);
		JointMatrix.Transform.SetRotation(ftr);
		JointMatrix.Transform.SetScale3D(FVector3f(1));
		if (hasParent)
			PmxMeshInfo->boneList[LinkIndex].absTF = PmxMeshInfo->boneList[ParentIndex].absTF * JointMatrix.Transform;
		else
			PmxMeshInfo->boneList[LinkIndex].absTF = JointMatrix.Transform;
	}

	return true;
}

bool UPmxFactory::FillSkelMeshImporterFromFbx(
	FSkeletalMeshImportData& ImportData,
	MMD4UE5::PmxMeshInfo*& PmxMeshInfo,
	UObject* InParent)
{
	TArray<UMaterialInterface*> Materials;

#if 1 // test Material Textuere
	TArray<UTexture*> textureAssetList;
	for (int k = 0; k < PmxMeshInfo->textureList.Num(); ++k)
	{
		pmxMaterialImportHelper.AssetsCreateTexture(
			InParent,
			FPaths::GetPath(GetCurrentFilename()),
			PmxMeshInfo->textureList[k].TexturePath,
			textureAssetList);
	}

	TArray<FString> UVSets;
	for (int k = 0; k < PmxMeshInfo->materialList.Num(); ++k)
	{
		pmxMaterialImportHelper.CreateUnrealMaterial(
			PmxMeshInfo->modelNameJP,
			PmxMeshInfo->materialList[k],
			ImportUI->bCreateMaterialInstanceMode,
			ImportUI->bUnlitMaterials,
			Materials,
			textureAssetList);
		{
			int ExistingMatIndex = k;
			int MaterialIndex = k;

			// material asset set flag for morph target
			if (UMaterialInterface* UnrealMaterialPtr = Materials[MaterialIndex])
			{
				UnrealMaterialPtr->CheckMaterialUsage(MATUSAGE_MorphTargets);
			}

			ImportData.Materials[ExistingMatIndex].MaterialImportName = //"M_" +
				PmxMeshInfo->materialList[k].Name;
			ImportData.Materials[ExistingMatIndex].Material = Materials[MaterialIndex];
		}
	}
#endif

	//	store the UVs in arrays for fast access in the later looping of triangles
	uint32 UniqueUVCount = UVSets.Num();

	UniqueUVCount = FMath::Min<uint32>(UniqueUVCount, MAX_TEXCOORDS);
	// One UV set is required but only import up to MAX_TEXCOORDS number of uv layers
	ImportData.NumTexCoords = FMath::Max<uint32>(ImportData.NumTexCoords, UniqueUVCount);

	ImportData.bHasNormals = true;
	ImportData.bHasTangents = false;

	// create the points / wedges / faces
	int32 ControlPointsCount =
		PmxMeshInfo->vertexList.Num();
	// Mesh->GetControlPointsCount();
	int32 ExistPointNum = ImportData.Points.Num();
	ImportData.Points.AddUninitialized(ControlPointsCount);

	int32 ControlPointsIndex;
	for (ControlPointsIndex = 0; ControlPointsIndex < ControlPointsCount; ControlPointsIndex++)
	{
		ImportData.Points[ControlPointsIndex + ExistPointNum] = PmxMeshInfo->vertexList[ControlPointsIndex].Position;
	}
#if 1							  // vertex
	bool OddNegativeScale = true; // IsOddNegativeScale(TotalMatrix);

	int32 VertexIndex;
	int32 TriangleCount = PmxMeshInfo->faceList.Num(); // Mesh->GetPolygonCount();
	int32 ExistFaceNum = ImportData.Faces.Num();
	ImportData.Faces.AddUninitialized(TriangleCount);
	SkeletalMeshImportData::FVertex TmpWedges[3];

	int32 facecount = 0;
	int32 matIndx = 0;

	for (int32 TriangleIndex = ExistFaceNum, LocalIndex = 0; TriangleIndex < ExistFaceNum + TriangleCount; TriangleIndex++, LocalIndex++)
	{
		SkeletalMeshImportData::FTriangle& Triangle = ImportData.Faces[TriangleIndex];

		//
		// smoothing mask
		//
		// set the face smoothing by default. It could be any number, but not zero
		Triangle.SmoothingGroups = 255;

		for (VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			const int32 UnrealVertexIndex = OddNegativeScale ? 2 - VertexIndex : VertexIndex;
			const FVector3f TangentZ = PmxMeshInfo->vertexList[PmxMeshInfo->faceList[LocalIndex].VertexIndex[VertexIndex]].Normal;

			Triangle.TangentX[UnrealVertexIndex] = FVector3f::ZeroVector;
			Triangle.TangentY[UnrealVertexIndex] = FVector3f::ZeroVector;
			Triangle.TangentZ[UnrealVertexIndex] = TangentZ.GetSafeNormal();
		}

		// material index
		Triangle.MatIndex = 0; // default value

		{
			// for mmd

			if (PmxMeshInfo->materialList.Num() > matIndx)
			{
				facecount++;
				facecount++;
				facecount++;
				Triangle.MatIndex = static_cast<uint16>(matIndx);
				if (facecount >= PmxMeshInfo->materialList[matIndx].MaterialFaceNum)
				{
					matIndx++;
					facecount = 0;
				}
			}
		}

		Triangle.AuxMatIndex = 0;
		for (VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			// If there are odd number negative scale, invert the vertex order for triangles
			const int32 UnrealVertexIndex = OddNegativeScale ? 2 - VertexIndex : VertexIndex;

			TmpWedges[UnrealVertexIndex].MatIndex = static_cast<uint8>(Triangle.MatIndex);
			TmpWedges[UnrealVertexIndex].VertexIndex = PmxMeshInfo->faceList[LocalIndex].VertexIndex[VertexIndex];
			// = ExistPointNum + Mesh->GetPolygonVertex(LocalIndex, VertexIndex);
			// Initialize all colors to white.
			TmpWedges[UnrealVertexIndex].Color = FColor::White;
		}

		// uvs
		uint32 UVLayerIndex;
		// Some FBX meshes can have no UV sets, so also check the UniqueUVCount
		for (UVLayerIndex = 0; UVLayerIndex < UniqueUVCount; UVLayerIndex++)
		{
			// Set all UV's to zero.  If we are here the mesh had no UV sets so we only need to do this for the
			// first UV set which always exists.
			TmpWedges[VertexIndex].UVs[UVLayerIndex].X = 0;
			TmpWedges[VertexIndex].UVs[UVLayerIndex].Y = 0;
		}

		// basic wedges matching : 3 unique per face. TODO Can we do better ?
		for (VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			int32 w;

			w = ImportData.Wedges.AddUninitialized();
			ImportData.Wedges[w].VertexIndex = TmpWedges[VertexIndex].VertexIndex;
			ImportData.Wedges[w].MatIndex = TmpWedges[VertexIndex].MatIndex;
			ImportData.Wedges[w].Color = TmpWedges[VertexIndex].Color;
			ImportData.Wedges[w].Reserved = 0;

			FVector2f tempUV = PmxMeshInfo->vertexList[TmpWedges[VertexIndex].VertexIndex].UV;
			TmpWedges[VertexIndex].UVs[0].X = tempUV.X;
			TmpWedges[VertexIndex].UVs[0].Y = tempUV.Y;
			FMemory::Memcpy(ImportData.Wedges[w].UVs,
				TmpWedges[VertexIndex].UVs,
				sizeof(FVector2f) * MAX_TEXCOORDS);

			Triangle.WedgeIndex[VertexIndex] = w;
		}
	}
#endif
	// now we can work on a per-cluster basis with good ordering

	// For mmd. skining
	if (PmxMeshInfo->boneList.Num() > 0)
	{
		// create influences for each cluster
		//	for each vertex index in the cluster
		for (int32 ControlPointIndex = 0;
			ControlPointIndex < PmxMeshInfo->vertexList.Num();
			++ControlPointIndex)
		{
			int32 multiBone = 0;
			switch (PmxMeshInfo->vertexList[ControlPointIndex].WeightType)
			{
				case 0: // 0:BDEF1
				{
					ImportData.Influences.AddUninitialized();
					ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[0];
					ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[0];
					ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
				}
				break;
				case 1: // 1:BDEF2
				{
					for (multiBone = 0; multiBone < 2; ++multiBone)
					{
						ImportData.Influences.AddUninitialized();
						ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[multiBone];
						ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[multiBone];
						ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
					}
				}
				break;
				case 2: // 2:BDEF4
				{
					for (multiBone = 0; multiBone < 4; ++multiBone)
					{
						ImportData.Influences.AddUninitialized();
						ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[multiBone];
						ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[multiBone];
						ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
					}
				}
				break;
				case 3: // 3:SDEF
				{
					// 限制：SDEF
					// 将SDEF转换为BDEF2进行处理。
					// 这是SDEF_C、SDEF_R0、SDEF_由于不知道如何设置R1的参数。
					// 根据在另一PF（ex.MMD4Mecanim或Dxlib）中的安装例进行分析及信息收集
					// 并保留，直到找到满足MMD SDEF操作规范的方法
					// 关于SDEF的规格（MMD）请参考以下页面。
					// Ref:：各软件的SDEF变形差异-FC2
					//  http://mikudan.blog120.fc2.com/blog-entry-339.html

					for (multiBone = 0; multiBone < 2; ++multiBone)
					{
						ImportData.Influences.AddUninitialized();
						ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[multiBone];
						ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[multiBone];
						ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
					}
				}
				break;
#if 0 // for pmx ver 2.1 formnat
			case 4:
				// 制限事項：QDEF
				// QDEFに関して、MMDでの仕様を調べる事。
			{
				for (multiBone = 0; multiBone < 4; ++multiBone)
				{
					ImportData.Influences.AddUninitialized();
					ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[multiBone];
					ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[multiBone];
					ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
				}
			}
			break;
#endif
				default:
				{
					// 異常系
					// 0:BDEF1 形式と同じ手法で暫定対応する
					ImportData.Influences.AddUninitialized();
					ImportData.Influences.Last().BoneIndex = PmxMeshInfo->vertexList[ControlPointIndex].BoneIndex[0];
					ImportData.Influences.Last().Weight = PmxMeshInfo->vertexList[ControlPointIndex].BoneWeight[0];
					ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
					UE_LOG(LogMMD4UE5_PMXFactory, Error,
						TEXT("Unkown Weight Type :: type = %d , vertex index = %d "),
						PmxMeshInfo->vertexList[ControlPointIndex].WeightType, ControlPointIndex);
				}
				break;
			}
		}
	}
	else // for rigid mesh — no bones, bind all vertices to bone 0
	{
		constexpr int32 BoneIndex = 0;
		for (int32 ControlPointIndex = 0; ControlPointIndex < ControlPointsCount; ++ControlPointIndex)
		{
			ImportData.Influences.AddUninitialized();
			ImportData.Influences.Last().BoneIndex = BoneIndex;
			ImportData.Influences.Last().Weight = 1.0;
			ImportData.Influences.Last().VertexIndex = ExistPointNum + ControlPointIndex;
		}
	}

	return true;
}

UObject* UPmxFactory::CreateAssetOfClass(
	UClass* AssetClass,
	FString ParentPackageName,
	FString ObjectName,
	bool bAllowReplace)
{
	// See if this sequence already exists.
	UObject* ParentPkg = CreatePackage(*ParentPackageName);
	FString ParentPath = FString::Printf(
		TEXT("%s/%s"),
		*FPackageName::GetLongPackagePath(*ParentPackageName),
		*ObjectName);
	UObject* Parent = CreatePackage(*ParentPath);
	// See if an object with this name exists
	UObject* Object = LoadObject<UObject>(Parent, *ObjectName, nullptr, LOAD_NoWarn | LOAD_Quiet);

	// if object with same name but different class exists, warn user
	if ((Object != nullptr) && (Object->GetClass() != AssetClass))
	{
		return nullptr;
	}

	if (Object == nullptr)
	{
		// add it to the set
		// do not add to the set, now create independent asset
		Object = NewObject<UObject>(Parent, AssetClass, *ObjectName, RF_Public | RF_Standalone);
		Object->MarkPackageDirty();
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(Object);
	}

	return Object;
}

void UPmxFactory::ImportMorphTargetsInternal(
	MMD4UE5::PmxMeshInfo& PmxMeshInfo,
	USkeletalMesh* BaseSkelMesh,
	UObject* InParent,
	const FString& InFilename,
	int32 LODIndex, FSkeletalMeshImportData& BaseImportData)
{
	TMap<FString, MMD4UE5::PMX_MORPH> ShapeNameToShapeArray;

	for (int32 NodeIndex = 0; NodeIndex < PmxMeshInfo.morphList.Num(); NodeIndex++)
	{
		MMD4UE5::PMX_MORPH* pmxMorphPtr = &(PmxMeshInfo.morphList[NodeIndex]);
		if (pmxMorphPtr->Type == 1 && pmxMorphPtr->Vertex.Num() > 0)
		{ // 頂点Morph
			FString ShapeName = pmxMorphPtr->Name;
			MMD4UE5::PMX_MORPH& ShapeArray = ShapeNameToShapeArray.FindOrAdd(ShapeName);
			ShapeArray = *pmxMorphPtr;
		}
	}
	bool bImportOperationCanceled = false;
	for (auto Iter = ShapeNameToShapeArray.CreateIterator(); Iter && !bImportOperationCanceled; ++Iter)
	{
		FString ShapeName = Iter.Key();
		MMD4UE5::PMX_MORPH& ShapeArray = Iter.Value();
		FSkeletalMeshImportData ShapeImportData;
		BaseImportData.CopyDataNeedByMorphTargetImport(ShapeImportData);

		// Store the rebuild morph data into the base import data, this will allow us to rebuild the morph data in case the mesh is rebuild and the vertex count change because of options (max bone per section, normals compute...)
		int32 MorphTargetIndex;
		if (BaseImportData.MorphTargetNames.Find(ShapeName, MorphTargetIndex))
		{
			BaseImportData.MorphTargetNames.RemoveAt(MorphTargetIndex);
			BaseImportData.MorphTargetModifiedPoints.RemoveAt(MorphTargetIndex);
			BaseImportData.MorphTargets.RemoveAt(MorphTargetIndex);
		}
		BaseImportData.MorphTargetNames.Add(ShapeName);
		TSet<uint32>& ModifiedPoints = BaseImportData.MorphTargetModifiedPoints.AddDefaulted_GetRef();

		TArray<FVector3f> CompressPoints;
		CompressPoints.Reserve(ShapeArray.Vertex.Num());

		for (int i = 0; i < ShapeArray.Vertex.Num(); i++)
		{
			MMD4UE5::PMX_MORPH_VERTEX tempMorphVertex = ShapeArray.Vertex[i];
			ModifiedPoints.Add(tempMorphVertex.Index);
			ShapeImportData.Points[tempMorphVertex.Index] += tempMorphVertex.Offset;
			CompressPoints.Add(ShapeImportData.Points[tempMorphVertex.Index]);
		}

		ShapeImportData.Points = CompressPoints;
		// GatherPointsForMorphTarget(&ShapeImportData, SkelMeshNodeArray, &ShapeArray, ModifiedPoints);
		// We do not need this data anymore empty it so we reduce the size of what we save into memory
		ShapeImportData.PointToRawMap.Empty();
		BaseImportData.MorphTargets.Add(ShapeImportData);
		if (!ensureMsgf(
				BaseImportData.MorphTargetNames.Num() == BaseImportData.MorphTargets.Num()
					&& BaseImportData.MorphTargetNames.Num() == BaseImportData.MorphTargetModifiedPoints.Num(),
				TEXT("MorphTarget arrays out of sync: Names=%d Targets=%d ModifiedPoints=%d"),
				BaseImportData.MorphTargetNames.Num(),
				BaseImportData.MorphTargets.Num(),
				BaseImportData.MorphTargetModifiedPoints.Num()))
		{
			const int32 SafeNum = FMath::Min3(
				BaseImportData.MorphTargetNames.Num(),
				BaseImportData.MorphTargets.Num(),
				BaseImportData.MorphTargetModifiedPoints.Num());
			BaseImportData.MorphTargetNames.SetNum(SafeNum);
			BaseImportData.MorphTargets.SetNum(SafeNum);
			BaseImportData.MorphTargetModifiedPoints.SetNum(SafeNum);
			UE_LOG(LogMMD4UE5_PMXFactory, Warning,
				TEXT("MorphTarget arrays truncated to SafeNum=%d to recover sync."), SafeNum);
		}
	}

	if (BaseSkelMesh->GetImportedModel() && BaseSkelMesh->GetImportedModel()->LODModels.IsValidIndex(LODIndex))
	{
		// If we can build the skeletal mesh there is no need to build the morph target now, all the necessary build morph target data was copied before.
		if (!BaseSkelMesh->HasMeshDescription(LODIndex))
		{
			FOverlappingThresholds hd;
			// Build MorphTargets
			FLODUtilities::BuildMorphTargets(
				BaseSkelMesh,	// USkeletalMesh*
				BaseImportData, // FSkeletalMeshImportData&
				LODIndex,		// int32 LODIndex
				true,			// bool ShouldImportNormals
				true,			// bool ShouldImportTangents
				true,			// bool bUseMikkTSpace
				hd);			// const FOverlappingThresholds&
		}
		// 確保 Skeleton 有對應的 Curves
		if (BaseSkelMesh->GetSkeleton())
		{
			USkeleton* Skeleton = BaseSkelMesh->GetSkeleton();

			// 清理舊的 morph curves（重新導入時避免殘留）
			TArray<FName> ExistingCurveNames;
			Skeleton->GetCurveMetaDataNames(ExistingCurveNames);
			for (const FName& CurveName : ExistingCurveNames)
			{
				FCurveMetaData* CurveMetaData = Skeleton->GetCurveMetaData(CurveName);
				if (CurveMetaData && CurveMetaData->Type.bMorphtarget)
				{
					Skeleton->RemoveCurveMetaData(CurveName);
				}
			}

			// 為每個成功導入的 morph target 確保有對應的 curve
			for (const FString& MorphName : BaseImportData.MorphTargetNames)
			{
				FCurveMetaData* FoundCurveMetaData = Skeleton->GetCurveMetaData(*MorphName);
				if (FoundCurveMetaData)
				{
					FoundCurveMetaData->Type.bMorphtarget = true;
					continue;
				}
				Skeleton->AccumulateCurveMetaData(*MorphName, false, true);
			}

			Skeleton->MarkPackageDirty();
		}
	}
}

/**
 * Import Morph Targets for the skeletal mesh
 */
void UPmxFactory::ImportFbxMorphTarget(
	MMD4UE5::PmxMeshInfo& PmxMeshInfo,
	USkeletalMesh* BaseSkelMesh,
	UObject* InParent,
	const FString& Filename,
	int32 LODIndex,
	FSkeletalMeshImportData& ImportData)
{
	if (PmxMeshInfo.morphList.Num() > 0)
	{
		ImportMorphTargetsInternal(
			PmxMeshInfo,
			BaseSkelMesh,
			InParent,
			Filename,
			LODIndex,
			ImportData);
	}
}

void UPmxFactory::AddTokenizedErrorMessage(
	TSharedRef<FTokenizedMessage> ErrorMsg,
	FName FbxErrorName)
{
	// if not found, use normal log
	switch (ErrorMsg->GetSeverity())
	{
		case EMessageSeverity::Error:
			UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("%d_%s"), __LINE__, *(ErrorMsg->ToText().ToString()));
			break;
		// case EMessageSeverity::CriticalError:
		// 	UE_LOG(LogMMD4UE5_PMXFactory, Error, TEXT("%d_%s"), __LINE__, *(ErrorMsg->ToText().ToString()));
		// 	break;
		case EMessageSeverity::Warning:
			UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("%d_%s"), __LINE__, *(ErrorMsg->ToText().ToString()));
			break;
		default:
			UE_LOG(LogMMD4UE5_PMXFactory, Warning, TEXT("%d_%s"), __LINE__, *(ErrorMsg->ToText().ToString()));
			break;
	}
}

#undef LOCTEXT_NAMESPACE
