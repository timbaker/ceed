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

#ifndef CEED_application_
#define CEED_application_

#include "CEEDBase.h"

#include "settings/settings_interface.h"
#include "version.h"

#include <QApplication>
#include <QSplashScreen>

class QSettings;

namespace CEED {
namespace application {

class SplashScreen : public QSplashScreen
{
public:
    SplashScreen();
};

/*!
\brief Application

The central application class

*/
class Application : public QApplication
{
public:
    Application(int &argc, char **argv, int flags = ApplicationFlags);

    QSettings* m_qsettings;
    settings::Settings* m_settings;
    SplashScreen* m_splash;
    mainwindow::MainWindow* m_mainWindow;
    error::ErrorHandler* m_errorHandler;
};

} // namespace application
} // namespace CEED

#endif
