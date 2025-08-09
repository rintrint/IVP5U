// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "IVP5UPrivatePCH.h"

class FIVP5U : public IVP5U
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FIVP5U, IVP5U)

void FIVP5U::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}

void FIVP5U::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
