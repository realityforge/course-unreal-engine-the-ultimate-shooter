// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MetaDataAccessLibrary.generated.h"

/**
 * A simple library for Blueprints and scripting designed to expose properties that are "protected".
 * These are however useful to have when scripting within the editor.
 */
UCLASS(meta = (ScriptName = "MetaDataAccessLibrary"))
class PELOR_API UMetaDataAccessLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintPure, Category = Utility)
    static UClass* GetParentClass(const UClass* Class);

    UFUNCTION(BlueprintPure, Category = Utility)
    static UClass* GetBlueprintParentClass(const UBlueprint* Blueprint);
};
