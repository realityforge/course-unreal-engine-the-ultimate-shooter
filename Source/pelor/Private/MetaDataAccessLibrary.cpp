// Fill out your copyright notice in the Description page of Project Settings.

#include "MetaDataAccessLibrary.h"

UClass* UMetaDataAccessLibrary::GetParentClass(const UClass* Class)
{
    return Class->GetSuperClass();
}

UClass* UMetaDataAccessLibrary::GetBlueprintParentClass(const UBlueprint* Blueprint)
{
    return Blueprint->ParentClass;
}
