################################################################################
#   CEED - A unified CEGUI editor
#   Copyright (C) 2011 Martin Preisler <preisler.m@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

# Allows freezing the executable (currently Windows specific code pretty much)

NAME = "CEED"
SHORTCUT_NAME = NAME
SHORTCUT_DIR = "CEGUI"

from cx_Freeze import setup, Executable

buildOptions = dict(
    base = "Win32GUI",
    
    packages =
    [
        "OpenGL",
        "OpenGL.platform",
        "OpenGL.arrays.formathandler"
    ],
    
    include_files =
    [
        "icons",
        "ui", # FIXME: because we always rebuild those upon starting
        "data",
        
        # hax for now
        #"C:/Python26/Lib/site-packages/PyCEGUI/CEGUIOpenGLRenderer.dll",
    ]   
)

setup(
    name = "CEED",
    version = "0.1",
    description = "CEGUI Unified Editor",
    options = dict(build_exe = buildOptions),
    executables = [
        Executable(
            "entry.py",
            targetName = NAME + ".exe",
            shortcutName = SHORTCUT_NAME,
            shortcutDir = SHORTCUT_DIR
        )
    ] 
)
