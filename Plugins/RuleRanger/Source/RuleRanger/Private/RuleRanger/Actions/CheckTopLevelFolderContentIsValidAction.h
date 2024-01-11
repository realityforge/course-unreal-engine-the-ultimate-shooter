/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RuleRangerAction.h"
#include "CheckTopLevelFolderContentIsValidAction.generated.h"

/**
 * Action to check that content in the top level directory matches the configured rules.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API UCheckTopLevelFolderContentIsValidAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

private:
    /** The custom message to emit if a check fails. */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger",
              meta = (AllowPrivateAccess, ExposeOnSpawn, MultiLine))
    FString Message;
    /** A set of explicitly allowed top level folders. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    TArray<FString> ValidFolderNames;
    /** A regex pattern to match valid folders. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    FString ValidFolderRegexPattern{ TEXT("") };
    /** A set of explicitly allowed top level folders. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    TArray<FString> ValidAssetNames;
    /** A regex pattern to match valid folders. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    FString ValidAssetRegexPattern{ TEXT("") };
    /** A flag controlling whether regex matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger|Regex",
              meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bCaseSensitive{ true };
};
