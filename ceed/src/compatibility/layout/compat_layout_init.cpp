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

#include "compatibility/layout/compat_layout_init.h"

#include "compat_layout_cegui.h"

namespace CEED {
namespace compatibility {
namespace layout {

Manager::Manager()
    : compatibility::Manager()
{
    CEGUIVersionTypes = {
        { "0.5", cegui::CEGUILayout2 },
        { "0.6", cegui::CEGUILayout2 },
        { "0.7", cegui::CEGUILayout3 },
        { "0.8", cegui::CEGUILayout4 }
    };

    EditorNativeType = cegui::CEGUILayout4;

    m_detectors.append(new cegui::Layout2TypeDetector());
    m_detectors.append(new cegui::Layout3TypeDetector());
    m_detectors.append(new cegui::Layout4TypeDetector());
#if 0 // TODO
    m_layers.append(new cegui::Layout3To4Layer());
    m_layers.append(new cegui::Layout4To3Layer());
#endif
}

Manager *manager = new Manager();

} // namespace layout
} // namespace compatibility
} // namespace CEED
