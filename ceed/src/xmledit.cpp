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

#include "xmledit.h"

namespace CEED {
namespace xmledit {


void indent(ElementTree::Element *elem, int level, const QString &tabImpostor)
{
#if 0
    QString i = "\n" + tabImpostor.repeated(level);
    if (elem != nullptr) {
        if (elem->text.isEmpty() || elem->text.trimmed().isEmpty())
            elem->text = i + tabImpostor;

        ElementTree::Element* last = nullptr;
        for (ElementTree::Element *e : elem) {
            indent(e, level + 1);
            if (e->tail.isEmpty() || e->tail.strip().isEmpty())
                e->tail = i + tabImpostor;

            last = e;
        }

        if (last->tail.isEmpty() || last->tail.trimmed().isEmpty())
            last->tail = i;
    } else {
        if (level && (elem->tail.isEmpty() || elem->tail.trimmed().isEmpty()))
            elem->tail = i;
    }
#endif
}


XMLSyntaxHighlighter::XMLSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // todo: some fail colour highlighting :D please someone change the colours
    QTextCharFormat keywordFormat;
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::darkCyan);

    QStringList patterns = { "\\b?xml\\b", "/>", ">", "</", "<" };
    for (auto pattern : patterns) {
        m_highlightingRules[pattern] = { QRegExp(pattern), keywordFormat };
    }

    QTextCharFormat elementNameFormat;
    elementNameFormat.setFontWeight(QFont::Bold);
    elementNameFormat.setForeground(Qt::red);
    QString pattern = "\\b[A-Za-z0-9_]+(?=[\\s/>])";
    m_highlightingRules[pattern] = { QRegExp(pattern), elementNameFormat };

    QTextCharFormat attributeKeyFormat;
    attributeKeyFormat.setFontItalic(true);
    attributeKeyFormat.setForeground(Qt::blue);
    pattern = "\\b[A-Za-z0-9_]+(?=\\=)";
    m_highlightingRules[pattern] = { QRegExp(pattern), attributeKeyFormat };
}

void XMLSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (auto it = m_highlightingRules.begin(); it != m_highlightingRules.end(); it++) {
        Rule& rule = it.value();
        QRegExp& expression = rule.re;
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.fmt);
            index = expression.indexIn(text, index + length);
        }
    }
}

/////

XMLEditWidget::XMLEditWidget()
    : QTextEdit()
{
    setAcceptRichText(false);
    zoomIn();

    m_highlighter = new XMLSyntaxHighlighter(document());
}

} // namespace xmledit
} // namespace CEED
