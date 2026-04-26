#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IVP5USettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class IVP5U_API UIVP5USettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** priority. Default plugins priority=100 */
	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ConfigRestartRequired = false))
	int32 ImportPriority = 80;

	/** Enable physics asset creation */
	UPROPERTY(config, EditAnywhere, Category = Settings)
	bool bCreatePhysicsAsset = false;

	/** Enable material instance creation mode */
	UPROPERTY(config, EditAnywhere, Category = Settings)
	bool bCreateMaterialInstanceMode = true;

	/** create Unreal materials of Unlit Type */
	UPROPERTY(config, EditAnywhere, Category = Settings)
	bool bUnlitMaterials = true;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
