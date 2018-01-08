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

#ifndef CEED_about_
#define CEED_about_

#include "CEEDBase.h"

/**Implements the License and About dialogs
*/

// TODO: What is LicenseDialog doing in this package?

#include "ui_LicenseDialog.h"
#include "ui_AboutDialog.h"

#include "version.h"

namespace CEED {
namespace about {

/*!
\brief LicenseDialog

Shows GPLv3 and related info in the UI of the application as
    FSF recommends.

    Almost all of it is in the .ui file, editable with QtDesigner

*/
class LicenseDialog : public QDialog
{
public:
    Ui_LicenseDialog* m_ui;

    LicenseDialog()
        : QDialog()
    {
        m_ui = new Ui_LicenseDialog();
        m_ui->setupUi(this);
    }

    ~LicenseDialog()
    {
        delete m_ui;
    }
};


/*!
\brief AboutDialog

About/Version dialog shown when user selects Help -> About.

    The main goal is to show versions of various things, we can then tell the
    user to just go to this dialog and tell us the versions when something
    goes wrong for them.

*/
class AboutDialog : public QDialog
{
public:
    Ui_AboutDialog* m_ui;

    AboutDialog()
        : QDialog()
    {
        m_ui = new Ui_AboutDialog();
        m_ui->setupUi(this);

        // background, see the data/images directory for SVG source
        m_ui->aboutImage->setPixmap(QPixmap(":images/splashscreen.png"));

        m_ui->CEEDDescription->setText(
            "Please report any issues to help this project."
        );

        m_ui->CEEDVersion->setText("CEED: " + version::CEED);
        m_ui->QtVersion->setText(QString("Qt: %1.%2.%3").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH));

//        findChild<QLabel>("CEEDVersion").setText("CEED: " + (version::CEED));
//        findChild<QLabel>("PySideVersion").setText("PySide: %1" % (version::PYSIDE))
//        findChild<QLabel>("QtVersion").setText("Qt: " + (version::QT));
//        findChild<QLabel>("PyCEGUIVersion").setText("PyCEGUI: %1" % (version::PYCEGUI))
    }

    ~AboutDialog()
    {
        delete m_ui;
    }
};

} // namespace about
} // namespace CEED

#endif
