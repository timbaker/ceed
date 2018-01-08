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

#include "application.h"

#include "error.h"
#include "mainwindow.h"

#include <QPixmap>
#include <QSettings>

namespace CEED {
namespace application {

SplashScreen::SplashScreen()
    : QSplashScreen(QPixmap("images/splashscreen.png"))
{
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    showMessage(QString("version: %1").arg(version::CEED), Qt::AlignTop | Qt::AlignRight, Qt::GlobalColor::white);
}

/////

Application::Application(int &argc, char **argv, int flags)
    : QApplication(argc, argv, flags)
    , m_splash(nullptr)
{
    m_qsettings = new QSettings("CEGUI", "CEED");

    m_settings = new settings::Settings(m_qsettings);
    // download all values from the persistence store
    m_settings->download();

    bool showSplash = m_settings->getEntry("global/app/show_splash")->m_value.toBool();
    if (showSplash) {
        m_splash = new SplashScreen();
        m_splash->show();

        // this ensures that the splash screen is shown on all platforms
        processEvents();
    }

    setOrganizationName("CEGUI");
    setOrganizationDomain("cegui.org.uk");
    setApplicationName("CEED - CEGUI editor");
    setApplicationVersion(version::CEED);

    m_mainWindow = new mainwindow::MainWindow(*this);
    m_mainWindow->show();
    m_mainWindow->raise();
    if (showSplash) {
        m_splash->finish(m_mainWindow);
    }

    // - Truncate exception log, if it exists.
    m_errorHandler = new error::ErrorHandler(m_mainWindow);
    m_errorHandler->installExceptionHook();
}

} // namespace application
} // namespace CEED
