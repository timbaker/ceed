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

#ifndef CEED_compatibility_property_mappings___init___
#define CEED_compatibility_property_mappings___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/compatibility_ceguihelpers.h"

namespace CEED {
namespace compatibility {
namespace property_mappings {

const QString PropertyMappings1 = "CEED Property Mappings 1";

class PropertyMappings1TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return PropertyMappings1;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return QSet<QString>(); // ???
    }

    bool matches(const QString& data, const QString& extension)
    {
        if (extension != "" && extension != "pmappings")
            return false;

        // should work as a pretty rigorous test for now, tests the root tag name and version
        // CEED property mapping files have a similar version check to CEGUI, that's why we can use
        // the cegui helper function here.
        return ceguihelpers::checkDataVersion("mappings", PropertyMappings1, data);
    }
};


/*!
\brief Manager

Manager of CEED project compatibility layers
*/
class Manager : public compatibility::Manager
{
public:
    Manager()
        : compatibility::Manager()
    {
        EditorNativeType = PropertyMappings1;

        // doesn't make much sense
        CEGUIVersionTypes = {
            { "0.6" , PropertyMappings1 },
            { "0.7" , PropertyMappings1 },
            { "0.8" , PropertyMappings1 }
        };

        m_detectors.append(new PropertyMappings1TypeDetector());
    }
};

extern Manager* manager;

} // namespace property_mappings
} // namespace compatibility
} // namespace CEED

#endif
