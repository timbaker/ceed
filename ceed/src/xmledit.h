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

#ifndef CEED_xmledit_
#define CEED_xmledit_

#include "CEEDBase.h"

#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextEdit>

#include "elementtree.h"

namespace CEED {
namespace xmledit {

// taken from ElementLib and slightly tweaked for readability
void indent(ElementTree::Element* elem, int level = 0, const QString& tabImpostor = "    ");

class XMLSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    struct Rule
    {
        QRegExp re;
        QTextCharFormat fmt;
    };

    QMap<QString, Rule> m_highlightingRules;

    XMLSyntaxHighlighter(QTextDocument* parent = nullptr);

    void highlightBlock(const QString &text) override;
};

class XMLEditWidget : public QTextEdit
{
public:
    XMLSyntaxHighlighter* m_highlighter;

    XMLEditWidget();
};

} // namespace xmledit
} // namespace CEED

#endif
