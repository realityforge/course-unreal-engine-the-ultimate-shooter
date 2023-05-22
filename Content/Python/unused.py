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


def remove_unused(instant_delete=True, trash_folder="/Game/Trash"):
    editor_util = unreal.EditorUtilityLibrary()
    editor_asset = unreal.EditorAssetLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)
    removed = 0

    to_delete = []

    for asset in selected_assets:
        # get the full path to the to be duplicated asset
        asset_path = editor_asset.get_path_name_for_loaded_asset(asset)

        # get a list of references for this asset
        asset_references = editor_asset.find_package_referencers_for_asset(asset_path, True)

        if 0 == len(asset_references):
            to_delete.append(asset)

    for asset in to_delete:
        asset_name = asset.get_package().get_name()

        if instant_delete:
            # instantly delete the assets
            if not editor_asset.delete_loaded_asset(asset):
                unreal.log_warning(f"Asset {asset_name} could not be deleted")
            else:
                removed += 1
        else:
            # move the assets to the trash folder
            new_path = f"{trash_folder}/{str(asset_name)}"
            unreal.log(f"Move {str(asset)} to {new_path}")
            editor_asset.make_directory(os.path.dirname(new_path))
            if not editor_asset.rename_loaded_asset(asset, new_path):
                unreal.log_warning(f"Asset {asset_name} could not be moved to Trash")
            else:
                removed += 1

    unreal.log(f"{removed} of {len(to_delete)} to be deleted assets, of {asset_count} selected, removed")


remove_unused(instant_delete=False, trash_folder="/Game/Trash")
