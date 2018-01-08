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

#ifndef CEED_error_
#define CEED_error_

#include "CEEDBase.h"

/** Implements a hook that displays a dialog whenever an exception is uncaught.
We display information to the user about where to submit a bug report and what
to include.
*/

#include "version.h"

#include <QDialog>
#include <QTextBrowser>
#include <QLabel>

#include "ui_ExceptionDialog.h"

namespace CEED {

namespace mainwindow {
class MainWindow;
}

namespace error {

/*!
\brief ExceptionDialog

This is a dialog that gets shown whenever an exception is thrown and
    isn't caught. This is done via duck overriding the sys.excepthook.

*/
class ExceptionDialog : public QDialog
{
public:
    Ui_ExceptionDialog* m_ui;
    mainwindow::MainWindow* m_mainWindow;
    QTextBrowser* m_details;
    QString m_detailsStr;
    QLabel* m_mantisLink;

    // TODO: Add an option to pack all the relevant data and error messages
    //       to a zip file for easier to reproduce bug reports.

    ExceptionDialog(const QString& excType, const QString& excMessage, const QString& excTraceback, mainwindow::MainWindow* mainWindow);

    static QString getTracebackStr(const QString& excTraceback);

    QString getVersionsStr();
};


/*!
\brief ErrorHandler

This class is responsible for all error handling. It only handles exceptions for now.

*/
class ErrorHandler
{
public:
    mainwindow::MainWindow* m_mainWindow;

    // TODO: handle stderr messages as soft errors

    ErrorHandler(mainwindow::MainWindow* mainWindow)
        : m_mainWindow(mainWindow)
    {

    }

    void installExceptionHook()
    {
#if 0
        sys.excepthook = self.excepthook
#endif
    }

    void uninstallExceptionHook()
    {
#if 0
        sys.excepthook = sys.__excepthook__
#endif
    }

#if 0
    void excepthook(exc_type, exc_message, exc_traceback)
    {
        // to be compatible with as much as possible we have the default
        // except hook argument names in the method signature, these do not
        // conform our guidelines so we alias them here

        excType = exc_type
        excMessage = exc_message
        excTraceback = exc_traceback

        if not self.mainWindow:
            sys.__excepthook__(excType, excMessage, excTraceback)

        else:
            dialog = ExceptionDialog(excType, excMessage, excTraceback, self.mainWindow)
            // we shouldn't use logging.exception here since we are not in an "except" block
            logging.error("Uncaught exception '%s'\n%s", excType, dialog.detailsStr)

            // We don't call the standard excepthook anymore since we output to stderr with the logging module
            #sys.__excepthook__(excType, excMessage, excTraceback)

            result = dialog.exec_()

            // if the dialog was rejected, the user chose to quit the whole app immediately (coward...)
            if result == QDialog.Rejected:
                # stop annoying the user
                self.uninstallExceptionHook()
                # and try to quit as soon as possible
                # we don't care about exception safety/cleaning up at this point
                sys.exit(1)
    }
#endif
};

} // namepace error
} // namespace CEED

#endif
