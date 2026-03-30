// Copyright 2015-2026 IVP5U contributors

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
            // --- Warning level ---
            // Low noise, safe to enable
            CppCompileWarningSettings.DeprecationWarningLevel = WarningLevel.Warning;
            CppCompileWarningSettings.DeterministicWarningLevel = WarningLevel.Warning;
            CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Warning;
            CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Warning;
            CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Warning;
            // High noise, may flood engine logs
            // CppCompileWarningSettings.ShortenSizeTToIntWarningLevel = WarningLevel.Warning;
            // CppCompileWarningSettings.SwitchUnhandledEnumeratorWarningLevel = WarningLevel.Warning;
            // CppCompileWarningSettings.UnusedParameterWarningLevel = WarningLevel.Warning;

            // --- Include paths ---
            PublicIncludePaths.AddRange(
                new string[] {
                }
            );

            PrivateIncludePaths.AddRange(
                new string[] {
                    "IVP5U/Private",
                }
            );

            // --- Dependencies ─---
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "Slate",
                    "SlateCore",
                    "MessageLog",
                    "RHI",
                    "RenderCore",
                    "IKRig",
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                }
            );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                    "AssetRegistry",
                }
            );

            if (Target.bBuildEditor)
            {
                PublicDependencyModuleNames.AddRange(
                    new string[]
                    {
                        "UnrealEd",
                        "AssetTools",
                        "MainFrame",
                        "PropertyEditor",
                        "ContentBrowser",
                        "PhysicsUtilities",
                        "SkeletalMeshUtilitiesCommon",
                        "IKRigEditor",
                        "RawMesh",
                    }
                );

                PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                        "EditorStyle",
                        "EditorWidgets",
                        "MeshDescription",
                        "SkeletalMeshDescription",
                    }
                );
            }
        }
    }
}
