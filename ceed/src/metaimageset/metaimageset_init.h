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

#ifndef CEED_metaimageset___init___
#define CEED_metaimageset___init___

#include "CEEDBase.h"

/**This module provides all metaimageset API core functionality (except editing)
*/


namespace CEED {
namespace editors {
namespace metaimageset {

class MetaImageset
{
public:
    QString m_filePath;
    QString m_name;
    int m_nativeHorzRes;
    int m_nativeVertRes;
    bool m_autoScaled;
    bool m_onlyPOT;
    QString m_output;
    QString m_outputTargetType;
    QList<inputs::Input*> m_inputs;

    MetaImageset(const QString& filePath);

    QString getOutputDirectory();

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();
};

} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
