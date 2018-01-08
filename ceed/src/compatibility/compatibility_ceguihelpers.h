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

#ifndef CEED_compatibility_ceguihelpers_
#define CEED_compatibility_ceguihelpers_

#include "CEEDBase.h"

/**Misc helper functionality often reused in compatibility layers
*/

#include "xmledit.h"

//from xml.sax import parseString, handler
//from io import BytesIO
//from xml.etree import cElementTree as ElementTree

namespace  CEED {
namespace compatibility {
namespace ceguihelpers {

bool checkDataVersion(const QString &rootElement, const QString& version, const QString& data);

QString prettyPrintXMLElement(ElementTree::Element *rootElement);

} // namespace ceguihelpers
} // namespace compatibility
} // namespace CEED

#endif
