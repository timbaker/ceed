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

#ifndef CEED_editors_looknfeel___init___
#define CEED_editors_looknfeel___init___

#include "CEEDBase.h"
/*
##############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2014   Martin Preisler <martin@preisler.me>
#                             and contributing authors (see AUTHORS file)
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
##############################################################################
*/

#include "editors/editors_init.h"

#include "compatibility/looknfeel/compat_looknfeel_init.h"

#include "editors/looknfeel/tabbed_editor.h"

namespace CEED {
namespace editors {
namespace looknfeel {

class LookNFeelTabbedEditorFactory : public editors::TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Look-N-Feel";
    }

    QSet<QString> getFileExtensions()
    {
        auto extensions = compatibility::looknfeel::manager->getAllPossibleExtensions();
        return extensions;
    }

    bool canEditFile(const QString& filePath)
    {
        auto extensions = getFileExtensions();

        for (QString extension : extensions) {
            if (filePath.endsWith("." + extension))
                return true;
        }

        return false;
    }

    TabbedEditor* create(const QString& filePath)
    {
        return new tabbed_editor::LookNFeelTabbedEditor(filePath);
    }
};

} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif