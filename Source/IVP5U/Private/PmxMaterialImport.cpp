// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "PmxMaterialImport.h"
#include "IVP5UPrivatePCH.h"

#include "CoreMinimal.h"
#include "ComponentReregisterContext.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

#include "ContentBrowserModule.h"
#include "Editor/ContentBrowser/Public/IContentBrowserSingleton.h"

#include "PackageTools.h"

#include "ObjectTools.h"

#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionConstant.h"

DEFINE_LOG_CATEGORY(LogCategoryPMXMaterialImport)

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 2

	#define StaticSwitchParameters StaticSwitchParameters_DEPRECATED

#endif

namespace
{
	FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
} // namespace

void UPmxMaterialImport::AssetsCreateTexture(
	UObject* InParentx,
	FString CurPath,
	FString filePath,
	TArray<UTexture*>& textureAssetList)
{
	TArray<FString> ImagePaths;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	ImagePaths.AddUnique(filePath);

	// texture
	for (int i = 0; i < ImagePaths.Num(); ++i)
	{
		// TODO: 考虑ImageBaseDirectory
		FString FileName = CurPath / ImagePaths[i];

		UTexture* ImportedTexture = NULL;
		if (ExistImages.Contains(ImagePaths[i]))
		{
			ImportedTexture = ExistImages.FindChecked(ImagePaths[i]);
			continue;
		}

		FString TextureName = ("T_" + FPaths::GetBaseFilename(ImagePaths[i]));

		FString TexturePackageName;
		FString BasePackageName;
		if (InParentx)
		{
			BasePackageName = FPackageName::GetLongPackagePath(InParentx->GetOutermost()->GetName()) / TextureName;
			InParent = InParentx;
		}
		else
		{
			BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / TextureName;
		}

		FString BasePackageName_2 = PackageTools::SanitizePackageName(BasePackageName);

		UTexture* ExistingTexture = NULL;
		// First check if the asset already exists.
		{
			FString ObjectPath = BasePackageName_2 + TEXT(".") + TextureName;
			ExistingTexture = LoadObject<UTexture>(NULL, *ObjectPath);
		}
		if (!ExistingTexture)
		{
			// 重复创建
		}
		else
		{
			textureAssetList.Add(ExistingTexture);
			continue;
		}

		TArray<uint8> Data;
		if (FFileHelper::LoadFileToArray(Data, *FileName))
		{
			// From fbx texture import code
			auto TextureFact = NewObject<UTextureFactory>();
			TextureFact->AddToRoot();
			// save texture settings if texture exist
			TextureFact->SuppressImportOverwriteDialog();
			UPackage* TexturePackage = NULL;
			if (ImportedTexture)
			{
				TexturePackage = ImportedTexture->GetOutermost();
			}
			else
			{
				AssetToolsModule.Get().CreateUniqueAssetName(
					BasePackageName, TEXT(""),
					TexturePackageName, TextureName);
				TexturePackage = CreatePackage(*TexturePackageName);
			}

			const uint8* BufferBegin = Data.GetData();
			const uint8* BufferEnd = BufferBegin + Data.Num();
			UTexture2D* NewTexture = (UTexture2D*)TextureFact->FactoryCreateBinary(
				UTexture2D::StaticClass(),
				TexturePackage,
				FName(*TextureName),
				RF_Standalone | RF_Public,
				NULL,
				*FPaths::GetExtension(ImagePaths[i]),
				BufferBegin,
				BufferEnd,
				GWarn);

			if (NewTexture)
			{
				if (!ImportedTexture)
				{
					NewTexture->MipGenSettings = TMGS_NoMipmaps;
					NewTexture->AddressX = TA_Clamp;
					NewTexture->AddressY = TA_Clamp;
					NewTexture->CompressionSettings = TC_Default;
					NewTexture->LODGroup = TEXTUREGROUP_World;
					// Test
					NewTexture->SRGB = 1;
				}

				FAssetRegistryModule::AssetCreated(NewTexture);
				// TexturePackage->SetDirtyFlag(true);

				// Updating Texture & mark it as unsaved
				NewTexture->AddToRoot();
				NewTexture->UpdateResource();
				TexturePackage->MarkPackageDirty();

				ImportedTexture = NewTexture;

				// TexturePackage->SetPackageFlags(PKG_FilterEditorOnly);
				int w = NewTexture->GetSizeX();
				int h = NewTexture->GetSizeY();

				UE_LOG(LogCategoryPMXMaterialImport, Warning,
					TEXT("PMX Import NewTexture Complete.[%s]{%d,%d}"), *TextureName, w, h);
			}

			ExistImages.Add(ImagePaths[i], ImportedTexture);

			TextureFact->RemoveFromRoot();
			/**/
		}

		textureAssetList.Add(ImportedTexture);
	}
}

//-----
UTexture* UPmxMaterialImport::ImportTexture(
	// FbxFileTexture* FbxTexture,
	FString InTextureFileName,
	bool bSetupAsNormalMap)
{
	// create an unreal texture asset
	UTexture* UnrealTexture = NULL;
	FString Filename1 = InTextureFileName; // ANSI_TO_TCHAR(FbxTexture->GetFileName());
	FString Extension = FPaths::GetExtension(Filename1).ToLower();
	// name the texture with file name
	FString TextureName = FPaths::GetBaseFilename(Filename1);

	TextureName = ObjectTools::SanitizeObjectName(TextureName);

	// set where to place the textures
	FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / TextureName;
	BasePackageName = PackageTools::SanitizePackageName(BasePackageName);

	UTexture* ExistingTexture = NULL;
	UPackage* TexturePackage = NULL;
	// First check if the asset already exists.
	{
		FString ObjectPath = BasePackageName + TEXT(".") + TextureName;
		ExistingTexture = LoadObject<UTexture>(NULL, *ObjectPath);
	}

	if (!ExistingTexture)
	{
		const FString Suffix(TEXT(""));

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FString FinalPackageName;
		AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, TextureName);

		TexturePackage = CreatePackage(*FinalPackageName);
	}
	else
	{
		TexturePackage = ExistingTexture->GetOutermost();

		return ExistingTexture;
	}
	TexturePackage->SetPackageFlags(PKG_FilterEditorOnly);

	// try opening from absolute path
	FString Filename = Filename1;
	TArray<uint8> DataBinary;
	if (!FFileHelper::LoadFileToArray(DataBinary, *Filename))
	{
#if 0 // test
	  //  try fbx file base path + relative path
		FString Filename2 = FileBasePath / ANSI_TO_TCHAR(FbxTexture->GetRelativeFileName());
		Filename = Filename2;
		if (!FFileHelper::LoadFileToArray(DataBinary, *Filename))
		{
			// try fbx file base path + texture file name (no path)
			FString Filename3 = ANSI_TO_TCHAR(FbxTexture->GetRelativeFileName());
			FString FileOnly = FPaths::GetCleanFilename(Filename3);
			Filename3 = FileBasePath / FileOnly;
			Filename = Filename3;
			if (!FFileHelper::LoadFileToArray(DataBinary, *Filename))
			{
				UE_LOG(LogFbxMaterialImport, Warning, TEXT("Unable to find TEXTure file %s. Tried:\n - %s\n - %s\n - %s"), *FileOnly, *Filename1, *Filename2, *Filename3);
			}
		}
#endif
	}
	if (DataBinary.Num() > 0)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Log,
			TEXT("Loading texture file %s"), *Filename);
		const uint8* PtrTexture = DataBinary.GetData();
		auto TextureFact = NewObject<UTextureFactory>();
		TextureFact->AddToRoot();

		// save texture settings if texture exist
		TextureFact->SuppressImportOverwriteDialog();
		const TCHAR* TextureType = *Extension;

		// Unless the normal map setting is used during import,
		//	the user has to manually hit "reimport" then "recompress now" button
		if (bSetupAsNormalMap)
		{
			if (!ExistingTexture)
			{
				TextureFact->LODGroup = TEXTUREGROUP_WorldNormalMap;
				TextureFact->CompressionSettings = TC_Normalmap;
			}
			else
			{
				UE_LOG(LogCategoryPMXMaterialImport, Warning,
					TEXT("Manual texture reimport and recompression may be needed for %s"), *TextureName);
			}
		}

		UnrealTexture = (UTexture*)TextureFact->FactoryCreateBinary(
			UTexture2D::StaticClass(), TexturePackage, *TextureName,
			RF_Standalone | RF_Public, NULL, TextureType,
			PtrTexture, PtrTexture + DataBinary.Num(), GWarn);

		if (UnrealTexture != NULL)
		{
			// Notify the asset registry
			FAssetRegistryModule::AssetCreated(UnrealTexture);

			// Set the dirty flag so this package will get saved later
			TexturePackage->SetDirtyFlag(true);
		}
		TextureFact->RemoveFromRoot();
	}

	return UnrealTexture;
}

