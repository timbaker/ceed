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

from PySide.QtGui import QDialog, QTextBrowser

import ui.exceptiondialog

class ExceptionDialog(QDialog):
    def __init__(self, exc_type, exc_message, exc_traceback):
        super(ExceptionDialog, self).__init__()
        
        self.ui = ui.exceptiondialog.Ui_ExceptionDialog()
        self.ui.setupUi(self)
        
        self.details = self.findChild(QTextBrowser, "details")

        self.setWindowTitle("Exception %s" % (exc_type))

        import traceback
        formattedTraceback = traceback.format_tb(exc_traceback)
        tracebackStr = ""
        for line in formattedTraceback:
            tracebackStr += line + "\n"
            
        self.details.setPlainText("Exception message: %s\n\n"
                                 "Traceback:\n"
                                 "%s"
                                 % (exc_message, tracebackStr))

class ErrorHandler(object):
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow
        
    def installExceptionHook(self):
        sys.excepthook = self.excepthook
        
    def excepthook(self, exc_type, exc_message, exc_traceback):
        if not self.mainWindow:
            sys.__excepthook__(exc_type, exc_message, exc_traceback)
            
        else:
            dialog = ExceptionDialog(exc_type, exc_message, exc_traceback)
            dialog.exec_()
            
            # we also call the original excepthook which will just output things to stderr
            sys.__excepthook__(exc_type, exc_message, exc_traceback)
            
            