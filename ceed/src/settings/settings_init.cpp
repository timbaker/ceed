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

#include "settings/settings_init.h"

#include "settings/settings_interface.h"
#include "settings/settings_persistence.h"

#include "cegui/cegui_settings_decl.h"
#include "editors/imageset/editor_imageset_settings_decl.h"
#include "editors/layout/editor_layout_settings_decl.h"
#include "editors/looknfeel/editor_looknfeel_settings_decl.h"

namespace CEED {
namespace settings {

Settings* settings::Settings::instance = nullptr;

using declaration::Category;
using declaration::Section;

Settings::Settings(QSettings *qsettings)
    : declaration::Settings("settings",
                            "CEGUI Unified Editor settings",
                            "Provides all persistent settings of CEGUI Unified Editor (CEED), everything is divided into categories (see the tab buttons).")
{
    setPersistenceProvider(new persistence::QSettingsPersistenceProvider(qsettings));

    // General settings
    Category& global_ = *createCategory("global", "Global");
    Section* undoRedo = global_.createSection("undo", "Undo and Redo");
    // by default we limit the undo stack to 500 undo commands, should be enough and should
    // avoid memory drainage. keep in mind that every tabbed editor has it's own undo stack,
    // so the overall command limit is number_of_tabs * 500!
    undoRedo->createEntry("limit",
                          QVariant::Int,
                          500,
                          "Limit (number of steps)",
                          "Puts a limit on every tabbed editor's undo stack. You can undo at most the number of times specified here.",
                          "int",
                          1,
                          true);

    Section* app = global_.createSection("app", "Application");
    app->createEntry("show_splash",
                     QVariant::Bool,
                     true,
                     "Show splash screen",
                     "Show the splash screen on startup",
                     "checkbox",
                     1,
                     false);

    Section* ui = global_.createSection("ui", "User Interface");
    ui->createEntry("toolbar_icon_size", QVariant::Int, 32,
                    "Toolbar icon size",
                    "Sets the size of the toolbar icons",
                    "combobox",
                    1,
                    false,
    { {32, "Normal"}, {24, "Small"}, {16, "Smaller"} });

    Section* ceguiDebugInfo = global_.createSection("cegui_debug_info", "CEGUI debug info");
    ceguiDebugInfo->createEntry("log_limit", QVariant::Int, 20000,
                                "Log messages limit",
                                "Limits number of remembered log messages to given amount. This is there to prevent endless growth of memory consumed by CEED.",
                                "int",
                                1, true);

    Section* navigation = global_.createSection("navigation", "Navigation");
    navigation->createEntry("ctrl_zoom", QVariant::Bool, true,
                            "Only zoom when CTRL is pressed",
                            "Mouse wheel zoom is ignored unless the Control key is pressed when it happens.",
                            "checkbox",
                            1, false);

    cegui::settings_decl::declare(this);
    editors::imageset::settings_decl::declare(this);
    editors::layout::settings_decl::declare(this);
    editors::looknfeel::settings_decl::declare(this);

    Q_ASSERT(Settings::instance == nullptr);
    Settings::instance = this;
}

/////

declaration::Entry *getEntry(const QString &path)
{
    Q_ASSERT(Settings::instance != nullptr);
    return Settings::instance->getEntry(path);
}


} // namespace settings
} // namespace CEED