//--------------------------------------------------------------------
//
//-------------------------------------------------------------------------
bool UPmxMaterialImport::CreateAndLinkExpressionForMaterialProperty(
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	UMaterial* UnrealMaterial,
	const char* MaterialProperty,
	FExpressionInput& MaterialInput,
	bool bSetupAsNormalMap,
	const FVector2D& Location,
	TArray<UTexture*>& textureAssetList)
{
	bool bCreated = false;

#if 1
	int32 TextureCount = PmxMaterial.TextureIndex; // FbxProperty.GetSrcObjectCount<FbxTexture>();
	if (TextureCount >= 0 && TextureCount < textureAssetList.Num())
	{
		// for (int32 TextureIndex = 0; TextureIndex<TextureCount; ++TextureIndex)
		{
			// FbxFileTexture* FbxTexture = FbxProperty.GetSrcObject<FbxFileTexture>(TextureIndex);

			// create an unreal texture asset
			UTexture* UnrealTexture = textureAssetList[TextureCount]; // ImportTexture(FbxTexture, bSetupAsNormalMap);

			if (UnrealTexture)
			{
				UnrealMaterial->BlendMode = BLEND_Masked;
				// float ScaleU = FbxTexture->GetScaleU();
				// float ScaleV = FbxTexture->GetScaleV();

				// Multipule
				UMaterialExpressionMultiply* MulExpression = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression);
	#else
				UnrealMaterial->Expressions.Add(MulExpression);
	#endif

				// UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression);
				// UnrealMaterial->BaseColor.Expression = MulExpression;
				MulExpression->MaterialExpressionEditorX = -250;
				MulExpression->MaterialExpressionEditorY = 0;
				MulExpression->bHidePreviewWindow = 0;

				MulExpression->Desc = TEXT("Textuer * Texture alpha -> BaseColor");

				// Multipule
				UMaterialExpressionMultiply* MulExpression_2 = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression_2);
				UnrealMaterial->GetEditorOnlyData()->BaseColor.Expression = MulExpression_2;
	#else
				UnrealMaterial->Expressions.Add(MulExpression_2);
				UnrealMaterial->BaseColor.Expression = MulExpression_2;
	#endif

				MulExpression_2->B.Expression = MulExpression;
				MulExpression_2->MaterialExpressionEditorX = -250;
				MulExpression_2->MaterialExpressionEditorY = 200;
				MulExpression_2->bHidePreviewWindow = 0;

				MulExpression_2->Desc = TEXT("Textuer alpha * Specure Coloer -> OpacityMask");
				// MulExpression->ConstA = 1.0f;
				// MulExpression->ConstB = FresnelBaseReflectFraction_DEPRECATED;

				// MulExpression->A.Connect(SpecularColor_DEPRECATED.OutputIndex, SpecularColor_DEPRECATED.Expression);
				// SpecularColor_DEPRECATED.Connect(0, MulExpression);

				// A
				// and link it to the material
				UMaterialExpressionTextureSample* UnrealTextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(UnrealTextureExpression);
	#else
				UnrealMaterial->Expressions.Add(UnrealTextureExpression); // #tag
	#endif

				// MaterialInput.Expression = UnrealTextureExpression;
				MulExpression->A.Expression = UnrealTextureExpression;
				MulExpression->B.Connect(4, UnrealTextureExpression);
				// MulExpression_2->B.Connect(4, UnrealTextureExpression);
				// MulExpression->B.Expression = UnrealTextureExpression.Outputs[4];

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->OpacityMask.Connect(4, UnrealTextureExpression);
	#else
				UnrealMaterial->OpacityMask.Connect(4, UnrealTextureExpression);
	#endif

				UnrealTextureExpression->Texture = UnrealTexture;
				UnrealTextureExpression->SamplerType = bSetupAsNormalMap ? SAMPLERTYPE_Normal : SAMPLERTYPE_Color;
				UnrealTextureExpression->MaterialExpressionEditorX = -500; // FMath::TruncToInt(Location.X);
				UnrealTextureExpression->MaterialExpressionEditorY = 0;	   // FMath::TruncToInt(Location.Y);
				UnrealTextureExpression->SamplerSource = SSM_Wrap_WorldGroupSettings;
				// For minus UV asix MMD(e.g. AnjeraBalz///)

				// MulExpression->B.Connect(UnrealTextureExpression->Outputs[4].Expression);

				// B
				UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MyColorExpression);
	#else
				UnrealMaterial->Expressions.Add(MyColorExpression);
	#endif

				// UnrealMaterial->BaseColor.Expression = MyColorExpression;
				// MulExpression->B.Expression = MyColorExpression;
				MulExpression_2->A.Expression = MyColorExpression;

				MyColorExpression->DefaultValue.R = PmxMaterial.Diffuse[0];
				MyColorExpression->DefaultValue.G = PmxMaterial.Diffuse[1];
				MyColorExpression->DefaultValue.B = PmxMaterial.Diffuse[2];
				MyColorExpression->DefaultValue.A = PmxMaterial.Diffuse[3]; // A
				MyColorExpression->MaterialExpressionEditorX = -500;
				MyColorExpression->MaterialExpressionEditorY = 300;
				MyColorExpression->SetEditableName("DiffuseColor");
				/*
				// add/find UVSet and set it to the texture
				FbxString UVSetName = FbxTexture->UVSet.Get();
				FString LocalUVSetName = ANSI_TO_TCHAR(UVSetName.Buffer());
				int32 SetIndex = UVSet.Find(LocalUVSetName);
				if ((SetIndex != 0 && SetIndex != INDEX_NONE) || ScaleU != 1.0f || ScaleV != 1.0f)
				{
				// Create a texture coord node for the texture sample
				UMaterialExpressionTextureCoordinate* MyCoordExpression = ConstructObject<UMaterialExpressionTextureCoordinate>(UMaterialExpressionTextureCoordinate::StaticClass(), UnrealMaterial);
				UnrealMaterial->Expressions.Add(MyCoordExpression);
				MyCoordExpression->CoordinateIndex = (SetIndex >= 0) ? SetIndex : 0;
				MyCoordExpression->UTiling = ScaleU;
				MyCoordExpression->VTiling = ScaleV;
				UnrealTextureExpression->Coordinates.Expression = MyCoordExpression;
				MyCoordExpression->MaterialExpressionEditorX = FMath::TruncToInt(Location.X - 175);
				MyCoordExpression->MaterialExpressionEditorY = FMath::TruncToInt(Location.Y);

				}

				*/
				bCreated = true;
			}
		}
	}

	if (MaterialInput.Expression)
	{
		TArray<FExpressionOutput> Outputs = MaterialInput.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		MaterialInput.Mask = Output->Mask;
		MaterialInput.MaskR = Output->MaskR;
		MaterialInput.MaskG = Output->MaskG;
		MaterialInput.MaskB = Output->MaskB;
		MaterialInput.MaskA = Output->MaskA;
	}
