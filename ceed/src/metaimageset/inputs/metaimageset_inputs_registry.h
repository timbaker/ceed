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

#ifndef CEED_metaimageset_inputs_registry_
#define CEED_metaimageset_inputs_registry_

#include "CEEDBase.h"

/**You only have to extend these functions to add more inputs to metaimageset.
*/

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace registry {

void loadFromElement(MetaImageset* metaImageset, ElementTree::Element* element);

// We currently just use the input classes to save to element
#if 0
def saveElement(metaImageset, element):
    pass
#endif

} // namesapce registry
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
