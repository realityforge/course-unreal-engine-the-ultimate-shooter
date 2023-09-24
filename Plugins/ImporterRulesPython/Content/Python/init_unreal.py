import unreal
import traceback

unreal.log("Plugin Importer Rules starting initialization.")

try:
    import Examples.post_import_texture2D_settings2
except Exception as err:
    unreal.log_error("Plugin Importer Rules failed to initialize.")
    traceback.print_exc()
