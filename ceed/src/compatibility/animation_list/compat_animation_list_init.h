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

#ifndef CEED_compatibility_animation_list___init___
#define CEED_compatibility_animation_list___init___

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"

#include "elementtree.h"

namespace CEED {
namespace compatibility {
namespace animation_list {

const QString AnimationList1 = "CEGUI Animation List 1";

class AnimationList1TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return AnimationList1;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return  {"anims" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        Q_UNUSED(data)

        if (extension != "" && extension != "anims")
            return false;

        // TODO
        return true;
    }
};


/*!
\brief Manager

Manager of CEGUI animation list compatibility layers
*/
class Manager : public compatibility::Manager
{
public:
    Manager()
        : compatibility::Manager()
    {
        EditorNativeType = AnimationList1;

        CEGUIVersionTypes = {
            { "0.6" , "" },
            { "0.7" , AnimationList1 },
            { "0.8" , AnimationList1 }
        };

        m_detectors.append(new AnimationList1TypeDetector());
    }
};

extern Manager* manager;

} // namespace animation_list
} // namespace compatibililty
} // namespace CEED

#endif
