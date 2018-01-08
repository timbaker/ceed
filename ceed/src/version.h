/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CEED_version_
#define CEED_version_

#include "CEEDBase.h"

#include <QString>

namespace CEED {
namespace version {

/*
 * This module is used as a centrepoint to gather version info

Beyond version information, we also store a few other details - e.g. system
architecture - which are used in the event of errors/exceptions.
*/

static const QString CEED("0.8.1");

// if this is true, all .ui files will be recompiled every time CEED.py is run
static const bool DEVELOPER_MODE = true;

#if 0
// Architecture
SYSTEM_ARCH = platform.architecture()
SYSTEM_TYPE = platform.machine()
SYSTEM_PROCESSOR = platform.processor()

# OS agnostic
OS_TYPE = platform.system()
OS_RELEASE = platform.release()
OS_VERSION = platform.version()


# OS specific
WINDOWS = LINUX = JAVA = MAC = None
if OS_TYPE == "Windows":
    WINDOWS = platform.win32_ver()
    #sys.getwindowsversion()
elif OS_TYPE == "Linux":
    LINUX = platform.linux_distribution()
elif OS_TYPE == "Java":
    # Jython
    JAVA = platform.java_ver()
elif OS_TYPE == "Darwin":
    # OSX
    MAC = platform.mac_ver()

# Python
PYTHON = sys.version
PYTHON_TUPLE = sys.version_info

# in case the try block fails, set all the tuples and values to something
PYSIDE = "N/A"
PYSIDE_TUPLE = ("N", "/", "A")

QT = "N/A"
QT_TUPLE = ("N", "/", "A")

OPENGL = "N/A"

PYCEGUI = "N/A"

# all of the other versions are just optional, what we always need and will always get
# is the CEED version

try:
    # PySide
    from PySide import __version__ as _PySideVersion
    from PySide import __version_info__ as _PySideVersion_Tuple
    PYSIDE = _PySideVersion
    PYSIDE_TUPLE = _PySideVersion_Tuple

    # Qt
    from PySide.QtCore import __version__ as _QtVersion
    from PySide.QtCore import __version_info__ as _QtVersion_Tuple
    QT = _QtVersion
    QT_TUPLE = _QtVersion_Tuple

except:
    pass

try:
    # PyOpenGL
    from OpenGL.version import __version__ as _OpenGLVersion
    OPENGL = _OpenGLVersion

except:
    pass

try:
    # PyCEGUI
    from PyCEGUI import Version__ as _PyCEGUIVersion
    PYCEGUI = _PyCEGUIVersion

except:
    pass
#endif

} // namespace version
} // namespace CEED

#endif
