// Copyright 2015-2026 IVP5U contributors

#include "MMDCameraImporterCommands.h"

#define LOCTEXT_NAMESPACE "FMMDCameraImporterModule"

void FMmdCameraImporterCommands::RegisterCommands()
{
	UI_COMMAND(ImportVmd, "Import VMD", "Import VMD file", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
