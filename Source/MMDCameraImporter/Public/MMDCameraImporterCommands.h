// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MMDCameraImporterStyle.h"

class FMmdCameraImporterCommands final : public TCommands<FMmdCameraImporterCommands>
{
public:
	FMmdCameraImporterCommands()
		: TCommands(TEXT("MMDCameraImporter"), NSLOCTEXT("Contexts", "MMDCameraImporter", "MMDCameraImporter Plugin"), NAME_None, FMmdCameraImporterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportVmd;
};
