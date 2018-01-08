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

#include "propertytree_parsers.h"

namespace CEED {
namespace propertytree {
namespace parsers {

OrderedMap<QString, QString> parseNamedValues(const QString &strValue_, const QStringList &allowedNames_, const QStringList &requiredNames_, bool ignoreCase)
{
    OrderedMap<QString, QString> pairs;

    QString strValue = strValue_.trimmed();

    if (strValue.isEmpty())
        return pairs;

    QSet<QString> allowedNames, requiredNames;
    if (ignoreCase) {
        strValue = strValue.toLower();
        for (QString s : allowedNames_)
            allowedNames += s.toLower();
        for (QString s : requiredNames_)
            requiredNames += s.toLower();
    }

    // assume the worst, like: 'test: 5  name : this is multiword q:one-word b :multi word again'
    // remote extra spaces
    QString _s = strValue.replace(QRegExp("\\s*:\\s*"), ":");
    // replace spaces before keys with ':'
    _s = _s.replace(QRegExp("\\s(\\w+):"), "\1:");
    // split by ':", with return name, value, name, value, ...
    QStringList ss = _s.split(':');
    // convert to dictionary (even elements are names, odd are values)
    for (int i = 0; i < ss.length(); i += 2) {
        pairs[ss[i]] = ss[i+1];
    }

    // check required and allowed names
    QSet<QString> pNames = pairs.keys().toSet();
    if (!requiredNames.isEmpty()) {
        if (!pNames.contains(requiredNames)) {
            pairs.clear();
            return pairs;
        }
    }
    if (!allowedNames.isEmpty()) {
        if (!allowedNames.contains(pNames)) {
            pairs.clear();
            return pairs;
        }
    }

    return pairs;
}


} // namespace parsers
} // namespace propertytree
} // namespace CEED
