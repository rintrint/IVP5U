// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "IVP5USettings.h"

UIVP5USettings::UIVP5USettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UIVP5USettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
