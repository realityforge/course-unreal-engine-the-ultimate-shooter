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

import json
import os

import realityforge.assetlib.matchers as matchers
import realityforge.assetlib.validators as validators
from realityforge.assetlib.validator import validator_rules_registry, register_validator


def load_rules():
    def r(asset_type: str,
          base_path: str = "",
          enum_name: str = "",
          enum_value: str = "",
          prefix: str = "",
          suffix: str = "",
          notes: str = "",
          priority: int = 100):
        input_locals: dict[str, str] = locals()

        validation_actions = []
        if '' != prefix:
            validation_actions.append(validators.name_prefix(prefix))
        if '' != suffix:
            validation_actions.append(validators.name_suffix(suffix))

        if 0 == len(validation_actions):
            print(f"Rule defined with no validation action: {input_locals}")
            return
        elif 1 == len(validation_actions):
            validator = validation_actions[0]
        else:
            validator = validators.validate_all(validation_actions)

        validation_matchers = []
        if '' != enum_name:
            validation_matchers.append(matchers.editor_property(enum_name, enum_value))

        if '' != base_path:
            validation_matchers.append(matchers.name_prefix(base_path))

        if 0 == len(validation_matchers):
            print(f"Rule defined with no matchers: {input_locals}")
            return
        elif 1 == len(validation_matchers):
            matcher = validation_matchers[0]
        else:
            matcher = matchers.match_all(validation_matchers)

        register_validator(asset_type, matcher, validator, priority)

        # Notes are unused and simply have them here, so we can keep notes in source document
        # noinspection PyStatementEffect
        notes

    # Clear the rules so we do not get duplicates when reloading
    validator_rules_registry.clear()

    # Load the rules from a json file beside this script
    f = open(os.path.join(os.path.dirname(__file__), 'conventions.json'))
    try:
        data = json.load(f)
        for entry in data:
            r(**entry)
    finally:
        f.close()


if __name__ == "__main__":
    load_rules()

# TODO: Make a rule that alerts on Inst suffix? Actually lets just use patterns to identify "old" naming
#  conventions (i.e. SK_Mannequin_PhysicsAsset -> PHYS_Mannequin, SK_Mannequin_Skeleton -> SKEL_Mannequin)
#  and then we can transform them (even if it is renaming as earlier step)
# TODO: Make a rule for handling texture extensions
# TODO: Add configuration of allow list and deny list for naming convention application
# TODO: Add configuration for assets that will be ignored when analyzing
# TODO: Material functions that are Material Layers are prefixed with ML not MF!
# TODO: Retargeters should be named as RTG_[Source]_To_[Target]
# TODO: PoseAsset => PA_ ???

# TODO: Rules to incoporate somehow
rules = """
[
{
    "type": "Level (World Partition)",
    "prefix": "L_",
    "suffix": "_WP"
  },
    {
    "type": "Data Asset",
    "prefix": "DA*_",
    "notes": "Prefix should be based on sub-class type"
  },
  {
    "type": "Texture (Base Color)",
    "prefix": "T_",
    "suffix": "_BC",
    "notes": "Base Color, Albedo, Diffuse"
  },
  {
    "type": "Texture (Ambient Occlusion)",
    "prefix": "T_",
    "suffix": "_AO"
  },
  {
    "type": "Texture (Roughness)",
    "prefix": "T_",
    "suffix": "_R"
  },
  {
    "type": "Texture (Specular)",
    "prefix": "T_",
    "suffix": "_S"
  },
  {
    "type": "Texture (Metallic)",
    "prefix": "T_",
    "suffix": "_M"
  },
  {
    "type": "Texture (Normal)",
    "prefix": "T_",
    "suffix": "_N"
  },
  {
    "type": "Texture (Emissive)",
    "prefix": "T_",
    "suffix": "_E"
  },
  {
    "type": "Texture (Mask)",
    "prefix": "T_",
    "suffix": "_Mask"
  },
  {
    "type": "Texture (Flow Map)",
    "prefix": "T_",
    "suffix": "_F"
  },
  {
    "type": "Texture (Height)",
    "prefix": "T_",
    "suffix": "_H"
  },
  {
    "type": "Texture (Displacement)",
    "prefix": "T_",
    "suffix": "_D"
  },
  {
    "type": "Texture (Light Map)",
    "prefix": "T_",
    "suffix": "_L"
  },
  {
    "type": "Texture (Alpha/Opacity)",
    "prefix": "T_",
    "suffix": "_A"
  },
  {
    "type": "Texture (Packed)",
    "prefix": "T_",
    "suffix": "_*",
    "notes": "Use texture type in RGBA order. e.g., ARMH (Ambient Occlusion, Roughness, Metallic, Height)"
  }
]
"""
