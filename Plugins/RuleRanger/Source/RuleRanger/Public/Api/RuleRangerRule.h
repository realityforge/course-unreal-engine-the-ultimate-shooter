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
#include "RuleRangerRule.generated.h"

class URuleRangerActionContext;
class URuleRangerAction;
class URuleRangerMatcher;

/**
 * The object that binds one or more matchers with one or more actions.
 */
UCLASS(AutoExpandCategories = ("Default", "Rule Ranger"), Blueprintable, BlueprintType, EditInlineNew)
class RULERANGER_API URuleRangerRule : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * An explanation of the rule.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    FString Description{ TEXT("") };

    /**
     * True to apply this rule when importing an asset initially.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    bool bApplyOnImport{ true };

    /**
     * True to apply this rule when reimporting an asset.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    bool bApplyOnReimport{ true };

    /**
     * True to apply this rule when the asset is validated.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    bool bApplyOnValidate{ true };

    /**
     * True to apply this rule when explicitly requested from the editor UI.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    bool bApplyOnDemand{ true };

    /**
     * A flag that controls whether the presence of an error will result in subsequent actions being skipped if an error
     * is detected.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    bool bContinueOnError{ false };

    /**
     * Priority used to order rules when multiple apply to the same ObjectType.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn))
    int32 Priority{ 1000 };

    /**
     * The names of the object types that this rule is applied to.
     *
     * This is primarily used as an optimisation strategy so that rules can be indexed by object type.
     * If we did not need this optimisation then we could just add a matcher that matches an object type.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn))
    TArray<TSubclassOf<UObject>> ObjectTypes;

    /** The matchers that an object MUST match before this rule is applied. */
    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, E))
    TArray<TObjectPtr<URuleRangerMatcher>> Matchers;

    /** The actions that will be applied if the object is matched by the rule. */
    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn))
    TArray<TObjectPtr<URuleRangerAction>> Actions;

    /**
     * Apply the actions associated with the rule to the specified object.
     *
     * @param ActionContext the context in which the actions are invoked.
     * @param Object the object to apply the actions to.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Rule Ranger")
    void Apply(URuleRangerActionContext* ActionContext, UObject* Object);
};
