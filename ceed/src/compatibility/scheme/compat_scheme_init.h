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

#ifndef CEED_compatibility_scheme___init___
#define CEED_compatibility_scheme___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/scheme/compat_scheme_cegui.h"

namespace CEED {
namespace compatibility {
namespace scheme {

/*!
\brief Manager

Manager of scheme compatibility layers
*/
class Manager : public compatibility::Manager
{
public:
    Manager()
        : compatibility::Manager()
    {
        CEGUIVersionTypes = {
            { "0.4" , cegui::CEGUIScheme1 },
            { "0.5" , cegui::CEGUIScheme2 },
            { "0.6" , cegui::CEGUIScheme3 },
            { "0.7" , cegui::CEGUIScheme4 },
            { "0.8" , cegui::CEGUIScheme5 }
        };

        EditorNativeType = cegui::CEGUIScheme5;

#if 0 // TODO
        m_detectors.append(new cegui::Scheme4TypeDetector());
#endif
        m_detectors.append(new cegui::Scheme5TypeDetector());
#if 0 // TODO
        m_layers.append(new cegui::CEGUI4ToCEGUI5Layer());
        m_layers.append(new cegui::CEGUI5ToCEGUI4Layer());
#endif
    }
};

extern Manager* manager;

} // namespace cegui
} // namespace compatibility
} // namespace CEED

#endif
