import json
import os

# Load the rules from a json file beside this script
f = open(os.path.join(os.path.dirname(__file__), 'conventions.json'))
try:
    data = json.load(f)
    for entry in data:
        print(f"{entry['asset_type']}{entry.get('enum_value') or ''},\"/Script/Engine.{entry['asset_type']}\",\"{entry.get('enum_value') or ''}\",\"{entry.get('prefix') or ''}\",\"{entry.get('suffix') or ''}\"")
finally:
    f.close()
