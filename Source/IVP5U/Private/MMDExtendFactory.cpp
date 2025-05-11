// Copyright 2023 NaN_Name, Inc. All Rights Reserved.

#include "MMDExtendFactory.h"
#include "IVP5UPrivatePCH.h"

#include "MMDExtendAsset.h"

UMMDExtendFactory::UMMDExtendFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UMMDExtendAsset::StaticClass();
	bCreateNew = true; // for editor create flag
}
bool UMMDExtendFactory::DoesSupportClass(UClass* Class)
{
	return (Class == UMMDExtendAsset::StaticClass());
}
UClass* UMMDExtendFactory::ResolveSupportedClass()
{
	return UMMDExtendAsset::StaticClass();
}

UObject* UMMDExtendFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn)
{
	return NewObject<UMMDExtendAsset>(InParent, InClass, InName, Flags);
}
