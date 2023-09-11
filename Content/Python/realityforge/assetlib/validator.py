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

from realityforge.assetlib.api import MatcherBase, ValidatorBase


class ValidatorRule:
    def __init__(self, asset_type: str, matcher: MatcherBase, validator: ValidatorBase, priority: int):
        self.asset_type = asset_type
        self.matcher = matcher
        self.validator = validator
        self.priority = priority


class ValidatorRulesRegistry:
    """Class to manage validator rules."""

    def __init__(self) -> None:
        self.rules: dict[str, list[ValidatorRule]] = {}

    def clear(self):
        self.rules.clear()

    def add(self, rule: ValidatorRule) -> None:
        if rule.asset_type in self.rules:
            self.rules[rule.asset_type].append(rule)
        else:
            self.rules[rule.asset_type] = [rule]

    def find_matching_rule(self, asset: unreal.Object) -> Optional[ValidatorRule]:
        rules = []
        # Collect all the rules for the class type
        clazz = asset.get_class()
        self.add_rules_for_class_hierarchy(asset, clazz, rules)

        if isinstance(asset, unreal.Blueprint):
            blueprint = unreal.Blueprint.cast(asset)
            blueprint_parent_class = unreal.MetaDataAccessLibrary.get_blueprint_parent_class(blueprint)
            self.add_rules_for_class_hierarchy(asset, blueprint_parent_class, rules)

        rules.sort(key=lambda x: x.priority)
        # print(f"Candidate rules: {rules}")
        if 0 == len(rules):
            return None
        else:
            # TODO: Should some rules always apply and others only apply if no other higher priority?
            #       Perhaps there should be rule "group" and only one in each "group" applied based on
            #       priority
            return rules[0]

    def add_rules_for_class_hierarchy(self, asset, clazz, rules):
        while clazz:
            self.add_rules_for_class(asset, clazz.get_name(), rules)
            clazz = unreal.MetaDataAccessLibrary.get_parent_class(clazz)

    def add_rules_for_class(self, asset, class_name, rules):
        if class_name in self.rules:
            for rule in self.rules[class_name]:
                if rule.matcher.test(asset):
                    rules.append(rule)


validator_rules_registry = ValidatorRulesRegistry()
warn_on_missing_asset_types = True


# The validator itself
@unreal.uclass()
class NamingEditorValidator(unreal.EditorValidatorBase):
    def __init__(self, *args, **kargs):
        super(NamingEditorValidator, self).__init__(*args, **kargs)

    @unreal.ufunction(override=True)
    def validate_loaded_asset(self, asset: unreal.Object, validation_errors):
        rule = validator_rules_registry.find_matching_rule(asset)
        if rule:
            messages = rule.validator.validate(asset)
            errors_present = False
            for message in messages:
                if message.warning:
                    self.asset_warning(asset, unreal.Text.cast(message.message))
                else:
                    errors_present = True
                    validation_errors = self.asset_fails(asset, unreal.Text.cast(message.message), validation_errors)

            if errors_present:
                return unreal.DataValidationResult.INVALID, validation_errors
            else:
                return unreal.DataValidationResult.VALID, validation_errors
        else:
            if warn_on_missing_asset_types:
                self.asset_warning(asset,
                                   unreal.Text.cast(f"Asset validator missing rules for the "
                                                    f"asset type: {asset.get_class().get_fname()}"))
            self.asset_passes(asset)
            return unreal.DataValidationResult.NOT_VALIDATED, validation_errors

    @unreal.ufunction(override=True)
    def can_validate_asset(self, asset: unreal.Object) -> bool:
        rule = validator_rules_registry.find_matching_rule(asset)
        if warn_on_missing_asset_types or rule:
            return True
        else:
            return False


def setup():
    # Register the validator
    print("Registering NamingEditorValidator")
    subsystem = unreal.get_editor_subsystem(unreal.EditorValidatorSubsystem)
    if subsystem:
        subsystem.add_validator(NamingEditorValidator())
    else:
        print("Failed to register NamingEditorValidator as EditorValidatorSubsystem not present")


def register_validator(asset_type: str, matcher: MatcherBase, validator: ValidatorBase, priority: int = 100) -> None:
    validator_rules_registry.add(ValidatorRule(asset_type, matcher, validator, priority))
