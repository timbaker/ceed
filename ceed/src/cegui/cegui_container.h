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

#ifndef CEED_cegui_container_
#define CEED_cegui_container_

#include "CEEDBase.h"

/**Implementation of a convenience Qt and CEGUI interaction containment.

Allows you to use CEGUI as if it were a Qt widget.
*/

#include <QDialog>
#include <QQueue>

class Ui_CEGUIContainerWidget;
class Ui_CEGUIWidgetInfo;

class QComboBox;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QVBoxLayout;
//class QWebView;

namespace CEED {
namespace cegui {
namespace container {

class ContainerWidget;

class LogMessageWrapper
{
public:
    QString m_message;
    CEGUI::LoggingLevel m_level;

    LogMessageWrapper(const QString& message, CEGUI::LoggingLevel level)
    {
        m_message = message;
        m_level = level;
    }

    QString asTableRow();
};


/*!
\brief DebugInfo

A debugging/info widget about the embedded CEGUI instance
*/
class DebugInfo : public QDialog
{
public:
    ContainerWidget* m_containerWidget;
    int m_boxUpdateInterval;
    Ui_CEGUIWidgetInfo* m_ui;
    QLineEdit *m_currentFPSBox;
    QLineEdit* m_currentRenderTimeBox;
    int m_errors;
    QLineEdit* m_errorsBox;
    int m_warnings;
    QLineEdit* m_warningsBox;
    int m_others;
    QLineEdit* m_othersBox;
    QWidget* m_logViewArea;
    QVBoxLayout* m_logViewAreaLayout;
#if 1
    QTextEdit* m_logView;
#else
    QWebView* m_logView;
#endif
    int m_logMessagesLimit;
    QQueue<LogMessageWrapper> m_logMessages;

    // This will allow us to view logs in Qt in the future

    DebugInfo(ContainerWidget* containerWidget);

    void logEvent(const QString& message, CEGUI::LoggingLevel level);

    void show();

    void updateFPSTick();
};


class ViewState
{
public:
    QTransform transform;
    int horizontalScroll;
    int verticalScroll;

    ViewState()
    {
        transform = QTransform();
        horizontalScroll = 0;
        verticalScroll = 0;
    }
};


/*!
\brief ContainerWidget


    This widget is what you should use (alongside your GraphicsScene derived class) to
    put CEGUI inside parts of the editor.

    Provides resolution changes, auto expanding and debug widget

*/
class ContainerWidget : public QWidget
{
public:
    cegui::Instance* ceguiInstance;
    mainwindow::MainWindow* m_mainWindow;
    Ui_CEGUIContainerWidget* m_ui;
    QWidget* m_currentParentWidget;
    DebugInfo* m_debugInfo;
    qtgraphics::GraphicsView* m_view;
    QComboBox* m_resolutionBox;
    QPushButton* m_debugInfoButton;

    ContainerWidget(cegui::Instance* ceguiInstance, mainwindow::MainWindow* mainWindow);

    /**If you have already activated this container, you can call this to enable CEGUI input propagation
        (The CEGUI instance associated will get mouse and keyboard events if the widget has focus)
        */
    void enableInput();

    /**Disables input propagation to CEGUI instance.
        see enableInput
        */
    void disableInput();

    /**Activates OpenGL context of the associated CEGUI instance*/
    void makeGLContextCurrent();

    /**The CEGUI view class has several enable/disable features that are very hard to achieve using
    inheritance/composition so they are kept in the CEGUI view class and its base class.

    This method enables/disables various features, calling it with no parameters switches to default.

    wheelZoom - mouse wheel will zoom in and out
    middleButtonScroll - pressing and dragging with the middle button will cause panning/scrolling
    continuousRendering - CEGUI will render continuously (not just when you tell it to)
    */
    void setViewFeatures(bool wheelZoom = false, bool middleButtonScroll = false, bool continuousRendering = true);

    /**Activates the CEGUI Widget for the given parentWidget (QWidget derived class).
    */
    void activate(QWidget* parentWidget, qtgraphics::GraphicsScene* scene = nullptr);

    /**Deactivates the widget from use in given parentWidget (QWidget derived class)
    see activate

    Note: We strive to be very robust about various differences across platforms (the order in which hide/show events
    are triggered, etc...), so we automatically deactivate if activating with a preexisting parentWidget. That's the
    reason for the parentWidget parameter.
    */
    void deactivate(QWidget* parentWidget);

    void updateResolution();

    void setViewState(ViewState* viewState);

    ViewState* getViewState();

    void slot_resolutionBoxChanged(const QString& text)
    {
        Q_UNUSED(text)
        updateResolution();
    }

    void slot_debugInfoButton()
    {
        m_debugInfo->show();
    }
};

} // namespace container
} // namespace cegui
} // namespace CEED

#endif
