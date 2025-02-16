Document Source Code
=========================

# Generating HTML Document
## Set Up Environment
Run `source setup.sh` to set up the environment for generating documents.

## Makefile targets

The top Makefile contains the following targets: 

+ **html**: calls `sphinx` to generate HTML files from generated reStructuredText files and manually written ones.
+ **install**: copies the HTML files to `$HTML_DEST_DIR`. If HTML_DEST_DIR is not set, the default is "html-output".

# Trademark Notice

Xilinx, the Xilinx logo, Artix, ISE, Kintex, Spartan, Virtex, Zynq, and other designated brands included herein
are trademarks of Xilinx in the United States and other countries.
All other trademarks are the property of their respective owners.

```

Copyright 2021 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

```
