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
import re
import json
import pathlib
from typing import Optional

def find_unreferenced_assets(report_path: str, base_path: str, entry_points: set[str], **kwargs: dict[str, str]) -> [
    str]:
    editor_asset = unreal.EditorAssetLibrary()
    editor_actor_lib = unreal.EditorActorSubsystem()
    asset_registry = unreal.AssetRegistryHelpers().get_asset_registry()

    options = unreal.AssetRegistryDependencyOptions()
    options.include_searchable_names = True
    options.include_hard_management_references = True
    options.include_hard_package_references = True
    options.include_soft_management_references = True
    options.include_soft_package_references = True

    referencer_cache_file: Optional[str] = kwargs["referencer_cache_file"] if "referencer_cache_file" in kwargs else None
    dependencies_cache_file: Optional[str] = kwargs["dependencies_cache_file"] if "dependencies_cache_file" in kwargs else None
    actors_cache_file: Optional[str] = kwargs["actors_cache_file"] if "actors_cache_file" in kwargs else None
    extra_unused: [str] = kwargs["extra_unused"] if "extra_unused" in kwargs else []

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

            asset = editor_asset.load_asset(asset_path)
            if not asset:
                unreal.log_warning(f"\n\n\n\nUUR: Unable to load asset: {asset_path}\n\n\n\n")
                continue
            else:
                dependencies = asset_registry.get_dependencies(unreal.Name(asset.get_package().get_path_name()),
                                                               options)
                if asset_path not in asset_to_dependencies:
                    asset_to_dependencies[asset_path] = set()
                for dependency in dependencies:
                    asset_reference = str(dependency)
                    asset_to_dependencies[asset_path].add(asset_reference)

                referencers = asset_registry.get_referencers(unreal.Name(asset.get_package().get_path_name()), options)
                if asset_path not in asset_to_referencers:
                    asset_to_referencers[asset_path] = set()
                for asset_reference in referencers:
                    asset_reference = str(asset_reference)
                    asset_to_referencers[asset_path].add(asset_reference)

            slow_task.enter_progress_frame(1, f"🔨 : {asset_path}")

    actor_classnames = set()

    # Assume all actors in the current level are entry points we care about
    actors = editor_actor_lib.get_all_level_actors()
    for actor in actors:
        actor_class_name = actor.get_class().get_path_name()
        actor_classnames.add(actor_class_name[:actor_class_name.rindex('.')])

    if referencer_cache_file:
        with open(referencer_cache_file, 'w') as f:
            output = {}
            for asset_name, referencers in asset_to_referencers.items():
                output[asset_name] = list(referencers)
            f.write(json.dumps(output, indent=2))
    if dependencies_cache_file:
        with open(dependencies_cache_file, 'w') as f:
            output = {}
            for asset_name, dependencies in asset_to_dependencies.items():
                output[asset_name] = list(dependencies)
            f.write(json.dumps(output, indent=2))
    if actors_cache_file:
        with open(actors_cache_file, 'w') as f:
            f.write(json.dumps(list(actor_classnames), indent=2))

    return generate_report(referencer_cache_file,
                           dependencies_cache_file,
                           actors_cache_file,
                           extra_unused,
                           report_path,
                           base_path,
                           entry_points)


def generate_report(referencer_cache_file: str,
                    dependencies_cache_file: str,
                    actors_cache_file: str,
                    extra_unused: [str],
                    report_path: str,
                    base_path: str,
                    entry_points: set[str]) -> [str]:
    asset_to_referencers = {}
    asset_to_dependencies = {}
    with open(referencer_cache_file, 'r') as f:
        contents = "".join(f.readlines())
        inputs: dict[str, [str]] = json.loads(contents)
        for asset_name, referencers in inputs.items():
            asset_to_referencers[asset_name] = set(referencers)
    with open(dependencies_cache_file, 'r') as f:
        contents = "".join(f.readlines())
        inputs: dict[str, [str]] = json.loads(contents)
        for asset_name, dependencies in inputs.items():
            asset_to_dependencies[asset_name] = set(dependencies)
    with open(actors_cache_file, 'r') as f:
        contents = "".join(f.readlines())
        actor_classnames = set(json.loads(contents))

    unreferenced = calculate_unused_asset_paths(actor_classnames,
                                                asset_to_dependencies.keys(),
                                                asset_to_referencers,
                                                extra_unused,
                                                entry_points)

    with open(report_path, 'w') as f:
        for asset_name in sorted(unreferenced):
            if asset_name.startswith(base_path):
                f.write(f'{asset_name}\n')

    return unreferenced


