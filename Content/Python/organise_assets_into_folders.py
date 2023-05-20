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
