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

#include "settings/settings_interface.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

namespace CEED {
namespace settings {
namespace interface_ {

QtSettingsInterface::QtSettingsInterface(Settings *settings)
    : SettingsInterface(settings)
    , QDialog()
{
    setWindowTitle(m_settings->m_label);
    setWindowModality(Qt::ApplicationModal);

    // sort everything so that it comes at the right spot when iterating
    m_settings->sort();

    // the basic UI
    m_layout = new QVBoxLayout();

    m_label = new QLabel(m_settings->m_help);
    m_label->setWordWrap(true);
    m_layout->addWidget(m_label);

    m_tabs = new QTabWidget();
    m_tabs->setTabPosition(QTabWidget::North);
    m_layout->addWidget(m_tabs);

    setLayout(m_layout);

    // for each category, add a tab
    for (declaration::Category* category : m_settings->m_categories) {
        m_tabs->addTab(new interface_types::InterfaceCategory(category, m_tabs), category->m_label);
    }

    // apply, cancel, etc...
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &QtSettingsInterface::slot_buttonBoxClicked);
    m_layout->addWidget(m_buttonBox);

    // Restart required
    m_needRestart = new QMessageBox();
    m_needRestart->setWindowTitle("CEED");
    m_needRestart->setIcon(QMessageBox::Warning);
    m_needRestart->setText("Restart is required for the changes to take effect.");
}

void QtSettingsInterface::restartRequired()
{
    if (m_settings->m_changesRequireRestart) {
        m_needRestart->exec();
        // FIXME: Kill the app; then restart it.
        //
        // - This may or may not be the way to get rid of this, but for the
        //   moment we use it as a "the user has been notified they must restart
        //   the application" flag.
        m_settings->m_changesRequireRestart = false;
    }
    return;
}

void QtSettingsInterface::slot_buttonBoxClicked(QAbstractButton *button)
{
    if (m_buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
        m_settings->applyChanges();

        // Check if restart required
        restartRequired();

    } else if (m_buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
        m_settings->applyChanges();
        accept();

        // Check if restart required
        restartRequired();

    } else if (m_buttonBox->buttonRole(button) == QDialogButtonBox::RejectRole) {
        m_settings->discardChanges();
        reject();

        // - Reset any entries with changes to their stored value.
        for (int tabIndex = 0; tabIndex < m_tabs->count(); tabIndex++) {
            auto widget = static_cast<interface_types::InterfaceCategory*>(m_tabs->widget(tabIndex));
            widget->discardChanges();
        }
    }

    // - Regardless of the action above, all categories are now unchanged.
    for (int tabIndex = 0; tabIndex < m_tabs->count(); tabIndex++) {
        auto widget = static_cast<interface_types::InterfaceCategory*>(m_tabs->widget(tabIndex));
        widget->markAsUnchanged();
    }

    // FIXME: That is not entirely true; using the 'X' to close the Settings
    // dialog is not handled here; although, this "bug as a feature" allows
    // Settings to be modified, closed, and it will remember (but not apply)
    // the previous changes.

}


} // namespace interface_
} // namespace settings
} // namespace CEED
