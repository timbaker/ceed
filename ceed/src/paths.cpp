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

#include "paths.h"

#include <QApplication>

#include "ceed_paths.h"

namespace CEED {
namespace paths {

bool FROZEN;
QString PACKAGE_DIR;
QString DATA_DIR;
QString SYSTEM_DATA_DIR;
bool SYSTEM_DATA_DIR_EXISTS;
QString DOC_DIR;
QString SYSTEM_DOC_DIR;
bool SYSTEM_DOC_DIR_EXISTS;
QString UI_DIR;
QString SYSTEM_PIXMAPS_DIR;
QString SYSTEM_APPLICATIONS_DIR;
QString SYSTEM_APPDATA_DIR;

void init()
{
    // Whether the application is frozen using cx_Freeze
    FROZEN = false;

    // What's the absolute path to the package directory
    PACKAGE_DIR = QDir(QApplication::applicationDirPath()).filePath("data");

    PACKAGE_DIR = QFileInfo("../data").absoluteFilePath();
    qDebug() << "setting PACKAGE_DIR to " << PACKAGE_DIR;
#if 0
    if (PACKAGE_DIR.endsWith(os.path.join("library.zip", "ceed"))) {
        FROZEN = true;
        PACKAGE_DIR = os.path.dirname(PACKAGE_DIR);
    }
#endif

    // What's the absolute path to the data directory
    DATA_DIR = "";

    // Potential system data dir, we check it's existence and set
    // DATA_DIR as system_data_dir if it exists
    SYSTEM_DATA_DIR = "/usr/share/ceed";
    SYSTEM_DATA_DIR_EXISTS = false;
    if (os.path.exists(SYSTEM_DATA_DIR)) {
        DATA_DIR = SYSTEM_DATA_DIR;
        SYSTEM_DATA_DIR_EXISTS = true;
    }

    if (!SYSTEM_DATA_DIR_EXISTS)
        DATA_DIR = os.path.join(os.path.dirname(PACKAGE_DIR), "data");

    // What's the absolute path to the doc directory
    DOC_DIR = "";

    // Potential system doc dir, we check it's existence and set
    // DOC_DIR as system_data_dir if it exists
    SYSTEM_DOC_DIR = QString("/usr/share/doc/ceed-%1").arg(version::CEED);
    SYSTEM_DOC_DIR_EXISTS = false;
    if (os.path.exists(SYSTEM_DOC_DIR)) {
        DOC_DIR = SYSTEM_DOC_DIR;
        SYSTEM_DOC_DIR_EXISTS = true;

    } else {
        SYSTEM_DOC_DIR = "/usr/share/doc/ceed";
        if (os.path.exists(SYSTEM_DOC_DIR)) {
            DOC_DIR = SYSTEM_DOC_DIR;
            SYSTEM_DOC_DIR_EXISTS = true;
        }
    }

    if (!SYSTEM_DOC_DIR_EXISTS)
        DOC_DIR = os.path.join(os.path.dirname(PACKAGE_DIR), "doc");

    // What's the absolute path to the ui directory
    UI_DIR = os.path.join(PACKAGE_DIR, "ui");

    // We don't need this locally, we don't even need it to exist
    // This is where we install our icons
    SYSTEM_PIXMAPS_DIR = "/usr/share/pixmaps";
    // This is where we install .desktop files
    SYSTEM_APPLICATIONS_DIR = "/usr/share/applications";
    // This is where we install .desktop files
    SYSTEM_APPDATA_DIR = "/usr/share/appdata";

    // if one of these assertions fail your installation is not valid!
    if (false && !FROZEN) {
        // these two checks will always fail in a frozen instance
        Q_ASSERT(os.path.exists(PACKAGE_DIR));
        Q_ASSERT(os.path.exists(UI_DIR));
    }
    Q_ASSERT(os.path.exists(DATA_DIR));
}

} // namespace paths
} // namespace CEED