#endif
#if 0
		}
	}
#endif
	return bCreated;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void UPmxMaterialImport::FixupMaterial(
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	UMaterial* UnrealMaterial)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	if (UnrealMaterial->GetEditorOnlyData()->BaseColor.Expression == NULL)
	{
		UnrealMaterial->BlendMode = BLEND_Masked;
		// FbxDouble3 DiffuseColor;

		UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
		UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MyColorExpression);
		UnrealMaterial->GetEditorOnlyData()->BaseColor.Expression = MyColorExpression;
		UnrealMaterial->GetEditorOnlyData()->OpacityMask.Connect(4, MyColorExpression);
		MyColorExpression->MaterialExpressionEditorX = -500;
		MyColorExpression->MaterialExpressionEditorY = 00;
		MyColorExpression->SetEditableName("DiffuseColor");

		bool bFoundDiffuseColor = true;
		/*
		if (PmxMaterial.GetClassId().Is(FbxSurfacePhong::ClassId))
		{
		DiffuseColor = ((FbxSurfacePhong&)(PmxMaterial)).Diffuse.Get();
		}
		else if (PmxMaterial.GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
		DiffuseColor = ((FbxSurfaceLambert&)(PmxMaterial)).Diffuse.Get();
		}
		else
		{
		bFoundDiffuseColor = false;
		}*/
		if (bFoundDiffuseColor)
		{
			MyColorExpression->DefaultValue.R = PmxMaterial.Diffuse[0]; // R
			MyColorExpression->DefaultValue.G = PmxMaterial.Diffuse[1]; // G
			MyColorExpression->DefaultValue.B = PmxMaterial.Diffuse[2]; // B
			MyColorExpression->DefaultValue.A = PmxMaterial.Diffuse[3]; // A
		}
		else
		{
			// use random color because there may be multiple materials, then they can be different
			MyColorExpression->DefaultValue.R = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.G = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.B = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
		}

		TArray<FExpressionOutput> Outputs = UnrealMaterial->GetEditorOnlyData()->BaseColor.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->GetEditorOnlyData()->BaseColor.Mask = Output->Mask;
		UnrealMaterial->GetEditorOnlyData()->BaseColor.MaskR = Output->MaskR;
		UnrealMaterial->GetEditorOnlyData()->BaseColor.MaskG = Output->MaskG;
		UnrealMaterial->GetEditorOnlyData()->BaseColor.MaskB = Output->MaskB;
		UnrealMaterial->GetEditorOnlyData()->BaseColor.MaskA = Output->MaskA;
	}

	//////////////////////////
	#if 1
	// add a basic diffuse color if no texture is linked to diffuse
	if (UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.Expression == NULL)
	{
		// FbxDouble3 DiffuseColor;

		UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
		UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MyColorExpression);
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.Expression = MyColorExpression;
		MyColorExpression->MaterialExpressionEditorX = -500;
		MyColorExpression->MaterialExpressionEditorY = 500;
		MyColorExpression->SetEditableName("AmbientColor");

		bool bFoundDiffuseColor = true;
		/*
		if (PmxMaterial.GetClassId().Is(FbxSurfacePhong::ClassId))
		{
		DiffuseColor = ((FbxSurfacePhong&)(PmxMaterial)).Diffuse.Get();
		}
		else if (PmxMaterial.GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
		DiffuseColor = ((FbxSurfaceLambert&)(PmxMaterial)).Diffuse.Get();
		}
		else
		{
		bFoundDiffuseColor = false;
		}*/
		if (bFoundDiffuseColor)
		{
			MyColorExpression->DefaultValue.R = PmxMaterial.Ambient[0];
			MyColorExpression->DefaultValue.G = PmxMaterial.Ambient[1];
			MyColorExpression->DefaultValue.B = PmxMaterial.Ambient[2];
		}
		else
		{
			// use random color because there may be multiple materials, then they can be different
			MyColorExpression->DefaultValue.R = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.G = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.B = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
		}

		TArray<FExpressionOutput> Outputs = UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.Mask = Output->Mask;
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.MaskR = Output->MaskR;
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.MaskG = Output->MaskG;
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.MaskB = Output->MaskB;
		UnrealMaterial->GetEditorOnlyData()->AmbientOcclusion.MaskA = Output->MaskA;
	}
	#endif
#else
	if (UnrealMaterial->BaseColor.Expression == NULL)
	{
		UnrealMaterial->BlendMode = BLEND_Masked;
		// FbxDouble3 DiffuseColor;

		UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
		UnrealMaterial->Expressions.Add(MyColorExpression);
		UnrealMaterial->BaseColor.Expression = MyColorExpression;
		UnrealMaterial->OpacityMask.Connect(4, MyColorExpression);
		MyColorExpression->MaterialExpressionEditorX = -500;
		MyColorExpression->MaterialExpressionEditorY = 00;
		MyColorExpression->SetEditableName("DiffuseColor");

		bool bFoundDiffuseColor = true;
		/*
		if (PmxMaterial.GetClassId().Is(FbxSurfacePhong::ClassId))
		{
		DiffuseColor = ((FbxSurfacePhong&)(PmxMaterial)).Diffuse.Get();
		}
		else if (PmxMaterial.GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
		DiffuseColor = ((FbxSurfaceLambert&)(PmxMaterial)).Diffuse.Get();
		}
		else
		{
		bFoundDiffuseColor = false;
		}*/
		if (bFoundDiffuseColor)
		{
			MyColorExpression->DefaultValue.R = PmxMaterial.Diffuse[0]; // R
			MyColorExpression->DefaultValue.G = PmxMaterial.Diffuse[1]; // G
			MyColorExpression->DefaultValue.B = PmxMaterial.Diffuse[2]; // B
			MyColorExpression->DefaultValue.A = PmxMaterial.Diffuse[3]; // A
		}
		else
		{
			// use random color because there may be multiple materials, then they can be different
			MyColorExpression->DefaultValue.R = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.G = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.B = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
		}

		TArray<FExpressionOutput> Outputs = UnrealMaterial->BaseColor.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->BaseColor.Mask = Output->Mask;
		UnrealMaterial->BaseColor.MaskR = Output->MaskR;
		UnrealMaterial->BaseColor.MaskG = Output->MaskG;
		UnrealMaterial->BaseColor.MaskB = Output->MaskB;
		UnrealMaterial->BaseColor.MaskA = Output->MaskA;
	}

	//////////////////////////
	#if 1
	// add a basic diffuse color if no texture is linked to diffuse
	if (UnrealMaterial->AmbientOcclusion.Expression == NULL)
	{
		// FbxDouble3 DiffuseColor;

		UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
		UnrealMaterial->Expressions.Add(MyColorExpression);
		UnrealMaterial->AmbientOcclusion.Expression = MyColorExpression;
		MyColorExpression->MaterialExpressionEditorX = -500;
		MyColorExpression->MaterialExpressionEditorY = 500;
		MyColorExpression->SetEditableName("AmbientColor");

		bool bFoundDiffuseColor = true;
		/*
		if (PmxMaterial.GetClassId().Is(FbxSurfacePhong::ClassId))
		{
		DiffuseColor = ((FbxSurfacePhong&)(PmxMaterial)).Diffuse.Get();
		}
		else if (PmxMaterial.GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
		DiffuseColor = ((FbxSurfaceLambert&)(PmxMaterial)).Diffuse.Get();
		}
		else
		{
		bFoundDiffuseColor = false;
		}*/
		if (bFoundDiffuseColor)
		{
			MyColorExpression->DefaultValue.R = PmxMaterial.Ambient[0];
			MyColorExpression->DefaultValue.G = PmxMaterial.Ambient[1];
			MyColorExpression->DefaultValue.B = PmxMaterial.Ambient[2];
		}
		else
		{
			// use random color because there may be multiple materials, then they can be different
			MyColorExpression->DefaultValue.R = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.G = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
			MyColorExpression->DefaultValue.B = 0.5f + (0.5f * FMath::Rand()) / RAND_MAX;
		}

		TArray<FExpressionOutput> Outputs = UnrealMaterial->AmbientOcclusion.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->AmbientOcclusion.Mask = Output->Mask;
		UnrealMaterial->AmbientOcclusion.MaskR = Output->MaskR;
		UnrealMaterial->AmbientOcclusion.MaskG = Output->MaskG;
		UnrealMaterial->AmbientOcclusion.MaskB = Output->MaskB;
		UnrealMaterial->AmbientOcclusion.MaskA = Output->MaskA;
	}
	#endif
