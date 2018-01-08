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

#ifndef CEED_compatibility_looknfeel___init___
#define CEED_compatibility_looknfeel___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/looknfeel/compat_looknfeel_cegui.h"

namespace CEED {
namespace compatibility {
namespace looknfeel {

/*!
\brief Manager

Manager of looknfeel compatibility layers
*/
class Manager : public compatibility::Manager
{
public:
    Manager()
        : compatibility::Manager()
    {
        CEGUIVersionTypes = {
            { "0.4" , cegui::CEGUILookNFeel1 },
            // we only support non-obsolete major versions
            //"0.5" : cegui.CEGUILookNFeel2,
            { "0.5" , cegui::CEGUILookNFeel3 },
            { "0.6" , cegui::CEGUILookNFeel4 },
            // we only support non-obsolete major versions
            //"0.7.0" : cegui.CEGUILookNFeel5,
            { "0.7" , cegui::CEGUILookNFeel6 }, // because of animations, since 0.7.2,
            { "0.8" , cegui::CEGUILookNFeel7 }
        };

        EditorNativeType = cegui::CEGUILookNFeel7;

#if 0 // TODO
        m_detectors.append(new cegui::LookNFeel6TypeDetector());
#endif
        m_detectors.append(new cegui::LookNFeel7TypeDetector());
#if 0 // TODO
        m_layers.append(new cegui::LookNFeel6To7Layer());
        m_layers.append(new cegui::LookNFeel7To6Layer());
#endif
    }
};

extern Manager* manager;

} // namespace looknfeel
} // namespace compatibility
} // namespace CEED

#endif
