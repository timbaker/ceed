#!/bin/bash

# Sure the linker gurus wouldn't approve, but I use this to avoid having to mess
# with RPATH or having to install everything when I rebuild

# Obviously doesn't work on Windows ;-) But works OK on Linux and MacOSX

# parent dir of this script
PARENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# relative to where you run the script from or absolute (probably a more robust solution)
CEGUI_BUILD_PATH="$PARENT_DIR/../../cegui_mk2"
# directory where the "ceed" package is located
CEED_PACKAGE_PATH="$PARENT_DIR/../"

# Not needed with the new RPATH stuff in 1.0
#export LD_LIBRARY_PATH="$CEGUI_BUILD_PATH/build/cegui/src/:$CEGUI_BUILD_PATH/build/cegui/src/RendererModules/OpenGL:$LD_LIBRARY_PATH"

export PYTHONPATH="$CEGUI_BUILD_PATH/build/lib:$CEED_PACKAGE_PATH:$PYTHONPATH"

# fork a new shell to avoid polluting the environment
bash
