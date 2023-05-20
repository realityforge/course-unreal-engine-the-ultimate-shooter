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


def duplicate_assets(duplicate_count) -> None:
    """
    Duplicate the selected assets duplicate_count times.

            Parameters:
                    :param duplicate_count: The number of duplicates to create.
    """

    # Get the libraries
    system_lib = unreal.SystemLibrary()
    editor_util = unreal.EditorUtilityLibrary()
    asset_util = unreal.EditorAssetLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)

    assets_created: int = 0
    unreal.log("Selected {} assets".format(asset_count))

    expected_asset_count = len(selected_assets) * duplicate_count

    with unreal.ScopedSlowTask(expected_asset_count, 'Duplicating Assets') as slow_task:

        # Make the dialog visible
        slow_task.make_dialog(True)

        for asset in selected_assets:
            # Get the name of the assets
            asset_name = system_lib.get_object_name(asset)

            asset_path = asset.get_package().get_name()

            local_assets_created: int = 0
            for i in range(1, duplicate_count + 1):

                # If user has requested operation cancelled then exit to main loop
                if slow_task.should_cancel():
                    break

                new_asset_name = f"{asset_path}_{i}"
                result = asset_util.duplicate_loaded_asset(asset, new_asset_name)
                slow_task.enter_progress_frame(1)
                if result:
                    assets_created += 1
                    local_assets_created += 1
                else:
                    unreal.log(f"Asset named {asset_name} already has a duplicate {new_asset_name}.")

            unreal.log(f"Asset named {asset_name} was duplicated {local_assets_created} times.")

            # If user has requested operation cancelled then exit out of operation
            if slow_task.should_cancel():
                break

    unreal.log(f"Total number of assets created was {assets_created}.")


duplicate_assets(5)
