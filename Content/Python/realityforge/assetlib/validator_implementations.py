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

from realityforge.assetlib.api import ValidatorBase, ValidationMessage


class NamePrefixValidator(ValidatorBase):
    """Validate that asset names start with the specified prefix."""

    def __init__(self, prefix: str, case_sensitive: bool = True) -> None:
        self.prefix = prefix
        self.case_sensitive = case_sensitive

    def validate(self, asset: unreal.Object) -> list[ValidationMessage]:
        asset_name = asset.get_name()
        if self.case_sensitive:
            if not asset_name.startswith(self.prefix):
                return [
                    ValidationMessage(f"Asset name '{asset_name}' expected to start with the prefix '{self.prefix}'")]
            else:
                return []
        else:
            if not asset_name.lower().startswith(self.prefix.lower()):
                return [ValidationMessage(f"Asset name '{asset_name}' expected to start with the "
                                          f"prefix '{self.prefix}' (case insensitive comparison)")]
            else:
                return []


class NameSuffixValidator(ValidatorBase):
    """Validate that asset names end with the specified suffix."""

    def __init__(self, suffix: str, case_sensitive: bool = True) -> None:
        self.suffix = suffix
        self.case_sensitive = case_sensitive

    def validate(self, asset: unreal.Object) -> list[ValidationMessage]:
        asset_name = asset.get_name()
        if self.case_sensitive:
            if not asset_name.endswith(self.suffix):
                return [ValidationMessage(f"Asset name '{asset_name}' expected to end "
                                          f"with the suffix '{self.suffix}'")]
            else:
                return []
        else:
            if not asset_name.lower().endswith(self.suffix.lower()):
                return [ValidationMessage(f"Asset name '{asset_name}' expected to end with the "
                                          f"suffix '{self.suffix}' (case insensitive comparison)")]
            else:
                return []


class AllValidator(ValidatorBase):
    """Validate that asset conforms to multiple rules."""

    def __init__(self, validators: list[ValidatorBase]) -> None:
        self.validators = validators

    def validate(self, asset: unreal.Object) -> list[ValidationMessage]:
        messages: list[ValidationMessage] = []
        for validator in self.validators:
            messages.extend(validator.validate(asset))
        return messages