#endif

	UnrealMaterial->TwoSided = PmxMaterial.CullingOff;

	// IF Translucent Opacu?
	// UnrealMaterial->BlendMode = BLEND_Translucent;
	// IF Addtion
	// UnrealMaterial->BlendMode = BLEND_Additive;
}

void UPmxMaterialImport::CreateUnrealMaterial(
	FString ParentObjName,
	// UObject * InParent,
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	bool bCreateMaterialInstMode,
	bool bMaterialUnlit,
	TArray<UMaterialInterface*>& OutMaterials,
	TArray<UTexture*>& textureAssetList)
{
	FString MaterialFullName = //"M_" +
		PmxMaterial.Name;	   // ANSI_TO_TCHAR(MakeName(PmxMaterial.Name));
	// 删除禁止文字（如果材质名称中包含禁止文字时的故障保护）)
	MaterialFullName = ObjectTools::SanitizeObjectName(MaterialFullName);

	// 删除禁止字符（如果模型名称中包含禁止字符时的故障保护）)
	ParentObjName = ObjectTools::SanitizeObjectName(ParentObjName);

	if (false == bCreateMaterialInstMode)
	{
		if (MaterialFullName.Len() > 6)
		{
			int32 Offset = MaterialFullName.Find(TEXT("_SKIN"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			if (Offset != INDEX_NONE)
			{
				// Chop off the material name so we are left with the number in _SKINXX
				FString SkinXXNumber = MaterialFullName.Right(MaterialFullName.Len() - (Offset + 1)).RightChop(4);

				if (SkinXXNumber.IsNumeric())
				{
					// remove the '_skinXX' suffix from the material name
					MaterialFullName = MaterialFullName.LeftChop(Offset + 1);
				}
			}
		}

		// MaterialFullName = ObjectTools::SanitizeObjectName(MaterialFullName);

		// Make sure we have a parent
		if (!ensure(InParent))
		{
			return;
		}

		FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / MaterialFullName;
		BasePackageName = PackageTools::SanitizePackageName(BasePackageName);

		// The material could already exist in the project
		FName ObjectPath = *(BasePackageName + TEXT(".") + MaterialFullName);

		/*
		if( ImportedMaterialData.IsUnique( PmxMaterial, ObjectPath ) )
		{
			UMaterialInterface* FoundMaterial = ImportedMaterialData.GetUnrealMaterial( PmxMaterial );
			if (FoundMaterial)
			{
				// The material was imported from this FBX.  Reuse it
				OutMaterials.Add(FoundMaterial);
				return;
			}
		}
		else*/
		{
			UMaterialInterface* FoundMaterial = LoadObject<UMaterialInterface>(NULL, *ObjectPath.ToString());
			// do not override existing materials
			if (FoundMaterial)
			{
				// ImportedMaterialData.AddImportedMaterial( PmxMaterial, *FoundMaterial );
				OutMaterials.Add(FoundMaterial);
				return;
			}
		}

		const FString Suffix(TEXT(""));
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FString FinalPackageName;
		AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, MaterialFullName);

		UPackage* Package = CreatePackage(*FinalPackageName);

		// create an unreal material asset
		auto MaterialFactory = NewObject<UMaterialFactoryNew>();

		UMaterial* UnrealMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
			UMaterial::StaticClass(), Package, *MaterialFullName, RF_Standalone | RF_Public, NULL, GWarn);

		if (UnrealMaterial != NULL)
		{
			// Notify the asset registry
			FAssetRegistryModule::AssetCreated(UnrealMaterial);

			// Set the dirty flag so this package will get saved later
			Package->SetDirtyFlag(true);
		}

		// textures and properties
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
		if (
			CreateAndLinkExpressionForMaterialProperty(
				PmxMaterial,
				UnrealMaterial,
				NULL,
				UnrealMaterial->GetEditorOnlyData()->BaseColor,
				false,
				FVector2D(240, -320),
				textureAssetList)
			== true)
		{
		}
		else
		{
		}
#else
		if (
			CreateAndLinkExpressionForMaterialProperty(
				PmxMaterial,
				UnrealMaterial,
				NULL,
				UnrealMaterial->BaseColor,
				false,
				FVector2D(240, -320),
				textureAssetList)
			== true)
		{
		}
		else
		{
		}
#endif

		FixupMaterial(PmxMaterial, UnrealMaterial); // add random diffuse if none exists

		// compile shaders for PC (from UPrecompileShadersCommandlet::ProcessMaterial
		// and FMaterialEditor::UpdateOriginalMaterial)

		// make sure that any static meshes, etc using this material will stop using the FMaterialResource of the original
		// material, and will use the new FMaterialResource created when we make a new UMaterial in place
		FGlobalComponentReregisterContext RecreateComponents;

		// let the material update itself if necessary
		UnrealMaterial->PreEditChange(NULL);
		UnrealMaterial->PostEditChange();

		UnrealMaterial->MarkPackageDirty();
		// ImportedMaterialData.AddImportedMaterial( FbxMaterial, *UnrealMaterial );

		OutMaterials.Add(UnrealMaterial);
	}
	else
	{
		/*
		 * 以下，从附带IVP5U插件的基本材质进行复制，制作MaterialInstance的模式
		 */

		/* 生成MI*/
		UMaterialInterface* UnrealMaterial_MI = nullptr;
		/* 生成处理：按优先顺序实施*/
		do
		{
			// Unlit Material
			if (bMaterialUnlit)
			{
				/* MMD AutoLuminous 疑似設定 */
				if (PmxMaterial.SpecularPower > 100) // auto luminus
				{
					UnrealMaterial_MI = this->CreateMaterialInst_Luminous_Unlit(
						ParentObjName, PmxMaterial, MaterialFullName, textureAssetList);
					if (UnrealMaterial_MI)
					{
						UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Material MIC -Luminous Unlit- OK"),
							*(FString(__FUNCTION__)));
						break;
					}
				}

				/* MMD 通常材質設定 */
				UnrealMaterial_MI = this->CreateMaterialInst_Masked_Unlit(
					ParentObjName, PmxMaterial, MaterialFullName, textureAssetList);
				if (UnrealMaterial_MI)
				{
					UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Material MIC -Masked Unlit- OK"),
						*(FString(__FUNCTION__)));
					break;
				}
			}
			else
			{
				/* MMD AutoLuminous 疑似設定 */
				if (PmxMaterial.SpecularPower > 100) // auto luminus
				{
					UnrealMaterial_MI = this->CreateMaterialInst_Luminous(
						ParentObjName, PmxMaterial, MaterialFullName, textureAssetList);
					if (UnrealMaterial_MI)
					{
						UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Material MIC -Luminous- OK"),
							*(FString(__FUNCTION__)));
						break;
					}
				}

				/* MMD 通常材質設定 */
				UnrealMaterial_MI = this->CreateMaterialInst_Masked(ParentObjName, PmxMaterial, MaterialFullName,
					textureAssetList);
				if (UnrealMaterial_MI)
				{
					UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Material MIC -Masked- OK"),
						*(FString(__FUNCTION__)));
					break;
				}
			}

			// 异常情况：生成失败
			{
				// OutMaterials.Add(UnrealMaterial);
				UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material MIC Null.Error[%s]:[%s]"),
					*(FString(__FUNCTION__)), *ParentObjName, *MaterialFullName);
				return;
			}
		}
		while (false);

		OutMaterials.Add(UnrealMaterial_MI);
	}
}

