// Copyright 2015-2026 IVP5U contributors

#pragma once

#include "CoreMinimal.h"

namespace MMDNameUtils
{
	// 替换UE5资产不允许的非法字符为"_"
	// 例如资产名称、骨骼名称、动画曲线名称
	inline FString ReplaceInvalidChars(const FString& InName)
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

	inline bool IsEmptyOrOnlyUnderscoresAndDigits(const FString& InName)
	{
		for (int32 i = 0; i < InName.Len(); ++i)
		{
			const TCHAR Ch = InName[i];
			if (Ch != TEXT('_') && !FChar::IsDigit(Ch))
				return false;
		}
		return true;
	}

	inline FString SanitizeAndDeduplicate(const FString& InName, const FStringView& FallbackPrefix, TSet<FString>& InOutUsedNames)
	{
		FString Name = ReplaceInvalidChars(InName);

		if (IsEmptyOrOnlyUnderscoresAndDigits(Name))
			Name = FallbackPrefix + Name;

		FString UniqueName = Name;
		int32 Suffix = 0;
		while (InOutUsedNames.Contains(UniqueName))
		{
			++Suffix;
			UniqueName = Name + FString::FromInt(Suffix);
		}
		InOutUsedNames.Add(UniqueName);
		return UniqueName;
	}
} // namespace MMDNameUtils
