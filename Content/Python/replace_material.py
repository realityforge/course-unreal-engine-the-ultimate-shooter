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


def replace_material(source_material_path: str, target_material_path: str) -> None:
    source_material = unreal.EditorAssetLibrary.load_asset(source_material_path)
    if not source_material:
        print(f"The source material '{source_material_path}' can not be loaded")
        quit()

    target_material = unreal.EditorAssetLibrary.load_asset(target_material_path)
    if not target_material:
        print(f"The target material '{target_material_path}' can't be loaded")
        quit()

    actor_list = unreal.EditorLevelLibrary.get_all_level_actors()
    actor_list = unreal.EditorFilterLibrary.by_class(actor_list, unreal.StaticMeshActor.static_class())
    # noinspection PyTypeChecker
    unreal.EditorLevelLibrary.replace_mesh_components_materials_on_actors(actor_list, source_material, target_material)
