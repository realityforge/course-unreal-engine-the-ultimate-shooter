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

from realityforge.assetlib.validator_implementations import *


def name_prefix(prefix: str, case_sensitive: bool = True) -> ValidatorBase:
    """Create a validator that validates that the asset names start with the specified prefix.

    :param prefix: the prefix to match.
    :param case_sensitive:  True to make comparison case-sensitive, False otherwise. Defaults to True.
    :return: the new validator
    """
    return NamePrefixValidator(prefix, case_sensitive)


def name_suffix(suffix: str, case_sensitive: bool = True) -> ValidatorBase:
    """Create a validator that validates that the asset names end with the specified suffix.

    :param suffix: the suffix to match.
    :param case_sensitive:  True to make comparison case-sensitive, False otherwise. Defaults to True.
    :return: the new validator
    """
    return NameSuffixValidator(suffix, case_sensitive)


def validate_all(validators: list[ValidatorBase]) -> ValidatorBase:
    """Create a validator that validates that asset conforms to multiple rules.

    :param validators: The validators to validate against.
    :return: the new validator
    """
    return AllValidator(validators)
