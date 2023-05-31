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


def set_texture_patterns(source_directory: str, patterns: [str], include_subfolders: bool = True):
    editor_asset_lib = unreal.EditorAssetLibrary()
    string_lib = unreal.StringLibrary()

    textures_matched_count: int = 0

    assets = editor_asset_lib.list_assets(source_directory, recursive=include_subfolders)
    for asset in assets:
        # for every asset check it against all the patterns
        for pattern in patterns:
            if string_lib.contains(asset, pattern):
                # load the asset, turn off sRGB and set compression settings to Mask
                asset_obj = editor_asset_lib.load_asset(asset)
                asset_obj.set_editor_property("sRGB", False)
                asset_obj.set_editor_property("CompressionSettings", unreal.TextureCompressionSettings.TC_MASKS)

                unreal.log(f"Setting TC_Masks and turning off sRGB for asset {asset}")
                textures_matched_count += 1
                break

    unreal.log(f"Linear color for matching textures set for {textures_matched_count} assets")


set_texture_patterns("/Game/Test/", ["_ORM", "_OcclusionRoughnessMetallic", "_Metallic", "_Roughness", "_MSK"])
