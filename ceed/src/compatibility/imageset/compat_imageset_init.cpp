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

#include "compatibility/imageset/compat_imageset_init.h"

#include "compatibility/imageset/compat_imageset_cegui.h"
#include "compatibility/imageset/compat_imageset_gorilla.h"

namespace CEED {
namespace compatibility {
namespace imageset {

Manager* manager = new Manager();

Manager::Manager()
    : compatibility::Manager()
{
    EditorNativeType = cegui::CEGUIImageset2;

    CEGUIVersionTypes = {
        {"0.6" , cegui::CEGUIImageset1},
        {"0.7" , cegui::CEGUIImageset1},
        {"0.8" , cegui::CEGUIImageset2}
    };

    m_detectors.append(new cegui::Imageset1TypeDetector());
    m_detectors.append(new cegui::Imageset2TypeDetector());
    m_layers.append(new cegui::CEGUI1ToCEGUI2Layer());
    m_layers.append(new cegui::CEGUI2ToCEGUI1Layer());
#if 0 // TODO
    m_detectors.append(new gorilla::GorillaTypeDetector());
    m_layers.append(new gorilla::GorillaToCEGUI1Layer());
    m_layers.append(new gorilla::CEGUI1ToGorillaLayer());
#endif
}


} // namespace imageset
} // namespace compatibility
} // namespace CEED
