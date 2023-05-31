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


def delete_empty_folders(source_dir: str, include_subfolders: bool = True) -> None:
    # instances of unreal classes
    editor_asset_lib = unreal.EditorAssetLibrary()

    deleted_folder_count: int = 0

    # get all assets in source dir
    assets = editor_asset_lib.list_assets(source_dir, recursive=include_subfolders, include_folder=True)

    # Created filtered list of folders
    folders = [asset for asset in assets if editor_asset_lib.does_directory_exist(asset)]

    for folder in folders:
        if not editor_asset_lib.does_directory_have_assets(folder):
            editor_asset_lib.delete_directory(folder)
            deleted_folder_count += 1
            unreal.log(f"Empty folder {folder} was deleted")

    unreal.log(f"Deleted {deleted_folder_count} empty folders")


delete_empty_folders("/Game/Test")
