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

from typing import Optional

import unreal
import json
import os


# Note: This approach was originally inspired by https://unrealcommunity.wiki/revisions/6100e8169c9d1a89e0c3456a
#       while the conventions were mostly derived from https://www.unrealdirective.com/resource/asset-naming-conventions


# Structure that is used by the NamingEditorValidator and stored as a property on that object
# The structure needs to be a "ustruct" as the property must be a uproperty (this needs to be
# so as unreal will "Re-instance" the validator after it is registered which means we lost any
# state on object and the unreal runtime guards against re-adding state as arbitrary fields
# unless it goes through their object model.
@unreal.ustruct()
class NamingRule(unreal.StructBase):
    base_path = unreal.uproperty(str)
    asset_type = unreal.uproperty(str)
    enum_name = unreal.uproperty(str)
    enum_value = unreal.uproperty(str)
    prefix = unreal.uproperty(str)
    suffix = unreal.uproperty(str)
    priority = unreal.uproperty(int)

    def describe_asset_non_conformance(self, asset: unreal.Object) -> Optional[str]:
        asset_name = asset.get_name()
        if "" != self.prefix and not asset_name.startswith(self.prefix):
            return f"Asset name expected to start with the prefix '{self.prefix}'"
        elif "" != self.suffix and not asset_name.endswith(self.suffix):
            return f"Asset name expected to end with the suffix '{self.suffix}'"
        else:
            return None

    def does_rule_match_asset(self, asset: unreal.Object):
        if "" != self.base_path and not asset.get_path_name().startswith(self.base_path):
            return False
        elif "" == self.enum_name:
            return True
        else:
            current_asset = asset
            while current_asset:
                try:
                    if current_asset.get_editor_property(self.enum_name) == self.enum_value:
                        return False
                except Exception:
                    # If we get here then the editor property does not exist on asset ... check parent assets if any
                    pass

                # Check parent object (this is mostly for things that have instances like MaterialInstances that
                # have a chain between the Instance and the actual. If any of these hierarchies use an editor property
                # other than "Parent" to represent parent hierarchy then we will have to make this configurable.
                # This currently works for Materials
                try:
                    current_asset = current_asset.get_editor_property("Parent")
                except:
                    current_asset = None

            return False


# A simple class to make lookup slightly faster rather than a linear array scan
class NamingRuleIndex:
    def __init__(self):
        self.rules: dict[str, list[NamingRule]] = {}

    def add(self, rule: NamingRule) -> None:
        if rule.asset_type in self.rules:
            self.rules[rule.asset_type].append(rule)
        else:
            self.rules[rule.asset_type] = [rule]

    def find_matching_rule(self, asset: unreal.Object) -> Optional[NamingRule]:
        rules = []
        # Collect all the rules for the class type
        clazz = asset.get_class()
        self.add_rules_for_class_hierarchy(asset, clazz, rules)

        try:
            blueprint = unreal.Blueprint.cast(asset)
            blueprint_parent_class = unreal.MetaDataAccessLibrary.get_blueprint_parent_class(blueprint)
            self.add_rules_for_class_hierarchy(asset, blueprint_parent_class, rules)
        except:
            # Class is not a blueprint and cast failed ... probably
            pass

        rules.sort(key=lambda x: x.priority)
        # print(f"Candidate rules: {rules}")
        if 0 == len(rules):
            return None
        else:
            return rules[0]

    def add_rules_for_class_hierarchy(self, asset, clazz, rules):
        while clazz:
            self.add_rules_for_class(asset, clazz.get_name(), rules)
            clazz = unreal.MetaDataAccessLibrary.get_parent_class(clazz)

    def add_rules_for_class(self, asset, class_name, rules):
        if class_name in self.rules:
            for rule in self.rules[class_name]:
                if rule.does_rule_match_asset(asset):
                    rules.append(rule)


# The validator itself
@unreal.uclass()
class NamingEditorValidator(unreal.EditorValidatorBase):
    # The uproperty field used to store the set of rules that we apply
    rules = unreal.uproperty(unreal.Array(NamingRule))

    def __init__(self, rules: list[NamingRule], *args, **kargs):
        super(NamingEditorValidator, self).__init__(*args, **kargs)
        self.rules = rules
        self.index = None

    @unreal.ufunction(override=True)
    def validate_loaded_asset(self, asset: unreal.Object, validation_errors):
        # print("NamingEditorValidator.validate_loaded_asset....")

        if not hasattr(self, "index"):
            self.index = NamingRuleIndex()
            for rule in self.rules:
                self.index.add(rule)

        rule = self.index.find_matching_rule(asset)
        # print(f"Rule detected for asset of type {asset.get_class().get_fname()}: {rule}")

        if rule:
            non_conformance_message = rule.describe_asset_non_conformance(asset)
            if non_conformance_message is None:
                return unreal.DataValidationResult.VALID, validation_errors
            else:
                validation_errors = self.asset_fails(asset,
                                                     unreal.Text.cast(non_conformance_message),
                                                     validation_errors)
                return unreal.DataValidationResult.INVALID, validation_errors
        else:
            self.asset_warning(asset,
                               unreal.Text.cast(f"Asset validator missing rules for the "
                                                f"asset type: {asset.get_class().get_fname()}"))
            return unreal.DataValidationResult.NOT_VALIDATED, validation_errors

    @unreal.ufunction(override=True)
    def can_validate_asset(self, asset):
        return True


# Unlike EditorValidatorBase subclasses in C++, which are registered automatically,
# EditorValidatorBase subclasses in Python must be registered manually:
def register_validator():
    def r(rules: list[NamingRule],
          asset_type: str,
          base_path: str = "/Game/",
          enum_name: str = "",
          enum_value: str = "",
          prefix: str = "",
          suffix: str = "",
          notes: str = "",
          priority: int = 100):
        rule = NamingRule()
        rule.base_path = base_path
        rule.asset_type = asset_type
        rule.enum_name = enum_name
        rule.enum_value = enum_value
        rule.prefix = prefix
        rule.suffix = suffix
        rule.priority = priority

        # Notes are unused and simply have them here so we can keep notes in source document
        # noinspection PyStatementEffect
        notes
        rules.append(rule)

    _rules = list()

    # Load the rules from a json file beside this script
    f = open(os.path.join(os.path.dirname(__file__), 'conventions.json'))
    try:
        data = json.load(f)
        for entry in data:
            r(_rules, **entry)
    finally:
        f.close()

    # Register the validator
    unreal.get_editor_subsystem(unreal.EditorValidatorSubsystem).add_validator(NamingEditorValidator(_rules))


if __name__ == "__main__":
    print("Registering Python NamingEditorValidator")
    register_validator()

# TODO: Make a rule that alerts on Inst suffix?
# TODO: Make a rule for handling texture extensions
# TODO: Make the rule auto applying ... via blueprint hook?
# TODO: Make the rules be able to be stored into a DataTable?
