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

import sys
from PySide.QtGui import QApplication, QSplashScreen

import compileuifiles

def main():
    # first recompile all UI files to ensure they are up to date
    compileuifiles.compileUIFiles("./ui")
    compileuifiles.compileUIFiles("./ui/widgets")

    # import mainwindow after UI files have been recompiled
    import mainwindow
    
    app = QApplication(sys.argv)
    app.setOrganizationName("CEGUI")
    app.setOrganizationDomain("cegui.org.uk")
    app.setApplicationName("CEED - CEGUI editor")
    app.setApplicationVersion("0.1 - WIP")
    
    splash = QSplashScreen()
    splash.show()
    splash.showMessage("Testing...")

    wnd = mainwindow.MainWindow()
    wnd.show()
    
    splash.finish(wnd)
    
    app.exec_()

if __name__ == "__main__":
    main()
