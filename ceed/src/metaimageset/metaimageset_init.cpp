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

#include "metaimageset/metaimageset_init.h"

#include "compatibility/imageset/compat_imageset_init.h"

#include "elementtree.h"

#include "metaimageset/inputs/metaimageset_inputs_init.h"
#include "metaimageset/inputs/metaimageset_inputs_registry.h"

#include "ceed_paths.h"

namespace CEED {
namespace editors {
namespace metaimageset {

MetaImageset::MetaImageset(const QString &filePath)
{
    m_filePath = filePath;

    m_name = "";
    m_nativeHorzRes = 800;
    m_nativeVertRes = 600;
    m_autoScaled = false;

    m_onlyPOT = false;

    m_output = "";
    m_outputTargetType = compatibility::imageset::manager->EditorNativeType;
    //        m_inputs = [];
}

QString MetaImageset::getOutputDirectory()
{
    return os.path.abspath(os.path.dirname(m_filePath));
}

void MetaImageset::loadFromElement(ElementTree::Element *element)
{
    m_name = element->get("name", "");
    m_nativeHorzRes = element->getInt("nativeHorzRes", 800);
    m_nativeVertRes = element->getInt("nativeVertRes", 600);
    m_autoScaled = element->get("autoScaled", "false") == "true";

    m_onlyPOT = element->get("onlyPOT", "false") == "true";

    m_outputTargetType = element->get("outputTargetType", compatibility::imageset::manager->EditorNativeType);
    m_output = element->get("output", "");

    inputs::registry::loadFromElement(this, element);
}

ElementTree::Element *MetaImageset::saveToElement()
{
    auto ret = new ElementTree::Element("MetaImageset");
    ret->set("name", m_name);
    ret->set("nativeHorzRes", QString::number(m_nativeHorzRes));
    ret->set("nativeVertRes", QString::number(m_nativeVertRes));
    ret->set("autoScaled", m_autoScaled);

    ret->set("onlyPOT", m_onlyPOT);

    ret->set("outputTargetType", m_outputTargetType);
    ret->set("output", m_output);

    // FIXME: Should we be using registry for this too?
    for (auto input_ : m_inputs) {
        auto element = input_->saveToElement();
        ret->append(element);
    }

    return ret;
}

} // namespace metaimageset
} // namespace editors
} // namespace CEED
