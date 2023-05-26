# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unreal
import os
import re


# TODO: Also add a variant that scans all assets and loads all of them amd finds an unused list.
#       As part of processing we should have a graph of all the assets and dependencies. Start at
#       unused leaf and then test if any dependencies of unused were only used by unused. Repeat
#       until no round produces any more unused and then produce a list of unused

# TODO: Also should we have script that adds metadata (i.e. Channel 1 is ambient occlusion, Channel 3 is Metallic, Channel 2 is Roughness for textures)
#       and then drives asset names and other tools from metadata

def find_unreferenced_assets(report_path: str, base_path: str, entry_points: set[str],
                             emit_full_report: bool = False) -> [str]:
    editor_asset = unreal.EditorAssetLibrary()
    editor_actor_lib = unreal.EditorActorSubsystem()
    asset_registry = unreal.AssetRegistryHelpers().get_asset_registry()

    options = unreal.AssetRegistryDependencyOptions()
    options.include_searchable_names = True
    options.include_hard_management_references = True
    options.include_hard_package_references = True
    options.include_soft_management_references = True
    options.include_soft_package_references = True

    asset_names = editor_asset.list_assets(base_path)

    asset_to_referencers: dict[str, set[str]] = {}
    asset_to_dependencies: dict[str, set[str]] = {}

    # This section will build up a map of assets to their dependencies and assets to their users
    with unreal.ScopedSlowTask(len(asset_names), 'Calculating Top-Level Unused Assets') as slow_task:
        # Make the dialog visible
        slow_task.make_dialog(True)

        for asset_name in asset_names:
            # If user has requested operation cancelled then abort
            if slow_task.should_cancel():
                break

            asset_path = asset_name[:asset_name.rindex('.')]

            # Find any "referencers" of the asset. The referencer and asset will be loaded to verify this
            asset_references = editor_asset.find_package_referencers_for_asset(asset_path, True)

            if asset_path not in asset_to_referencers:
                asset_to_referencers[asset_path] = set()

            for asset_reference in asset_references:
                asset_to_referencers[asset_path].add(asset_reference)
                if asset_reference not in asset_to_dependencies:
                    asset_to_dependencies[asset_reference] = set()
                asset_to_dependencies[asset_reference].add(asset_path)

            asset = editor_asset.load_asset(asset_path)

            if asset_path not in asset_to_dependencies:
                asset_to_dependencies[asset_path] = set()

            dependencies = asset_registry.get_dependencies(unreal.Name(asset.get_package().get_path_name()), options)
            for dependency in dependencies:
                asset_reference = str(dependency)
                asset_to_dependencies[asset_path].add(asset_reference)
                if asset_reference not in asset_to_referencers:
                    asset_to_referencers[asset_reference] = set()
                asset_to_referencers[asset_reference].add(asset_path)

            referencers = asset_registry.get_referencers(unreal.Name(asset.get_package().get_path_name()), options)
            for asset_reference in referencers:
                asset_reference = str(asset_reference)
                if asset_reference not in asset_to_referencers:
                    asset_to_referencers[asset_reference] = set()
                asset_to_referencers[asset_reference].add(asset_path)

            slow_task.enter_progress_frame(1, f"🔨 : {asset_path}")

    unreferenced_asset_paths = set()
    used_asset_paths = set()

    local_entry_points = set()
    pattern_entry_points = set()
    for entry_point in entry_points:
        if '*' in entry_point:
            pattern_entry_points.add(entry_point)
        else:
            local_entry_points.add(entry_point)

    # Assume all actors in the current level are entry points we care about
    actors = editor_actor_lib.get_all_level_actors()
    for actor in actors:
        actor_class_name = actor.get_class().get_path_name()
        local_entry_points.add(actor_class_name[:actor_class_name.rindex('.')])

    with unreal.ScopedSlowTask(len(asset_names), 'Tracing dependencies') as slow_task:
        # Make the dialog visible
        slow_task.make_dialog(True)

        change_made = True

        while change_made:
            change_made = False

            for asset_path in sorted(asset_to_referencers.keys()):
                asset_referencers = asset_to_referencers[asset_path]

                if asset_path in unreferenced_asset_paths or asset_path in used_asset_paths:
                    # If we have already made a decision about this asset then skip it
                    pass
                elif asset_path in local_entry_points:
                    # If asset is a known entrypoint then skip it
                    used_asset_paths.add(asset_path)
                    change_made = True
                else:
                    for pattern in pattern_entry_points:
                        if re.search(pattern, asset_path):
                            # If asset is a known entrypoint then skip it
                            used_asset_paths.add(asset_path)
                            change_made = True

                    # if we did not match a pattern, and we have no references then mark as unreferenced
                    if not change_made and 0 == len(asset_referencers):
                        unreferenced_asset_paths.add(asset_path)
                        change_made = True
                    else:
                        for asset_referencer in asset_referencers:
                            if asset_referencer in used_asset_paths:
                                # This asset is referenced by an asset that is used then mark it as used
                                used_asset_paths.add(asset_path)
                                change_made = True

    # For any asset that was not marked as used or unused then they have no explicit dependencies
    # from anything marked as explicitly used so mark them as unused
    for asset_path in sorted(asset_to_referencers.keys()):
        if asset_path not in unreferenced_asset_paths and asset_path not in used_asset_paths:
            unreferenced_asset_paths.add(asset_path)

    with open(report_path, 'w') as f:
        if emit_full_report:
            f.write('# Entrypoint\n')
            for entry_point in sorted(local_entry_points):
                f.write(f'{entry_point} FromCurrentMap={entry_point not in entry_points}\n')
            f.write('\n\n\n# Asset -> Dependencies\n')
            for asset_name in sorted(asset_to_dependencies.keys()):
                if asset_name.startswith(base_path):
                    f.write(f'{asset_name}={asset_to_dependencies[asset_name]}\n')
            f.write('\n\n\n# Asset -> Referencers\n')
            for asset_name in sorted(asset_to_referencers.keys()):
                if asset_name.startswith(base_path):
                    f.write(f'{asset_name}={asset_to_referencers[asset_name]}\n')
            f.write('\n\n\n# Unreferenced Assets\n')
        for asset_name in sorted(unreferenced_asset_paths):
            if asset_name.startswith(base_path):
                f.write(f'{asset_name}\n')

    return unreferenced_asset_paths


if __name__ == "__main__":
    find_unreferenced_assets(r"C:\Projects\Shooter\tmp\UnusedAssets.txt",
                             "/Game/_Game/",
                             {'/Game/_Game/Maps/BasicMap',
                              '^/Game/_Game/Character/Rigs/.*$',
                              '^/Game/_Game/Character/Animations/Crouching/MixamoCharacter/.*$',
                              '^/Game/_Game/Automation/.*$',
                              '/Game/_Game/Character/TwinBlastCharacterBP',
                              '/Game/_Game/GameMode/PelorGameModeBaseBP'})
