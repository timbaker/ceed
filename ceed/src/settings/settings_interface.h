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

#ifndef CEED_settings_interface_
#define CEED_settings_interface_

#include "CEEDBase.h"

#include "settings/settings_interface_types.h"
#include "settings/settings_init.h"

#include <QDialog>

class QDialogButtonBox;
class QLabel;
class QMessageBox;

namespace CEED {
namespace settings {
namespace interface_ {

class SettingsInterface
{
public:
    SettingsInterface(settings::Settings* settings)
        : m_settings(settings)
    {
    }

    Settings* m_settings;
};

class QtSettingsInterface : public SettingsInterface, public QDialog
{
public:
    QVBoxLayout* m_layout;
    QLabel* m_label;
    QTabWidget* m_tabs;
    QDialogButtonBox* m_buttonBox;
    QMessageBox* m_needRestart;

    QtSettingsInterface(Settings* settings);

    void restartRequired();

private slots:
    void slot_buttonBoxClicked(QAbstractButton* button);
};

} // namespace interface_
} // namespace settings
} // namespace CEED

#endif