//--------------------------------------------------------------------
//
//-------------------------------------------------------------------------
bool UPmxMaterialImport::CreateAndLinkExpressionForMaterialProperty_ForMmdAutoluminus(
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	UMaterial* UnrealMaterial,
	// const char* MaterialProperty,
	FExpressionInput& MaterialInput,
	// bool bSetupAsNormalMap,
	// TArray<FString>& UVSet,
	const FVector2D& Location,
	TArray<UTexture*>& textureAssetList)
{
	bool bCreated = false;
#if 1
	int32 TextureCount = PmxMaterial.TextureIndex; // FbxProperty.GetSrcObjectCount<FbxTexture>();
	if (TextureCount >= 0 && TextureCount < textureAssetList.Num())
	{
		// for (int32 TextureIndex = 0; TextureIndex<TextureCount; ++TextureIndex)
		if (PmxMaterial.SphereMode == 2) // auto luminus
		{
			// FbxFileTexture* FbxTexture = FbxProperty.GetSrcObject<FbxFileTexture>(TextureIndex);

			// create an unreal texture asset
			UTexture* UnrealTexture = textureAssetList[TextureCount]; // ImportTexture(FbxTexture, bSetupAsNormalMap);

			if (UnrealTexture)
			{
				UnrealMaterial->BlendMode = BLEND_Masked;
				// float ScaleU = FbxTexture->GetScaleU();
				// float ScaleV = FbxTexture->GetScaleV();

				// Multipule
				UMaterialExpressionMultiply* MulExpression = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression);
				UnrealMaterial->GetEditorOnlyData()->BaseColor.Expression = MulExpression; // test
	#else
				UnrealMaterial->Expressions.Add(MulExpression);
				UnrealMaterial->BaseColor.Expression = MulExpression; // test
	#endif

				MulExpression->MaterialExpressionEditorX = -250;
				MulExpression->MaterialExpressionEditorY = 0;
				MulExpression->bHidePreviewWindow = 0;

				MulExpression->Desc = TEXT("Textuer * Texture alpha -> BaseColor");

				// Multipule
				UMaterialExpressionMultiply* MulExpression_2 = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression_2);
	#else
				UnrealMaterial->Expressions.Add(MulExpression_2);
	#endif

				// UnrealMaterial->OpacityMask.Expression = MulExpression_2;
				MulExpression_2->MaterialExpressionEditorX = -250;
				MulExpression_2->MaterialExpressionEditorY = 200;
				MulExpression_2->bHidePreviewWindow = 0;

				MulExpression_2->Desc = TEXT("Textuer alpha * Specure Coloer -> OpacityMask");
				// MulExpression->ConstA = 1.0f;
				// MulExpression->ConstB = FresnelBaseReflectFraction_DEPRECATED;

				// MulExpression->A.Connect(SpecularColor_DEPRECATED.OutputIndex, SpecularColor_DEPRECATED.Expression);
				// SpecularColor_DEPRECATED.Connect(0, MulExpression);

				// Multipule
				UMaterialExpressionMultiply* MulExpression_3 = NewObject<UMaterialExpressionMultiply>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MulExpression_3);
	#else
				UnrealMaterial->Expressions.Add(MulExpression_3);
	#endif

				// UnrealMaterial->EmissiveColor.Expression = MulExpression_3; //TES: lighting EmmisicveColor For AutoLuminous
				MulExpression_3->MaterialExpressionEditorX = -250;
				MulExpression_3->MaterialExpressionEditorY = 400;
				MulExpression_3->bHidePreviewWindow = 0;
				MulExpression_3->A.Expression = MulExpression_2;

				MulExpression_3->Desc = TEXT("Textuer alpha * Specure Coloer -> OpacityMask");
				// MulExpression->ConstA = 1.0f;
				// MulExpression->ConstB = FresnelBaseReflectFraction_DEPRECATED;

				// MulExpression->A.Connect(SpecularColor_DEPRECATED.OutputIndex, SpecularColor_DEPRECATED.Expression);
				// SpecularColor_DEPRECATED.Connect(0, MulExpression);

				// A
				// and link it to the material
				UMaterialExpressionTextureSample* UnrealTextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(UnrealTextureExpression);
	#else
				UnrealMaterial->Expressions.Add(UnrealTextureExpression);
	#endif

				// MaterialInput.Expression = UnrealTextureExpression;
				MulExpression->A.Expression = UnrealTextureExpression;
				MulExpression->B.Connect(4, UnrealTextureExpression);
				MulExpression_2->B.Connect(4, UnrealTextureExpression);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->OpacityMask.Connect(4, UnrealTextureExpression);
	#else
				UnrealMaterial->OpacityMask.Connect(4, UnrealTextureExpression);
	#endif

				// TEST: Non Light EmmisciveColor For Easy AutoLuminous
				// MulExpression->B.Expression = UnrealTextureExpression.Outputs[4];
				UnrealTextureExpression->Texture = UnrealTexture;
				UnrealTextureExpression->SamplerType = /*bSetupAsNormalMap ? SAMPLERTYPE_Normal :*/ SAMPLERTYPE_Color;
				UnrealTextureExpression->MaterialExpressionEditorX = -500; // FMath::TruncToInt(Location.X);
				UnrealTextureExpression->MaterialExpressionEditorY = 0;	   // FMath::TruncToInt(Location.Y);
				UnrealTextureExpression->SamplerSource = SSM_Wrap_WorldGroupSettings;
				// For minus UV asix MMD(e.g. AnjeraBalz///)

				// MulExpression->B.Connect(UnrealTextureExpression->Outputs[4].Expression);

				// B
				UMaterialExpressionVectorParameter* MyColorExpression = NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MyColorExpression);
	#else
				UnrealMaterial->Expressions.Add(MyColorExpression);
	#endif

				// UnrealMaterial->BaseColor.Expression = MyColorExpression;
				// MulExpression->B.Expression = MyColorExpression;
				MulExpression_2->A.Expression = MyColorExpression;

				MyColorExpression->DefaultValue.R = PmxMaterial.Diffuse[0];
				MyColorExpression->DefaultValue.G = PmxMaterial.Diffuse[1];
				MyColorExpression->DefaultValue.B = PmxMaterial.Diffuse[2];
				MyColorExpression->DefaultValue.A = PmxMaterial.Diffuse[3]; // A
				MyColorExpression->MaterialExpressionEditorX = -500;
				MyColorExpression->MaterialExpressionEditorY = 300;
				MyColorExpression->SetEditableName("DiffuseColor");

				// const
				UMaterialExpressionConstant* MyConstExpression = NewObject<UMaterialExpressionConstant>(UnrealMaterial);
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
				UnrealMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(MyConstExpression);
	#else
				UnrealMaterial->Expressions.Add(MyConstExpression);
	#endif

				// UnrealMaterial->BaseColor.Expression = MyColorExpression;
				// MulExpression->B.Expression = MyColorExpression;
				MulExpression_3->B.Expression = MyConstExpression;

				MyConstExpression->R = PmxMaterial.SpecularPower - 100.;
				MyConstExpression->MaterialExpressionEditorX = -500;
				MyConstExpression->MaterialExpressionEditorY = 500;

				bCreated = true;
			}
		}
	}

	if (MaterialInput.Expression)
	{
		TArray<FExpressionOutput> Outputs = MaterialInput.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		MaterialInput.Mask = Output->Mask;
		MaterialInput.MaskR = Output->MaskR;
		MaterialInput.MaskG = Output->MaskG;
		MaterialInput.MaskB = Output->MaskB;
		MaterialInput.MaskA = Output->MaskA;
	}
