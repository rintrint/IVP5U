// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "CoreMinimal.h"

class FMmdImportHelper
{
public:
	static FString ShiftJisToFString(const uint8* InBuffer, int32 InSize);
};