def calculate_unused_asset_paths(actor_classnames: [str],
                                 asset_names: [str],
                                 asset_to_referencers: dict[str, [str]],
                                 extra_unused: [str],
                                 entry_points: set[str]) -> [str]:
    unreferenced_asset_paths = set(extra_unused)
    used_asset_paths = set()

    local_entry_points = set()
    pattern_entry_points = set()

    for entry_point in entry_points:
        if '*' in entry_point:
            pattern_entry_points.add(entry_point)
        else:
            local_entry_points.add(entry_point)

    for actor_classname in actor_classnames:
        local_entry_points.add(actor_classname)

    with unreal.ScopedSlowTask(len(asset_names), 'Tracing dependencies') as slow_task:
        # Make the dialog visible
        slow_task.make_dialog(True)

        change_made = True

        while change_made:
            unreal.log("Tracing dependencies iteration...")
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
                    print(f"Found Local Entry Point: {asset_path} ")
                else:
                    pattern_matched = False
                    for pattern in pattern_entry_points:
                        if not pattern_matched and re.search(pattern, asset_path):
                            # If asset is a known entrypoint then skip it
                            used_asset_paths.add(asset_path)
                            change_made = True
                            pattern_matched = True
                            print(f"Found Entry Point via pattern {pattern}: {asset_path} ")

                    # if we did not match a pattern, and we have no references then mark as unreferenced
                    if not pattern_matched and 0 == len(asset_referencers):
                        unreferenced_asset_paths.add(asset_path)
                        print(f"No referencers: {asset_path} ")
                        change_made = True
                    else:
                        for asset_referencer in asset_referencers:
                            if asset_referencer in used_asset_paths:
                                # This asset is referenced by an asset that is used then mark it as used
                                if not asset_path in used_asset_paths:
                                    used_asset_paths.add(asset_path)
                                    print(f"Referencer {asset_referencer} used so marking as used: {asset_path} ")
                                    change_made = True
                                    continue
    # For any asset that was not marked as used or unused then they have no explicit dependencies
    # from anything marked as explicitly used so mark them as unused
    for asset_path in sorted(asset_to_referencers.keys()):
        if asset_path not in unreferenced_asset_paths and asset_path not in used_asset_paths:
            unreferenced_asset_paths.add(asset_path)
            print(f"Reference neither used nor unused. Marking as unused: {asset_path} ")
    return unreferenced_asset_paths


def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))


def generate_clean_script(unused_assets_file: str,
                          extra_unused_assets_file: str,
                          filter_repo_script: str,
                          clean_script: str,
                          keep_prefix: str):
    output = ''

    files = []
    with open(unused_assets_file, 'r') as f:
        files.extend(f.readlines())

    with open(extra_unused_assets_file, 'r') as f:
        files.extend(f.readlines())

    for group in chunker(files, 30):
        args = ['python', filter_repo_script]
        for f in group:
            if f.startswith(keep_prefix):
                continue
            f = f.rstrip()
            relative_filename = f.replace('/Game', 'Content')
            if '' == pathlib.Path(relative_filename).suffix:
                relative_filename += '.uasset'
            args.append('--path')
            args.append(relative_filename)

        args.append('--invert-paths')
        args.append('--force')
        output += ' '.join(args) + '\n'
        output += 'if %errorlevel% neq 0 exit /b %errorlevel%\n'

    with open(clean_script, 'w') as f:
        f.write(output)


if __name__ == "__main__":
    _report_path = r'C:\Projects\Shooter\tmp\UnusedAssets.txt'
    _clean_script = r'C:\Projects\ShooterClean\clean.bat'
    _extra_unused_assets_file = r'C:\Projects\Shooter\Content\Python\maps_assets_to_remove.txt'
    _filter_repo_script = r'C:\Projects\Shooter\Content\Python\git-filter-repo.py'
    _base_path = "/Game/"
    _keep_prefix = '/Game/_Game'
    _entry_points = {'/Game/_Game/Maps/BasicMap',
                    '^/Game/_Game/Character/Rigs/.*$',
                    '^/Game/_Game/Character/Animations/Crouching/MixamoCharacter/.*$',
                    '^/Game/_Game/Automation/.*$',
                    '/Game/_Game/Character/TwinBlastCharacterBP',
                    '/Game/_Game/GameMode/PelorGameModeBaseBP'}
    _actors_cache_file = r"C:\Projects\Shooter\tmp\actor_classnames.json"
    _referencer_cache_file = r"C:\Projects\Shooter\tmp\actor_referencers.json"
    _dependencies_cache_file = r"C:\Projects\Shooter\tmp\actor_dependencies.json"
    _extra_unused = ['Content/A_Surface_Footstep/Environment_Assets/SM_Scapes1.fbx',
                    'Content/A_Surface_Footstep/Niagara_FX/Textures/ParticleFlamesSheet.tga',
                    'Content/A_Surface_Footstep/Niagara_FX/Textures/T_FireSheet.tga',
                    'Content/A_Surface_Footstep/Niagara_FX/Textures/T_FireSheet2.tga',
                    'Content/A_Surface_Footstep/Niagara_FX/Textures/T_FireSheet3.tga',
                    'Content/A_Surface_Footstep/Niagara_FX/Textures/T_FireSheet4.tga',
                    'Content/A_Surface_Footstep/Surface_FootstepFX_DemoMap.umap',
                    'Content/ParagonGrux/Placeholder.txt']

    # find_unreferenced_assets(_report_path,
    #                          _base_path,
    #                          _entry_points,
    #                          extra_unused=_extra_unused,
    #                          referencer_cache_file=_referencer_cache_file,
    #                          dependencies_cache_file=_dependencies_cache_file,
    #                          actors_cache_file=_actors_cache_file)

    # generate_report(_referencer_cache_file,
    #                 _dependencies_cache_file,
    #                 _actors_cache_file,
    #                 _extra_unused,
    #                 _report_path,
    #                 _base_path,
    #                 _entry_points)
    generate_clean_script(_report_path, _extra_unused_assets_file, _filter_repo_script, _clean_script, _keep_prefix)
