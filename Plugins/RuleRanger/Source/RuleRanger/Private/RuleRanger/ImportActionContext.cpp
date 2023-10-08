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

#include "ImportActionContext.h"

ERuleRangerActionState UImportActionContext::GetState()
{
    return ActionState;
}

ERuleRangerActionTrigger UImportActionContext::GetActionTrigger()
{
    return ActionTrigger;
}

void UImportActionContext::ResetContext(UObject* InObject, const ERuleRangerActionTrigger InActionTrigger)
{
    check(nullptr != InObject);
    Object = InObject;
    ActionTrigger = InActionTrigger;
    ActionState = ERuleRangerActionState::AS_Success;
}

void UImportActionContext::Info(const FText& InMessage)
{
    InfoMessages.Add(InMessage);
}
void UImportActionContext::Warning(const FText& InMessage)
{
    WarningMessages.Add(InMessage);
    ActionState = ActionState < ERuleRangerActionState::AS_Warning ? ERuleRangerActionState::AS_Warning : ActionState;
}
void UImportActionContext::Error(const FText& InMessage)
{
    ErrorMessages.Add(InMessage);
    ActionState = ActionState < ERuleRangerActionState::AS_Error ? ERuleRangerActionState::AS_Error : ActionState;
}
void UImportActionContext::Fatal(const FText& InMessage)
{
    FatalMessages.Add(InMessage);
    ActionState = ActionState < ERuleRangerActionState::AS_Fatal ? ERuleRangerActionState::AS_Fatal : ActionState;
}
