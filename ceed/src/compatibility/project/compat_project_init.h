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

#ifndef CEED_compatibility_project___init___
#define CEED_compatibility_project___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/compatibility_ceguihelpers.h"

namespace CEED {
namespace compatibility {
namespace project {

const QString Project1 = "CEED Project 1";

class Project1TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return Project1;
    }

    QSet<QString> getPossibleExtensions()
    {
        return { "project" };
    }

    bool matches(const QString& data, const QString &extension) override
    {
        if (extension != "" && extension != "project")
            return false;

        // should work as a pretty rigorous test for now, tests the root tag name and version
        // CEED project files have a similar version check to CEGUI, that's why we can use
        // the cegui helper function here.
        return ceguihelpers::checkDataVersion("Project", Project1, data);
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
        EditorNativeType = Project1;

        // doesn't make much sense
        CEGUIVersionTypes = {
            { "0.6",  Project1 },
            { "0.7", Project1 },
            { "0.8", Project1 }
        };

        m_detectors.append(new Project1TypeDetector());
    }
};

extern Manager* manager;

} // namespace project
} // namespace compatibility
} // namespace CEED

#endif
