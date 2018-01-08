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

#include "error.h"

namespace CEED {
namespace error {

ExceptionDialog::ExceptionDialog(const QString &excType, const QString &excMessage, const QString &excTraceback, mainwindow::MainWindow *mainWindow)
    : QDialog()
{
    m_ui = new Ui_ExceptionDialog();
    m_ui->setupUi(this);

    m_mainWindow = mainWindow;

    m_details = m_ui->details;
    m_mantisLink = m_ui->mantisLink;

    setWindowTitle(tr("Uncaught exception '%1'").arg(excType));

    m_detailsStr = tr("Exception message: %1\n\n"
                      "Traceback:\n"
                      "========================\n"
                      "%2\n"
                      "Versions:\n"
                      "========================\n"
                      "%3").arg(excMessage).arg(getTracebackStr(excTraceback)).arg(getVersionsStr());

    m_details->setPlainText(m_detailsStr);
}

QString ExceptionDialog::getTracebackStr(const QString &excTraceback)
{
#if 1
    return "TODO";
#else
    import traceback

            formattedTraceback = traceback.format_tb(excTraceback)
            return unicode("\n").join(formattedTraceback)
#endif
}

QString ExceptionDialog::getVersionsStr()
{
    QStringList lines;
#if 0
    lines.append(QString(QStringLiteral("CEED revision: %1")).arg(version.MERCURIAL_REVISION));

    lines.append(QString(QStringLiteral("CEED version: %1").arg(version.CEED));

            lines.append(QString(QStringLiteral("HW architecture: %1").arg(version.SYSTEM_ARCH)));
    lines.append(QString(QStringLiteral("HW type: %1").arg(version.SYSTEM_TYPE)));
    lines.append(QString(QStringLiteral("HW processor: %1".arg(version.SYSTEM_PROCESSOR)));
            lines.append(QString(QStringLiteral("OS type: %1").arg(version.OS_TYPE)));
    lines.append(QString(QStringLiteral("OS release: %1").arg(version.OS_RELEASE)));
    lines.append(QString(QStringLiteral("OS version: %1").arg(version.OS_VERSION)));

    if (OS_TYPE == "Windows")
        lines.append(QString(QStringLiteral("OS Windows: %1").arg(version.WINDOWS)));
    else if (version.OS_TYPE == "Linux")
        lines.append(QString(QStringLiteral("OS Linux: %1").arg(unicode(version.LINUX)));
                else if (version.OS_TYPE == "Java")
                lines.append(QString(QStringLiteral("OS Java: %1").arg(version.JAVA)));
    else if (version.OS_TYPE == "Darwin")
        lines.append("OS Darwin: %s" % (unicode(version.MAC)));
    else:
        lines.append("OS Unknown: No version info available")

                lines.append("SW Python: %s" % (version.PYTHON))
                lines.append("SW PySide: %s" % (version.PYSIDE))
                lines.append("SW Qt: %s" % (version.QT))
                lines.append("SW PyCEGUI: %s" % (version.PYCEGUI))

                lines.append("GL bindings version: %s" % (version.OPENGL))

                if self.mainWindow.ceguiInstance is not None:
            self.mainWindow.ceguiInstance.makeGLContextCurrent()
          lines.append("GL version: %s" % (GL.glGetString(GL.GL_VERSION)))
          lines.append("GL vendor: %s" % (GL.glGetString(GL.GL_VENDOR)))
          lines.append("GL renderer: %s" % (GL.glGetString(GL.GL_RENDERER)))

  # FIXME: This is not available in OpenGL 3.1 and above
          extensionList = unicode(GL.glGetString(GL.GL_EXTENSIONS)).split(" ")
  # sometimes the OpenGL vendors return superfluous spaces at the
  # end of the extension str, this causes the last element be ""
          if extensionList[-1] == "":
          extensionList.pop()
          lines.append("GL extensions:\n    - %s" % (",\n    - ".join(extensionList)))

          else:
          lines.append("Can't query OpenGL info, CEGUI instance hasn't been started!")
  #endif
          return lines.join('\n');
}


} // namepace error
} // namespace CEED
