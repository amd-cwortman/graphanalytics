#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Version control large files
# Developers: Jim Wu
#
# Copyright 2021 Xilinx, Inc.
#
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

import sys
from pathlib import Path
import json

product = sys.argv[1]
version = sys.argv[2]

print('INFO: Updating {} version to {}'.format(product, version))
version_file = Path('./amd-agml-install/VERSION')

# read the current versions
with version_file.open(mode='r') as fh:
    try:
        version_json = json.load(fh)
    except:
        # invalid version file. reset the json dictionary
        version_json = {}

    version_json[product] = version

    with version_file.open(mode='w') as fh:        
        json.dump(version_json, fh, indent=4)

    print('INFO: Version file updated. Current versions:')
    print(json.dumps(version_json, indent=4))