#endif
	return bCreated;
}

//--------------------------------------------------------------------
//
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::DuplicateBaseMaterial(
	FString ParentObjName,
	EDuplicateBaseMatTypeIndex targetMatIndex)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser");
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	FString DupAssetBaseName;

	if (EDuplicateBaseMatTypeIndex::E_DupBaseMat_Typ_Normal == targetMatIndex)
	{
		DupAssetBaseName = D_IVP5U_MMDBaseMat_Path_Normal;
	}
	else if (EDuplicateBaseMatTypeIndex::E_DupBaseMat_Typ_Luminous == targetMatIndex)
	{
		DupAssetBaseName = D_IVP5U_MMDBaseMat_Path_Luminou;
	}
	else if (EDuplicateBaseMatTypeIndex::E_DupBaseMat_Typ_Unlit_Normal == targetMatIndex)
	{
		DupAssetBaseName = D_IVP5U_MMDBaseMat_Path_Unlit_Normal;
	}
	else if (EDuplicateBaseMatTypeIndex::E_DupBaseMat_Typ_Unlit_Luminous == targetMatIndex)
	{
		DupAssetBaseName = D_IVP5U_MMDBaseMat_Path_Unlit_Luminou;
	}
	else
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]::BaseMaterial Kind Not Found... "),
			*(FString(__FUNCTION__)));
		return nullptr;
	}

	// 获取基础材质的复制源
	UMaterial* BaseMatOriginal = nullptr;

	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(*DupAssetBaseName));
	// TArray<FAssetData> AssetDataArray;
	// AssetRegistryModule.Get().GetAssetsByPath(FName(*DupAssetBaseName), AssetDataArray, true);
	// FAssetData AssetData = AssetDataArray[0];
	BaseMatOriginal = Cast<UMaterial>(AssetData.GetAsset());
	// check(BaseMatOriginal);
	if (nullptr == BaseMatOriginal)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:BaseMaterial Not Found... Path[%s]"),
			*(FString(__FUNCTION__)), *DupAssetBaseName);
		return nullptr;
	}

	TArray<UObject*> ObjectsToSync;

	// 复制基础材质
	FString TargetPathName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());
	UMaterial* BaseMat = nullptr;
	FString BaseMatSimpleName;
	{
		BaseMatSimpleName = BaseMatOriginal->GetName().Replace(
			TEXT("M_MMD_MatBase_"), TEXT(""), ESearchCase::CaseSensitive);
		FString BaseMatName = FString::Printf(TEXT("M_%s_Base_%s"), *ParentObjName, *BaseMatSimpleName);

		// The material could already exist in the project
		FName ObjectPath = *(TargetPathName / BaseMatName + TEXT(".") + BaseMatName);
		// Existing check
		UMaterialInterface* FoundMaterial = LoadObject<UMaterialInterface>(NULL, *ObjectPath.ToString());
		// do not override existing materials
		if (FoundMaterial)
		{
			if (Cast<UMaterial>(FoundMaterial))
			{
				/* 因为已经存在同名的MaterialAsset了*/

				UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Existing Material. can't Duplicate. Path[%s]"),
					*(FString(__FUNCTION__)), *ObjectPath.ToString());
				return FoundMaterial;
			}
			else
			{
				/* 因为存在相同的AssetName的不同种类所以NG*/

				UE_LOG(LogCategoryPMXMaterialImport, Error,
					TEXT("[%s]:Existing Material. can't Duplicate. and Not Mat-Assets. Path[%s]"),
					*(FString(__FUNCTION__)), *ObjectPath.ToString());
				return nullptr;
			}
		}

		UObject* DuplicatedObject = AssetToolsModule.Get().DuplicateAsset(
			BaseMatName,
			TargetPathName,
			BaseMatOriginal);
		BaseMat = Cast<UMaterial>(DuplicatedObject);
		if (nullptr == BaseMat)
		{
			UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material can't Duplicate. Path[%s]"),
				*(FString(__FUNCTION__)), *ObjectPath.ToString());
			return nullptr;
			// continue;
		}
	}

	if (0 < ObjectsToSync.Num())
	{
		ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync, true);
	}
	return BaseMat;
}

//-------------------------------------------------------------------------
// Create Material Instance
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::CreateMaterialInst(
	FString ParentObjName,
	FString TargetMaterialName,
	UMaterialInterface* ParentMaterial)
{
	if ((nullptr == ParentMaterial))
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Parameter nullptr. New MIC Create NG.[%s]"),
			*(FString(__FUNCTION__)), *TargetMaterialName);
		return nullptr;
	}

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// 新MIC名称
	FString NewMICName = FString::Printf(TEXT("MI_%s"), *TargetMaterialName);

	FString TargetPathName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / NewMICName;

	// The material could already exist in the project
	FName ObjectPath = *(TargetPathName + TEXT(".") + NewMICName);
	// Existing check
	UMaterialInterface* FoundMaterial = LoadObject<UMaterialInterface>(NULL, *ObjectPath.ToString());
	// do not override existing materials
	if (FoundMaterial)
	{
		if (Cast<UMaterialInstanceConstant>(FoundMaterial))
		{
			/*因为已经存在同名的MaterialAsset*/
			// 话说，正常再利用可以吗？
			UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Existing Material MIC. can't Duplicate. Path[%s]"),
				*(FString(__FUNCTION__)), *ObjectPath.ToString());
			return FoundMaterial;
		}
		else
		{
			/* 因为存在相同的AssetName的不同种类所以NG */

			UE_LOG(LogCategoryPMXMaterialImport, Error,
				TEXT("[%s]:Existing Material MIC. can't Duplicate. and Not Mat-Assets. Path[%s]"),
				*(FString(__FUNCTION__)), *ObjectPath.ToString());
			return nullptr;
		}
	}

	// 新MIC作成
	UMaterialInstanceConstant* NewMIC = nullptr;
	{
		UMaterialInstanceConstantFactoryNew* Factory =
			NewObject<UMaterialInstanceConstantFactoryNew>();
		Factory->InitialParent = ParentMaterial;
#if 0 // 简易版，这样不也可以吗？
		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
			NewMICName,
			TargetPathName,
			UMaterialInstanceConstant::StaticClass(),
			Factory
		);

#else // 与SetDirtyFlag一致的模式
		const FString Suffix(TEXT(""));
		FString FinalPackageName;
		AssetToolsModule.Get().CreateUniqueAssetName(TargetPathName, Suffix, FinalPackageName, NewMICName);

		UPackage* Package = CreatePackage(*FinalPackageName);
		UObject* NewAsset = Factory->FactoryCreateNew(
			UMaterialInstanceConstant::StaticClass(), Package, *NewMICName, RF_Standalone | RF_Public, NULL, GWarn);

		if (Cast<UMaterialInstanceConstant>(NewAsset))
		{
			// Notify the asset registry
			FAssetRegistryModule::AssetCreated(NewAsset);

			// Set the dirty flag so this package will get saved later
			Package->SetDirtyFlag(true);
		}
