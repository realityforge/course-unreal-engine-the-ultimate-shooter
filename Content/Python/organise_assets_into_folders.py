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

import os
import unreal


def organise_assets_into_folders() -> None:

    editor_util = unreal.EditorUtilityLibrary()
    system_lib = unreal.SystemLibrary()
    asset_lib = unreal.EditorAssetLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)

    organised_asset_count: int = 0
    unreal.log("Selected {} assets".format(asset_count))

    top_level_dir = r'\Game'

    for asset in selected_assets:
        # Get the name of the assets
        asset_name = system_lib.get_object_name(asset)
        class_name = system_lib.get_class_display_name(asset.get_class())

        # Derive a new name for the asset. Hardcoded and ugly now but could
        # follow better rules in the future
        new_asset_name = os.path.join(top_level_dir, class_name, asset_name)

        result = asset_lib.rename_loaded_asset(asset, new_asset_name)
        if result:
            unreal.log(f"Renamed asset named '{asset_name}' with class {class_name} to '{new_asset_name}'")
            organised_asset_count += 1
        else:
            unreal.log(f"Failed to rename asset named '{asset_name}' with class {class_name} to '{new_asset_name}'")

    unreal.log(f"Organised  #{organised_asset_count} asset(s) into required directories")


organise_assets_into_folders()
