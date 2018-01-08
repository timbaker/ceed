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

#ifndef CEED_propertytree_utility_
#define CEED_propertytree_utility_

#include "CEEDBase.h"

#include "propertytree_properties.h"

#include <QVariant>

namespace CEED {
namespace propertytree {
namespace utility {

using properties::EditorOptions;

/**Misc utilities.

getDictionaryTreePath -- Retrieve a value from a dictionary tree.
*/

properties::EditorOption getDictionaryTreePath(const EditorOptions& dtree, const QString& path, const properties::EditorOption &defaultValue = EditorOptions());

bool boolFromString(const QString& text);
int intFromString(const QString& text);
uint uintFromString(const QString& text);
float floatFromString(const QString& text);

} // namespace utility
} // namespace propertytree
} // namespace CEED

#endif
