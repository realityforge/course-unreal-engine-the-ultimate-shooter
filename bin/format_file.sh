#!/usr/bin/env bash

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

if [[ "$#" != "1" ]]; then
  echo "$0: Must be invoked with a single argument."
  exit 1
else
  # Format Source code using clang-format
  set pattern = "^${1//\\//}"
  git diff --name-only HEAD | grep -E "${pattern}" | grep -i -e "\.h$" -e "\.cpp$" | grep -e "^Source/" | xargs --no-run-if-empty clang-format -i
fi
