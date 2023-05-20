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
import math


def verify_power_of_two():
    # Get the libraries
    editor_util = unreal.EditorUtilityLibrary()
    system_lib = unreal.SystemLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)

    invalid_asset_count: int = 0
    unreal.log("Selected {} assets".format(asset_count))

    for asset in selected_assets:
        package_name = asset.get_package().get_name()

        classname = system_lib.get_class_display_name(asset.get_class())
        if 'Texture2D' == classname:
            x_size = asset.blueprint_get_size_x()
            y_size = asset.blueprint_get_size_y()

            is_x_valid = math.log(x_size, 2).is_integer()
            is_y_valid = math.log(y_size, 2).is_integer()

            if not is_x_valid or not is_y_valid:
                unreal.log("{} is not a power of two ({}, {})".format(package_name, x_size, y_size))
                invalid_asset_count += 1

    if 1 == invalid_asset_count:
        unreal.log("1 texture identified that is not a power of two.")
    else:
        unreal.log("{} textures identified that are not a power of two.".format(invalid_asset_count))

    return invalid_asset_count


verify_power_of_two()
