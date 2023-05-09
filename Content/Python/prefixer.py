import unreal


def rename_assets_based_on_rules():

    # Get the libraries
    system_lib = unreal.SystemLibrary()
    editor_util = unreal.EditorUtilityLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)

    renamed_asset_count: int = 0
    unreal.log("Selected {} assets".format(asset_count))

    prefixes = {
        "Blueprint": "BP_",
        "StaticMesh": "SM_",
        "Material": "M_",
        "MaterialInstanceConstant": "MI_",
        "MaterialFunction": "MF_",
        "ParticleSystem": "PS_",
        "SoundCue": "A_",
        "SoundWave": "A_",
        "Texture2D": "T_",
        "WidgetBlueprint": "WBP_",
        "MorphTarget": "MT_",
        "SkeletalMesh": "SK_",
        "RenderTarget": "RT_",
        "TextureRenderTarget2D": "TRT_"
    }
    suffixes = {
        "SoundCue": "_Cue",
    }

    for asset in selected_assets:
        # Get the name of the assets
        # asset_name = system_lib.get_object_name(asset)
        asset_name = asset.get_name()
        asset_class = asset.get_class()
        asset_class_name = system_lib.get_class_display_name(asset_class)

        prefix = prefixes.get(asset_class_name, None)
        suffix = suffixes.get(asset_class_name, None)

        if prefix is None and suffix is None:
            unreal.log_warning("No mapping for asset {} of type {}".format(asset_name, asset_class_name))
            continue

        new_asset_name = asset_name
        if prefix and not asset_name.startswith(prefix):
            new_asset_name = prefix + new_asset_name

        if suffix and not asset_name.endswith(suffix):
            new_asset_name = new_asset_name + suffix

        if new_asset_name != asset_name:
            renamed_asset_count += 1
            editor_util.rename_asset(asset, new_asset_name)
            unreal.log("Renamed {} to {}.".format(asset_name, new_asset_name))
        else:
            unreal.log("{} not renamed as it matched convention.".format(asset_name))

    unreal.log("Renamed {} of {} assets.".format(renamed_asset_count, asset_count))

    return renamed_asset_count


rename_assets_based_on_rules()
