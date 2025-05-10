// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class IVP5U : ModuleRules
	{
		private string ModulePath
		{
			get { return ModuleDirectory; }
		}

		private string ThirdPartyPath
		{
			get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
		}

		
		public IVP5U(ReadOnlyTargetRules Target) : base(Target)
		{
			// string LibName;
			if ((Target.Platform == UnrealTargetPlatform.Win64 ))
			{


			}

			
			
			PublicIncludePaths.AddRange(
                new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
                new string[] {
                    "IVP5U/Private",
					ThirdPartyPath+"/glm"
					// ... add other private include paths required here ...
				}
                );

			PublicDependencyModuleNames.AddRange(
				new string[]
				{


					"Core",
					"CoreUObject",
					"Engine", 
					"InputCore", 
					"UnrealEd",
					"AssetTools",
					"Slate" ,
					"SlateCore",
					"RawMesh" ,
					"MessageLog",
					"MainFrame",
					"PropertyEditor",
                    "RHI",
                    "RenderCore",
                    "ContentBrowser",
					"PhysicsUtilities","SkeletalMeshUtilitiesCommon",
					"IKRig",
					"IKRigEditor"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorStyle",
					"EditorWidgets"
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
                {
                    "AssetRegistry",
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}
