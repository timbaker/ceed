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

#ifndef CEED_compatibility_imageset_cegui_
#define CEED_compatibility_imageset_cegui_

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/compatibility_ceguihelpers.h"

namespace CEED {
namespace compatibility {
namespace imageset {
namespace cegui {

const QString CEGUIImageset1 = "CEGUI imageset 1";
const QString CEGUIImageset2 = "CEGUI imageset 2";

class Imageset1TypeDetector : public compatibility::TypeDetector
{
    QString getType() override
    {
        return CEGUIImageset1;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "imageset" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "imageset")
            return false;

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)
        return ceguihelpers::checkDataVersion("Imageset", "", data);
    }
};

class Imageset2TypeDetector : public compatibility::TypeDetector
{
    QString getType() override
    {
        return CEGUIImageset2;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "imageset" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "imageset")
            return false;

        return ceguihelpers::checkDataVersion("Imageset", "2", data);
    }
};

class CEGUI1ToCEGUI2Layer : public compatibility::Layer
{
    QString getSourceType() override
    {
        return CEGUIImageset1;
    }

    QString getTargetType() override
    {
        return CEGUIImageset2;
    }

    QString transform(const QString& data);
};

class CEGUI2ToCEGUI1Layer : public compatibility::Layer
{
    QString getSourceType() override
    {
        return CEGUIImageset2;
    }

    QString getTargetType() override
    {
        return CEGUIImageset1;
    }
#if 0
    @classmethod
    def autoScaledToBoolean(cls, value)
    {
        if (value in ["true", "vertical", "horizontal", "min", "max"]) {
            return "true";
        else
            return "false";
        }
    }
#endif

    QString transform(const QString& data);
};

} // namespace cegui
} // namespace imageset
} // namespace compatibilty
} // namespace CEED

#endif
