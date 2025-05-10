// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

class IIVP5U : public IModuleInterface
{

public:
    static inline IIVP5U &Get()
    {
        return FModuleManager::LoadModuleChecked<IIVP5U>("IVP5U");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("IVP5U");
    }
};
