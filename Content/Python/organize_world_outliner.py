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


def organize_world_outliner(name_mapping: dict[str, str], type_mapping: dict[unreal.Class, str]):
    editor_level_lib = unreal.EditorLevelLibrary()
    editor_filter_lib = unreal.EditorFilterLibrary()

    # get all actors and filter down to specific elements
    actors: unreal.Array[unreal.Actor] = editor_level_lib.get_all_level_actors()

    moved = 0

    for name_sub_string in name_mapping.keys():
        folder_name = name_mapping[name_sub_string]
        matched_actors: unreal.Array[unreal.Actor] = editor_filter_lib.by_id_name(actors, name_sub_string)
        for actor in matched_actors:
            actor_name = actor.get_fname()
            actor.set_folder_path(unreal.Name(folder_name))
            unreal.log(f"Moved {actor_name} into {folder_name}")
            moved += 1

    for actor_type in type_mapping.keys():
        folder_name = type_mapping[actor_type]
        matched_actors: unreal.Array[unreal.Actor] = editor_filter_lib.by_class(actors, actor_type)
        for actor in matched_actors:
            actor_name = actor.get_fname()
            actor.set_folder_path(unreal.Name(folder_name))
            unreal.log(f"Moved {actor_name} into {folder_name}")
            moved += 1

    unreal.log(f"Moved {moved} actors into respective folders")


organize_world_outliner({"BP_": "Blueprints"}, {unreal.StaticMeshActor: "StaticMeshActors",
                                                unreal.ReflectionCapture: "ReflectionCaptures"})