#endif
		NewMIC = Cast<UMaterialInstanceConstant>(NewAsset);
	}
	if (nullptr == NewMIC)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material Instance Asset Create NG.[%s][%s]"),
			*(FString(__FUNCTION__)), *TargetPathName, *NewMICName);
		return nullptr;
	}

	return NewMIC;
}

//--------------------------------------------------------------------
// Create Material Inst. for Masked Mat
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::CreateMaterialInst_Masked(
	FString ParentObjName,
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	FString MaterialFullName,
	TArray<UTexture*>& textureAssetList)
{
	UMaterialInterface* UnrealMaterial = nullptr;

	UMaterialInterface* ParentMaterial = this->DuplicateBaseMaterial(ParentObjName, E_DupBaseMat_Typ_Normal);
	if (nullptr == ParentMaterial)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Parent Material NULL:Path[%s]"),
			*(FString(__FUNCTION__)), *ParentObjName);

		return nullptr;
	}

	UnrealMaterial = CreateMaterialInst(ParentObjName, MaterialFullName, ParentMaterial);

	UMaterialInstanceConstant* pUMIC = nullptr;
	pUMIC = Cast<UMaterialInstanceConstant>(UnrealMaterial);
	if (nullptr == pUMIC)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material NULL"), *(FString(__FUNCTION__)));

		return nullptr;
	}
	if (PmxMaterial.SphereMode > 0)
	{
		pUMIC->BasePropertyOverrides.BlendMode = BLEND_Masked;
		pUMIC->BasePropertyOverrides.bOverride_BlendMode = true;
	}
	// Set Param
	FStaticParameterSet StaticParams;

	{

		UTexture* ColorTex = nullptr;
		int32 TextureCount = PmxMaterial.TextureIndex;

		if ((0 <= TextureCount) && (TextureCount < textureAssetList.Num()))
		{
			ColorTex = textureAssetList[TextureCount];
		}

		if (nullptr != ColorTex)
		{
			pUMIC->SetTextureParameterValueEditorOnly(
				FName(TEXT(D_IVP5U_MatInst_Name_BaseTexture)),
				ColorTex);

			// ColorTex
			FStaticSwitchParameter Param;
			Param.ParameterInfo.Name = FName(D_IVP5U_MatInst_Name_isTextureEnable);
			Param.Value = true;
			Param.bOverride = true;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
			StaticParams.StaticSwitchParameters.Add(Param);
#else
			StaticParams.StaticSwitchParameters.Add(Param);
#endif
			UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Base Texure mode enable "), *(FString(__FUNCTION__)));
		}
	}
	{
		UTexture* ColorTex = nullptr;
		int32 TextureCount = PmxMaterial.ToonTextureIndex;

		if ((0 <= TextureCount) && (TextureCount < textureAssetList.Num()))
		{
			ColorTex = textureAssetList[TextureCount];
		}

		if (nullptr != ColorTex)
		{
			pUMIC->SetTextureParameterValueEditorOnly(
				FName(TEXT(D_IVP5U_MatInst_Name_Toon)),
				ColorTex);

			// ColorTex有効
			FStaticSwitchParameter Param;
			Param.ParameterInfo.Name = FName(D_IVP5U_MatInst_Name_isToonEnable);
			Param.Value = true;
			Param.bOverride = true;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
			StaticParams.StaticSwitchParameters.Add(Param); // DestSeq->MarkRawDataAsModified();
#else
			StaticParams.StaticSwitchParameters.Add(Param);
#endif
			UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:Toon Texure mode enable "), *(FString(__FUNCTION__)));
		}
	}

	// Diffuse Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_DiffuseColor)),
			FLinearColor(
				PmxMaterial.Diffuse[0],
				PmxMaterial.Diffuse[1],
				PmxMaterial.Diffuse[2],
				PmxMaterial.Diffuse[3]));
	}
	// Ambient Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_AmbientColor)),
			FLinearColor(
				PmxMaterial.Ambient[0],
				PmxMaterial.Ambient[1],
				PmxMaterial.Ambient[2]));
	}

	if (pUMIC->TwoSided != PmxMaterial.CullingOff)
	{
		// TowSide有効
		pUMIC->BasePropertyOverrides.TwoSided = PmxMaterial.CullingOff;
		pUMIC->BasePropertyOverrides.bOverride_TwoSided = true;
		pUMIC->UpdateOverridableBaseProperties(); // OverrideParam Update
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC TwoSided mode enable "), *(FString(__FUNCTION__)));
	}

	// StaticSwitch
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#else
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#endif

	return UnrealMaterial;
}

//--------------------------------------------------------------------
// Create Material Inst. for Masked Mat Unlit
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::CreateMaterialInst_Masked_Unlit(
	FString ParentObjName,
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	FString MaterialFullName,
	TArray<UTexture*>& textureAssetList)
{
	UMaterialInterface* UnrealMaterial = nullptr;

	UMaterialInterface* ParentMaterial = this->DuplicateBaseMaterial(ParentObjName, E_DupBaseMat_Typ_Unlit_Normal);
	if (nullptr == ParentMaterial)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Parent Material NULL:Path[%s]"),
			*(FString(__FUNCTION__)), *ParentObjName);

		return nullptr;
	}

	UnrealMaterial = CreateMaterialInst(ParentObjName, MaterialFullName, ParentMaterial);

	UMaterialInstanceConstant* pUMIC = nullptr;
	pUMIC = Cast<UMaterialInstanceConstant>(UnrealMaterial);
	if (nullptr == pUMIC)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material NULL"), *(FString(__FUNCTION__)));

		return nullptr;
	}

	// Set Param
	FStaticParameterSet StaticParams;
	// 纹理到新MIC
	UTexture* ColorTex = nullptr;
	int32 TextureCount = PmxMaterial.TextureIndex;
	// 范围内
	if ((0 <= TextureCount) && (TextureCount < textureAssetList.Num()))
	{
		ColorTex = textureAssetList[TextureCount];
	}

	if (nullptr != ColorTex)
	{
		pUMIC->SetTextureParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_BaseTexture)),
			ColorTex);

		// ColorTex有效时，在StaticSwitch上设为ON
		FStaticSwitchParameter Param;
		Param.ParameterInfo.Name = FName(D_IVP5U_MatInst_Name_isTextureEnable);
		Param.Value = true;
		Param.bOverride = true;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
		StaticParams.StaticSwitchParameters.Add(Param); // 5.2失效
#else
		StaticParams.StaticSwitchParameters.Add(Param); // 5.2失效
#endif
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC Texure mode enable "), *(FString(__FUNCTION__)));
	}

	// Diffuse Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_DiffuseColor)),
			FLinearColor(
				PmxMaterial.Diffuse[0],
				PmxMaterial.Diffuse[1],
				PmxMaterial.Diffuse[2],
				PmxMaterial.Diffuse[3]));
	}
	// Ambient Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_AmbientColor)),
			FLinearColor(
				PmxMaterial.Ambient[0],
				PmxMaterial.Ambient[1],
				PmxMaterial.Ambient[2]));
	}

	if (pUMIC->BasePropertyOverrides.TwoSided != PmxMaterial.CullingOff)
	{
		// TowSide有效时设为ON
		pUMIC->BasePropertyOverrides.TwoSided = PmxMaterial.CullingOff;
		pUMIC->BasePropertyOverrides.bOverride_TwoSided = true;
		pUMIC->UpdateOverridableBaseProperties(); // OverrideParam Update
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC TwoSided mode enable "), *(FString(__FUNCTION__)));
	}

	// 应用多个StaticSwitch
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#else
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#endif

	return UnrealMaterial;
}

