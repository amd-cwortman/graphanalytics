#!/bin/bash

# Copyright 2022 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set -e
set -x

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 release"
    echo "    release: 0: Only generate a local package"
    echo "             1: Copy package to the release directory"
    exit 1
fi 

release=$1
pkg_name=amd-agml-install
version=$(cat ../plugin/tigergraph/agml/VERSION)
tar_file=$pkg_name-$version.tar

# remove the package with the same version
rm -f $tar_file.gz 

#update version number in the pacakge
python3 ./pkg-update-version.py agml $version

# copy requirements.txt
cp ../requirements.txt $pkg_name/

#generate .tar.gz package
tar cf $tar_file $pkg_name
gzip $tar_file

if [[ $release == 1 ]]; then
    dest=/proj/gdba/release
else
    dest=/proj/gdba/release/internal
fi

mv $tar_file.gz $dest/

# Make a symbolic link of the latest release
if [[ $release == 1 ]]; then
    cd $dest
    ln -sf $tar_file.gz $pkg_name-latest.tar.gz
    cd -
fi
echo "INFO: Pakcage is available at $dest/$tar_file.gz"

