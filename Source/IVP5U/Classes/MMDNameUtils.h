// Copyright 2023 NaN_Name, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

// 替换UE5骨骼名称和Animation Curve不允许使用的非法字符为"_"
inline FString NormalizeBoneAndMorphName(const FString& InName)
{
	// All ASCII punctuation to replace with '_'
	static const TSet<TCHAR> PunctuationSet = {
		' ', '!', '"', '#', '$', '%', '&', '\'',
		'(', ')', '*', '+', ',', '-', '.', '/',
		':', ';', '<', '=', '>', '?', '@',
		'[', '\\', ']', '^', '`', '{', '|', '}', '~'
	};

	FString Result = InName;
	for (TCHAR& Ch : Result)
	{
		if (PunctuationSet.Contains(Ch))
			Ch = TEXT('_');
	}
	return Result;
}