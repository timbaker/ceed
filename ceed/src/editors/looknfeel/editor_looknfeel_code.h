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

#ifndef CEED_editors_looknfeel_code_
#define CEED_editors_looknfeel_code_

#include "CEEDBase.h"

#include "editors/editors_multi.h"

#include <QSyntaxHighlighter>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace code {

class WidgetLookHighlighter;

class CodeEditing : public multi::CodeEditMode
{
public:
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    WidgetLookHighlighter* m_highlighter;

    /**
    :param tabbedEditor: LookNFeelTabbedEditor
    :return:
    */
    CodeEditing(tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    // Returns the Look n' Feel XML string based on all WidgetLookFeels that belong to the Look n' Feel file according to the editor
    QString getNativeCode() override;

    bool propagateNativeCode(const QString& code) override;

    void moveToAndSelectWidgetLookFeel(const QString& widgetLookFeelName);

    /**Refreshes this Code editing mode with current native source code and moves to and selects the
    WidgetLookFeel code.*/
    void refreshFromVisual() override;

    UndoStackTabbedEditor* getTabbedEditor() override { return m_tabbedEditor; }
};


/*!
\brief WidgetLookHighlighter


    Highlighter for the LNF code editing

*/
class WidgetLookHighlighter : public QSyntaxHighlighter
{
public:
#if 1 // TODO
    WidgetLookHighlighter(QObject* parent)
        : QSyntaxHighlighter(parent)
    {

    }

    void highlightBlock(const QString &text) override
    {

    }

    void updateWidgetLookRule(const QString& widgetLookName)
    {

    }
#else
    def __init__(self, parent):
        super(WidgetLookHighlighter, self).__init__(parent)
        self.parent = parent

        // A dictionary containing the rules names associated with their start regex, end regex and pattern to be used
        self.multilineHighlightingRules = dict()

    def updateWidgetLookRule(self, widgetLookName):
        /**
        Updates the regular expression for the WidgetLook highlighting rule
        :param widgetLookName:
        :return:
        */
        wlfTagStartText = "<WidgetLook name=\"%s\"" % widgetLookName
        regexStart = QtCore.QRegExp(wlfTagStartText)
        regexStart.setMinimal(true)

        wlfTagEndText = "</WidgetLook>"
        regexEnd = QtCore.QRegExp(wlfTagEndText)
        regexEnd.setMinimal(true)

        palette = QApplication.palette()

        // FIXME: The color palette should be used correctly here instead of hardcoding the color.
        // However neither mpreisler or me (Ident) knew how to do it "the right way"

        highlightingFormat = QTextCharFormat()
        highlightingFormat.setForeground(QColor(0, 89, 176))
        highlightingFormat.setBackground(palette.color(QPalette.Normal, QPalette.Base))

        rule = [regexStart, regexEnd, highlightingFormat]
        self.multilineHighlightingRules["WidgetLookRule"] = rule

    def highlightBlock(self, text):
        /**

        :param text:
        :return:
        */

        // Sets an integer representing the state of the multiline highlighting rule
        self.setCurrentBlockState(0)

        for dictionaryKey in self.multilineHighlightingRules:
            multilineHighlightingRule = self.multilineHighlightingRules[dictionaryKey]

            regexStart = QtCore.QRegExp(multilineHighlightingRule[0])
            regexEnd = QtCore.QRegExp(multilineHighlightingRule[1])
            highlightFormat = multilineHighlightingRule[2]

            positionOfStartMatch = regexStart.indexIn(text)
            if positionOfStartMatch >= 0:
                self.setCurrentBlockState(1)

            positionOfEndMatch = regexEnd.indexIn(text)
            if positionOfEndMatch >= 0:
                self.setCurrentBlockState(2)

            // In case the match of the start has been found in this line
            if self.currentBlockState() == 1:
                length = len(text) - positionOfStartMatch
                self.setFormat(positionOfStartMatch, length, highlightFormat)

            // In case a match for the end been found in this line and the start was found in a previous one
            if self.currentBlockState() == 2 and self.previousBlockState() == 1:
                length = positionOfEndMatch + regexEnd.matchedLength()
                self.setFormat(0, length, highlightFormat)

            // In case the match of the start has been found in a previous line, and no end was found in this line
            if self.previousBlockState() == 1 and not self.currentBlockState() == 2:
                length = len(text)
                self.setFormat(0, length, highlightFormat)
                self.setCurrentBlockState(1)
#endif
};

} // namespace code
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
