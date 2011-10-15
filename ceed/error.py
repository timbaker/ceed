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

from PySide.QtGui import QDialog, QTextBrowser, QLabel
import logging

import version
import ui.exceptiondialog

class ExceptionDialog(QDialog):
    """This is a dialog that gets shown whenever an exception is thrown and
    isn't caught. This is realised via duck overriding the sys.excepthook.
    """

    # Long term TODO:
    # Add an option to pack all the relevant data and error messages to a zip
    # file for easier to reproduce bug reports.

    def __init__(self, exc_type, exc_message, exc_traceback):
        super(ExceptionDialog, self).__init__()

        self.ui = ui.exceptiondialog.Ui_ExceptionDialog()
        self.ui.setupUi(self)

        self.details = self.findChild(QTextBrowser, "details")
        self.mantisLink = self.findChild(QLabel, "mantisLink")

        self.setWindowTitle("Exception %s" % (exc_type))

        import traceback
        formattedTraceback = traceback.format_tb(exc_traceback)
        self.tracebackStr = ""
        for line in formattedTraceback:
            self.tracebackStr += line + "\n"

        # Add some extra info.
        self._stampMercurialInfo()
        self._stampVersionInfo()

        self.details.setPlainText("Exception message: %s\n\n"
                                 "Traceback:\n"
                                 "%s"
                                 % (exc_message, self.tracebackStr))
        
    # Convenience; internal use only
    def _stamp(self, newLine, arg):
        self.tracebackStr = "\n".join([self.tracebackStr, newLine.format(arg)])

    # Appends Mercurial info to traceback string
    def _stampMercurialInfo(self):
        self._stamp("CEED revision: {0}", version.MercurialRevision)

    # Appends version info to traceback string
    def _stampVersionInfo(self):
        # If the versioning info was stored in an object, not a module,
        # we could iterate ... hint hint.
        self._stamp("CEED version: {0}", version.CEED)
        
        self._stamp("HW architecture: {0}", version.SystemArch)
        self._stamp("HW type: {0}", version.SystemType)
        self._stamp("HW processor: {0}", version.SystemCore)
        self._stamp("OS type: {0}", version.OSType)
        self._stamp("OS release: {0}", version.OSRelease)
        self._stamp("OS version: {0}", version.OSVersion)
        OSType = version.OSType
        if OSType == "Windows":
            self._stamp("OS Windows: {0}", version.Windows)
        elif OSType == "Linux":
            self._stamp("OS Linux: {0}", version.Linux)
        elif OSType == "Java":
            self._stamp("OS Java: {0}", version.Java)
        elif OSType == "Darwin":
            self._stamp("OS Darwin: {0}", version.Mac)
        self._stamp("SW Python: {0}", version.Python)
        self._stamp("SW PySide: {0}", version.PySide)
        self._stamp("SW Qt: {0}", version.Qt)
        self._stamp("SW OpenGL: {0}", version.OpenGL)
        self._stamp("SW PyCEGUI: {0}", version.PyCEGUI)

class ErrorHandler(object):
    """This class is responsible for all error handling. It only handles exceptions for now.
    """

    # TODO: handle stderr messages as soft errors

    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def installExceptionHook(self):
        sys.excepthook = self.excepthook

    def uninstallExceptionHook(self):
        sys.excepthook = sys.__excepthook__

    def excepthook(self, exc_type, exc_message, exc_traceback):
        if not self.mainWindow:
            sys.__excepthook__(exc_type, exc_message, exc_traceback)

        else:
            dialog = ExceptionDialog(exc_type, exc_message, exc_traceback)
            # we shouldn't use logging.exception here since we are not in an "except" block
            logging.error("Uncaught exception '%s', message: '%s'\n%s'" % (exc_type, exc_message, dialog.tracebackStr))
            # We don't call the standard excepthook anymore since we output to stderr with the logging module
            # we also call the original excepthook which will just output things to stderr
            #sys.__excepthook__(exc_type, exc_message, exc_traceback)
            
            result = dialog.exec_()

            # if the dialog was reject, the user chose to quit the whole app immediately (coward...)
            if result == QDialog.Rejected:
                # stop annoying the user
                self.uninstallExceptionHook()
                # and try to quit as soon as possible
                # we don't care about exception safety/cleaning up at this point
                sys.exit(1)