//--------------------------------------------------------------------
// Create Material Inst. for Luminous Mat
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::CreateMaterialInst_Luminous(
	FString ParentObjName,
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	FString MaterialFullName,
	TArray<UTexture*>& textureAssetList)
{
	UMaterialInterface* UnrealMaterial = nullptr;

	UMaterialInterface* ParentMaterial = this->DuplicateBaseMaterial(ParentObjName, E_DupBaseMat_Typ_Luminous);
	if (nullptr == ParentMaterial)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Parent Material NULL:Path[%s]"),
			*(FString(__FUNCTION__)), *ParentObjName);

		return nullptr;
	}

	UnrealMaterial = CreateMaterialInst(ParentObjName, MaterialFullName, ParentMaterial);

	UMaterialInstanceConstant* pUMIC = nullptr;
	pUMIC = Cast<UMaterialInstanceConstant>(UnrealMaterial);
	if (nullptr == pUMIC)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material NULL"), *(FString(__FUNCTION__)));

		return nullptr;
	}

	// Set Param
	FStaticParameterSet StaticParams;
	UTexture* ColorTex = nullptr;
	int32 TextureCount = PmxMaterial.TextureIndex;
	if ((0 <= TextureCount) && (TextureCount < textureAssetList.Num()))
	{
		ColorTex = textureAssetList[TextureCount];
	}

	if (nullptr != ColorTex)
	{
		pUMIC->SetTextureParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_BaseTexture)),
			ColorTex);

		// ColorTex
		FStaticSwitchParameter Param;
		Param.ParameterInfo.Name = FName(D_IVP5U_MatInst_Name_isTextureEnable);
		Param.Value = true;
		Param.bOverride = true;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
		StaticParams.StaticSwitchParameters.Add(Param);
#else
		StaticParams.StaticSwitchParameters.Add(Param);
#endif
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC Texure mode enable "), *(FString(__FUNCTION__)));
	}

	// Diffuse Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_DiffuseColor)),
			FLinearColor(
				PmxMaterial.Diffuse[0],
				PmxMaterial.Diffuse[1],
				PmxMaterial.Diffuse[2],
				PmxMaterial.Diffuse[3]));
	}
	// Ambient Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_AmbientColor)),
			FLinearColor(
				PmxMaterial.Ambient[0],
				PmxMaterial.Ambient[1],
				PmxMaterial.Ambient[2]));
	}
	// Specular Powor
	{
		float specPowr = 0.0f;
		if (PmxMaterial.SpecularPower > D_IVP5U_Param_SpecularPowor_Min)
		{
			specPowr = (PmxMaterial.SpecularPower - D_IVP5U_Param_SpecularPowor_Min);
			specPowr = specPowr / (D_IVP5U_Param_SpecularPowor_Thd - D_IVP5U_Param_SpecularPowor_Min);
		}
		pUMIC->SetScalarParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_SpecularPower)),
			specPowr);
	}

	if (pUMIC->BasePropertyOverrides.TwoSided != PmxMaterial.CullingOff)
	{
		// TowSide有効
		pUMIC->BasePropertyOverrides.TwoSided = PmxMaterial.CullingOff;
		pUMIC->BasePropertyOverrides.bOverride_TwoSided = true;
		pUMIC->UpdateOverridableBaseProperties(); // OverrideParam Update
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC TwoSided mode enable "), *(FString(__FUNCTION__)));
	}

	// StaticSwitch
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#else
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#endif

	return UnrealMaterial;
}

//--------------------------------------------------------------------
// Create Material Inst. for Luminous Mat Unlit
//-------------------------------------------------------------------------
UMaterialInterface* UPmxMaterialImport::CreateMaterialInst_Luminous_Unlit(
	FString ParentObjName,
	MMD4UE5::PMX_MATERIAL& PmxMaterial,
	FString MaterialFullName,
	TArray<UTexture*>& textureAssetList)
{
	UMaterialInterface* UnrealMaterial = nullptr;

	UMaterialInterface* ParentMaterial = this->DuplicateBaseMaterial(ParentObjName, E_DupBaseMat_Typ_Unlit_Luminous);
	if (nullptr == ParentMaterial)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Parent Material NULL:Path[%s]"),
			*(FString(__FUNCTION__)), *ParentObjName);

		return nullptr;
	}

	UnrealMaterial = CreateMaterialInst(ParentObjName, MaterialFullName, ParentMaterial);

	UMaterialInstanceConstant* pUMIC = nullptr;
	pUMIC = Cast<UMaterialInstanceConstant>(UnrealMaterial);
	if (nullptr == pUMIC)
	{
		UE_LOG(LogCategoryPMXMaterialImport, Error, TEXT("[%s]:Material NULL"), *(FString(__FUNCTION__)));

		return nullptr;
	}

	// Set Param
	FStaticParameterSet StaticParams;
	UTexture* ColorTex = nullptr;
	int32 TextureCount = PmxMaterial.TextureIndex;
	if ((0 <= TextureCount) && (TextureCount < textureAssetList.Num()))
	{
		ColorTex = textureAssetList[TextureCount];
	}

	if (nullptr != ColorTex)
	{
		pUMIC->SetTextureParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_BaseTexture)),
			ColorTex);

		// ColorTex有効
		FStaticSwitchParameter Param;
		Param.ParameterInfo.Name = FName(D_IVP5U_MatInst_Name_isTextureEnable);
		Param.Value = true;
		Param.bOverride = true;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
		StaticParams.StaticSwitchParameters.Add(Param);
#else
		StaticParams.StaticSwitchParameters.Add(Param);
#endif
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC Texure mode enable "), *(FString(__FUNCTION__)));
	}

	// Diffuse Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_DiffuseColor)),
			FLinearColor(
				PmxMaterial.Diffuse[0],
				PmxMaterial.Diffuse[1],
				PmxMaterial.Diffuse[2],
				PmxMaterial.Diffuse[3]));
	}
	// Ambient Color
	{
		pUMIC->SetVectorParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_AmbientColor)),
			FLinearColor(
				PmxMaterial.Ambient[0],
				PmxMaterial.Ambient[1],
				PmxMaterial.Ambient[2]));
	}
	// Specular Powor
	{
		float specPowr = 0.0f;
		if (PmxMaterial.SpecularPower > D_IVP5U_Param_SpecularPowor_Min)
		{
			specPowr = (PmxMaterial.SpecularPower - D_IVP5U_Param_SpecularPowor_Min);
			specPowr = specPowr / (D_IVP5U_Param_SpecularPowor_Thd - D_IVP5U_Param_SpecularPowor_Min);
		}
		pUMIC->SetScalarParameterValueEditorOnly(
			FName(TEXT(D_IVP5U_MatInst_Name_SpecularPower)),
			specPowr);
	}

	if (pUMIC->BasePropertyOverrides.TwoSided != PmxMaterial.CullingOff)
	{
		// TowSide有効の場合はONにする
		pUMIC->BasePropertyOverrides.TwoSided = PmxMaterial.CullingOff;
		pUMIC->BasePropertyOverrides.bOverride_TwoSided = true;
		pUMIC->UpdateOverridableBaseProperties(); // OverrideParam Update
		UE_LOG(LogCategoryPMXMaterialImport, Log, TEXT("[%s]:MIC TwoSided mode enable "), *(FString(__FUNCTION__)));
	}

	// StaticSwitchの適用 複数まとめて
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#else
	if (0 < StaticParams.StaticSwitchParameters.Num())
	{
		pUMIC->UpdateStaticPermutation(StaticParams);
	}
#endif

	return UnrealMaterial;
}

//-------------------------------------------------------------------------
