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
#include "RuleRanger.h"
#include "ContentBrowserModule.h"
#include "RuleRangerCommands.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMessageLog.h"
#include "RuleRangerStyle.h"

static const FName RuleRangerModuleName = FName(TEXT("RuleRanger"));

// -------------------------------------------------------------------------------------------
// Content browser extensions
// -------------------------------------------------------------------------------------------

static void OnScanSelectedPaths(const TArray<FString>& SelectedPaths)
{
    if (const auto& Module = FModuleManager::GetModulePtr<FRuleRangerModule>(RuleRangerModuleName))
    {
        // TODO: Replace this with dialog to perform scan
        UE_LOG(RuleRanger, Error, TEXT("OnScanSelectedPaths not implemented yet."));
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendContentBrowserForSelectedPaths(FMenuBuilder& MenuBuilder, const TArray<FString> SelectedPaths)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: OnExtendContentBrowser() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan with RuleRanger"),
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan selected paths with RuleRanger"),
        FSlateIcon(FRuleRangerStyle::GetStyleSetName(), FRuleRangerStyle::GetToolbarIconStyleName()),
        FUIAction(FExecuteAction::CreateLambda([SelectedPaths]() { OnScanSelectedPaths(SelectedPaths); })),
        NAME_None,
        EUserInterfaceActionType::Button);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnContentBrowserExtendSelectedPathsMenu(const TArray<FString>& SelectedPaths)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: OnContentBrowserExtendSelectedPathsMenu() invoked."));
    auto Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate =
        FMenuExtensionDelegate::CreateStatic(&OnExtendContentBrowserForSelectedPaths, SelectedPaths);
    Extender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, nullptr, MenuExtensionDelegate);
    return Extender;
}

static void OnScanSelectedAssets(const TArray<FAssetData>& SelectedAssets)
{
    if (const auto& Module = FModuleManager::GetModulePtr<FRuleRangerModule>(RuleRangerModuleName))
    {
        // TODO: Replace this with dialog to perform scan
        UE_LOG(RuleRanger, Error, TEXT("OnScanSelectedAssets not implemented yet."));
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendContentBrowserForSelectedAssets(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: OnExtendAssetSelectionMenu() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan with RuleRanger"),
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan selected paths with RuleRanger"),
        FSlateIcon(FRuleRangerStyle::GetStyleSetName(), FRuleRangerStyle::GetToolbarIconStyleName()),
        FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() { OnScanSelectedAssets(SelectedAssets); })),
        NAME_None,
        EUserInterfaceActionType::Button);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnContentBrowserExtendSelectedAssetsMenu(const TArray<FAssetData>& SelectedAssets)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: OnContentBrowserExtendSelectedAssetsMenu() invoked."));
    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate =
        FMenuExtensionDelegate::CreateStatic(&OnExtendContentBrowserForSelectedAssets, SelectedAssets);
    Extender->AddMenuExtension("CommonAssetActions", EExtensionHook::After, nullptr, MenuExtensionDelegate);
    return Extender;
}

void FRuleRangerModule::RegisterContentBrowserExtensions()
{
    auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: Registering ContentBrowser Extensions."));
    ContentBrowserMenuExtender_SelectedPathsDelegate =
        FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&OnContentBrowserExtendSelectedPathsMenu);
    Module.GetAllPathViewContextMenuExtenders().Add(ContentBrowserMenuExtender_SelectedPathsDelegate);
    ContentBrowserMenuExtender_SelectedPathsDelegateHandle =
        ContentBrowserMenuExtender_SelectedPathsDelegate.GetHandle();
    check(ContentBrowserMenuExtender_SelectedPathsDelegateHandle.IsValid());

    // Asset extenders
    ContentBrowserMenuExtender_SelectedAssetsDelegate =
        FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnContentBrowserExtendSelectedAssetsMenu);
    Module.GetAllAssetViewContextMenuExtenders().Add(ContentBrowserMenuExtender_SelectedAssetsDelegate);
    ContentBrowserMenuExtender_SelectedAssetsDelegateHandle =
        ContentBrowserMenuExtender_SelectedAssetsDelegate.GetHandle();
    check(ContentBrowserMenuExtender_SelectedAssetsDelegateHandle.IsValid());
}

void FRuleRangerModule::DeregisterContentBrowserExtensions() const
{
    auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    if (ContentBrowserMenuExtender_SelectedPathsDelegateHandle.IsValid())
    {
        UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: Deregistering ContentBrowser Extensions."));
        auto Target = ContentBrowserMenuExtender_SelectedPathsDelegateHandle;
        Module.GetAllPathViewContextMenuExtenders().RemoveAll(
            [&Target](const auto& Delegate) { return Delegate.GetHandle() == Target; });
        Target.Reset();
    }
    else
    {
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("RuleRangerModule: Skipping deregister of ContentBrowserExtensions as handle is Invalid."));
    }
}

// -------------------------------------------------------------------------------------------
// Core Module functionality
// -------------------------------------------------------------------------------------------

void FRuleRangerModule::StartupModule()
{
    FRuleRangerMessageLog::Initialize();

    // Add integrations of RuleRanger into Editor UI
    if (!IsRunningCommandlet())
    {
        UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: Initializing Editor integration."));
        // First setup stylesheet

        FRuleRangerStyle::Initialize();
        FRuleRangerStyle::ReloadTextures();

        FRuleRangerCommands::Register();

        RegisterContentBrowserExtensions();
    }
    else
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("RuleRangerModule: Skipping Editor integration as running as a commandlet."));
    }
}

void FRuleRangerModule::ShutdownModule()
{
    if (!IsRunningCommandlet())
    {
        // Remove integrations of RuleRanger into Editor UI
        if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
        {
            DeregisterContentBrowserExtensions();

            FRuleRangerCommands::Unregister();
            FRuleRangerStyle::Shutdown();
        }
    }

    FRuleRangerMessageLog::Shutdown();
}

IMPLEMENT_MODULE(FRuleRangerModule, RuleRanger)

// The implementation of log category top use within plugin
DEFINE_LOG_CATEGORY(RuleRanger);
