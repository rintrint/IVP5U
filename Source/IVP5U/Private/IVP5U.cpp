// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "IVP5U.h"
#include "IVP5UPrivatePCH.h"
#include "IVP5USettings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "IVP5U"

class FIVP5U : public IVP5U
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FIVP5U, IVP5U)

void FIVP5U::StartupModule()
{
	// 註冊設定到Project Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "IVP5U",
			LOCTEXT("RuntimeSettingsName", "IVP5U"),
			LOCTEXT("RuntimeSettingsDescription", "Configure IVP5U plugin settings"),
			GetMutableDefault<UIVP5USettings>());
	}
}

void FIVP5U::ShutdownModule()
{
	// 取消註冊設定
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "IVP5U");
	}
}

#undef LOCTEXT_NAMESPACE