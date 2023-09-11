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

from realityforge.assetlib.matcher_implementations import *


def name_prefix(prefix: str, case_sensitive: bool = True) -> MatcherBase:
    """Create a matcher for name prefix.

    :param prefix: the prefix to match.
    :param case_sensitive:  True to make matching case-sensitive, False otherwise. Defaults to True.
    :return: the new matcher
    """
    return NamePrefixMatcher(prefix, case_sensitive)


def name_suffix(suffix: str, case_sensitive: bool = True) -> MatcherBase:
    """Create a matcher for name suffix.

    :param suffix: the suffix to match.
    :param case_sensitive:  True to make matching case-sensitive, False otherwise. Defaults to True.
    :return: the new matcher
    """
    return NameSuffixMatcher(suffix, case_sensitive)


def name_prefix(prefix: str, case_sensitive: bool = True) -> MatcherBase:
    """Create a matcher for path prefix.

    :param prefix: the prefix to match.
    :param case_sensitive:  True to make matching case-sensitive, False otherwise. Defaults to True.
    :return: the new matcher
    """
    return PathPrefixMatcher(prefix, case_sensitive)


def editor_property(property_name: str, property_value: str) -> MatcherBase:
    """Create a matcher to match assets that have an editor property with the specified value.

    :param property_name: the name of the editor property.
    :param property_value: the value of the editor property to match.
    :return: the new matcher
    """
    return EditorPropertyMatcher(property_name, property_value)


def match_all(expressions: list[MatcherBase]) -> MatcherBase:
    """Create a matcher to match all conditions.

    :param expressions: the expressions to match.
    :return: the new matcher
    """
    return AllMatcher(expressions)


def match_any(expressions: list[MatcherBase]) -> MatcherBase:
    """Create a matcher to match any conditions.

    :param expressions: the expressions to match.
    :return: the new matcher
    """
    return AnyMatcher(expressions)


def not_match(expression: MatcherBase) -> MatcherBase:
    """Create a matcher to match where expression does not match.

    :param expression: the expression to invert.
    :return: the new matcher
    """
    return NotMatcher(expression)
