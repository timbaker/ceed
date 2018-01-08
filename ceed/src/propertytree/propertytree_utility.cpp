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

#include "propertytree_utility.h"

namespace CEED {
namespace propertytree {
namespace utility {

properties::EditorOption getDictionaryTreePath(const properties::EditorOptions &dtree, const QString &path,
                                               const properties::EditorOption &defaultValue)
{
    /**Get the value of the dictionary tree at the specified path string.

    Return 'defaultValue' if the path can't be found.

    Example::
        tree = { "instantApply" : false, "numeric": { "max" : 10 } }
        path = "numeric/max"
        getDictionaryTreePath(tree, path, 100)
    */
    if (dtree.isEmpty() || path.isEmpty())
        return defaultValue;

    // remove slashes from start because the path is always absolute.
    // we do not remove the final slash, if any, because it is
    // allowed (to return a subtree of options)
    QString path_ = path;
    while (path_.startsWith('/'))
        path_.remove(0, 1);

    QStringList pcs = path_.split('/');
    properties::EditorOptions optRoot = dtree;
    properties::EditorOption option = optRoot;

    for (int i = 0; i < pcs.length(); i++) {
        QString pc = pcs[i];
        // if the path component is an empty string we've reached the destination,
        // getDictionaryTreePath("folder1/") for example.
        if (pc.isEmpty())
            return option;

        // if the pc exists in the current root, make it root and
        // process the next pc
        if (optRoot.contains(pc)) {
            option = optRoot.value(pc);
            optRoot = option.toMap();
            continue;
        }

        // if it wasn't found in the current root, return the default value.
        return defaultValue;
    }

    // we've traversed the option tree, return whatever our root is
    return option;
}

bool boolFromString(const QString &text)
{
//    if (isinstance(text, bool))
//        return text;

    QString lower = text.toLower();
    return lower == "true" || lower == "yes" || lower == "1";
}

int intFromString(const QString &text)
{
    bool ok;
    int ret = text.toInt(&ok);
    if (!ok) throw ValueError(QString("intFromString failed with string \"" + text + "\""));
    return ret;
}

uint uintFromString(const QString &text)
{
    bool ok;
    uint ret = text.toUInt(&ok);
    if (!ok) throw ValueError(QString("uintFromString failed with string \"" + text + "\""));
    return ret;
}

float floatFromString(const QString &text)
{
    bool ok;
    float ret = text.toFloat(&ok);
    if (!ok) throw ValueError(QString("floatFromString failed with string \"" + text + "\""));
    return ret;
}


} // namespace utility
} // namespace propertytree
} // namespace CEED
