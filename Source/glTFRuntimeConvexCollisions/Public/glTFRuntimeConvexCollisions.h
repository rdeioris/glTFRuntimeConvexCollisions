// Copyright 2020-2022, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FglTFRuntimeConvexCollisionsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	GLTFRUNTIMECONVEXCOLLISIONS_API static void BuildConvexCollisions(class UStaticMesh* StaticMesh, class UglTFRuntimeConvexCollisionConfig* Config);
};
