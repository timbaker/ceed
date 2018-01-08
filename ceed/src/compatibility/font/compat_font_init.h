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

#ifndef CEED_compatibility_font___init___
#define CEED_compatibility_font___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/font/compat_font_cegui.h"

namespace CEED {
namespace compatibility {
namespace font {

/*!
\brief Manager

Manager of font compatibility layers
*/
class Manager : public compatibility::Manager
{
public:
    Manager()
        : compatibility::Manager()
    {
        EditorNativeType = cegui::CEGUIFont3;

        CEGUIVersionTypes = {
            {"0.4" , cegui::CEGUIFont1 },
            {"0.5" , cegui::CEGUIFont2 }, // font rewrite
            {"0.6" , cegui::CEGUIFont2 },
            {"0.7" , cegui::CEGUIFont2 },
            {"0.8" , cegui::CEGUIFont3 },
            {"9999.0" , cegui::CEGUIFont4 },
        };

        m_detectors.append(new cegui::Font2TypeDetector());
        m_detectors.append(new cegui::Font3TypeDetector());
        m_detectors.append(new cegui::Font4TypeDetector());

        m_layers.append(new cegui::Font2ToFont3Layer());
#if 0 // TODO
        m_layers.append(new cegui::Font3ToFont2Layer());
#endif
    }
};

extern Manager* manager;

} // namesapce font
} // namespace compatibility
} // namespace CEED

#endif
