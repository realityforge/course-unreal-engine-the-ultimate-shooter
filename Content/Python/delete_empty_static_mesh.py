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


def delete_empty_static_mesh_actors() -> None:
    # Get the libraries
    editor_filter_lib = unreal.EditorFilterLibrary()
    editor_actor_subsystem = unreal.EditorActorSubsystem()

    actors = editor_filter_lib.by_class(editor_actor_subsystem.get_all_level_actors(), unreal.StaticMeshActor)

    deleted_asset_count: int = 0

    for actor in actors:
        # If there was invalid static mesh component then delete the actor
        if None == actor.static_mesh_component.static_mesh:
            actor_name = actor.get_fname()
            actor.destroy_actor()
            deleted_asset_count += 1
            unreal.log(f"The Actor {actor_name} had an invalid Mesh Component and was deleted")

    unreal.log(f"Deleted {deleted_asset_count} actor(s) with invalid an Mesh Component")


delete_empty_static_mesh_actors()
