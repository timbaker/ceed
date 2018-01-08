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

#ifndef CEED_editors_htmlview_
#define CEED_editors_htmlview_

#include "CEEDBase.h"

#include "editors/editors_init.h"

//#include <QWebView> // NOT IN 5.7 !!!

namespace CEED {
namespace editors {
namespace htmlview {

/*!
\brief HTMLViewTabbedEditor

This is basically a stub tabbed editor, it simply displays a HTML message
    and doesn't allow any sort of editing at all, all functionality is stubbed

    This is for internal use only so there is no factory for this particular editor

*/
class HTMLViewTabbedEditor : public editors::TabbedEditor
{
    HTMLViewTabbedEditor(const QString& filePath, const QString& message)
        : editors::TabbedEditor(nullptr, filePath)
    {
#if 0
        m_tabWidget = new QWebView();
        m_tabWidget->setHtml(message);
#endif
    }

    bool hasChanges() override
    {
        return false;
    }
};

} // namespace htmlview
} // namespace editors
} // namespace CEED

#endif
