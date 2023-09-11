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

from realityforge.assetlib.api import MatcherBase


class NamePrefixMatcher(MatcherBase):
    """Match assets that are named with a specified prefix."""

    def __init__(self, prefix: str, case_sensitive: bool = True) -> None:
        self.prefix = prefix
        self.case_sensitive = case_sensitive

    def test(self, asset: unreal.Object) -> bool:
        asset_name = asset.get_name()
        if self.case_sensitive:
            return asset_name.startswith(self.prefix)
        else:
            return asset_name.lower().startswith(self.prefix.lower())


class NameSuffixMatcher(MatcherBase):
    """Match assets that are named with a specified suffix."""

    def __init__(self, suffix: str, case_sensitive: bool = True) -> None:
        self.suffix = suffix
        self.case_sensitive = case_sensitive

    def test(self, asset: unreal.Object) -> bool:
        asset_name = asset.get_name()
        if self.case_sensitive:
            return asset_name.endswith(self.suffix)
        else:
            return asset_name.lower().endswith(self.suffix.lower())


class PathPrefixMatcher(MatcherBase):
    """Match assets that are in path with a specified prefix."""

    def __init__(self, prefix: str, case_sensitive: bool = True, absolute_prefix: bool = False) -> None:
        self.prefix = prefix
        self.case_sensitive = case_sensitive
        self.absolute_prefix = absolute_prefix

    def test(self, asset: unreal.Object) -> bool:
        path_name = asset.get_path_name()
        if self.absolute_prefix:
            actual_prefix = f"/Game/{self.prefix}"
        else:
            actual_prefix = self.prefix

        if self.case_sensitive:
            return path_name.startswith(actual_prefix)
        else:
            return path_name.lower().startswith(actual_prefix.lower())


class EditorPropertyMatcher(MatcherBase):
    """Match assets that have an editor property with the specified value."""

    def __init__(self, property_name: str, property_value: str) -> None:
        self.property_name = property_name
        self.property_value = property_value

    def test(self, asset: unreal.Object) -> bool:
        current_asset = asset
        while current_asset:
            try:
                if current_asset.get_editor_property(self.property_name) == self.property_value:
                    return True
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
