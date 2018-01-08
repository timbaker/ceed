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

#ifndef CEED_settings___init___
#define CEED_settings___init___

#include "CEEDBase.h"

/** Declares global settings entries. Provides an entry point to the settings
system via getEntry(..).
*/

#include "settings/settings_declaration.h"
//#include "settings/interface.h"

class QSettings;

namespace CEED {
namespace settings {

class Settings : public declaration::Settings
{
public:
    static Settings* instance;

    Settings(QSettings* qsettings);
};

/**This is a convenience method to make settings retrieval easier
*/
declaration::Entry* getEntry(const QString& path);

//__all__ = ["declaration", "persistence", "interface", "Settings", "getEntry"]

} // namespace settings
} // namespace CEED

#endif
