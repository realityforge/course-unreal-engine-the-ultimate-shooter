import unreal


def rename_assets(search_pattern, replace_pattern, case_sensitive):
    '''
    Renames selected assets if they have search_pattern in name and replaces it with replace_pattern and does it in
    a case-sensitive way based on case_sensitive parameter.

            Parameters:
                    :param search_pattern: the pattern to match an asset as a candidate for renaming.
                    :param replace_pattern: the text to use to replace search pattern if matched.
                    :param case_sensitive: True if you want the matching to be case sensitive, false otherwise.

            Returns:
                    :return renamed_asset_count: The number of assets renamed.
    '''

    # Get the libraries
    system_lib = unreal.SystemLibrary()
    editor_util = unreal.EditorUtilityLibrary()
    string_lib = unreal.StringLibrary()

    selected_assets = editor_util.get_selected_assets()
    asset_count = len(selected_assets)

    renamed_asset_count: int = 0
    unreal.log("Selected {} assets".format(asset_count))

    for asset in selected_assets:
        # Get the name of the assets
        asset_name = system_lib.get_object_name(asset)

        # Check if the asset name contains the to-be-replaced text
        if string_lib.contains(asset_name, search_pattern, use_case=case_sensitive):
            search_case = unreal.SearchCase.CASE_SENSITIVE if case_sensitive else unreal.SearchCase.IGNORE_CASE
            new_asset_name = string_lib.replace(asset_name, search_pattern, replace_pattern, search_case=search_case)
            editor_util.rename_asset(asset, new_asset_name)
            renamed_asset_count += 1
            unreal.log("Renamed {} to {}.".format(asset_name, new_asset_name))
        else:
            unreal.log("{} did not match the search pattern, skipping rename operation.".format(asset_name))

    unreal.log("Rename {} of {} assets.".format(renamed_asset_count, asset_count))

    return renamed_asset_count
