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

#include "cegui_container.h"

#include "mainwindow.h"
#include "project.h"
#include "cegui_qtgraphics.h"

#include "ui_CEGUIWidgetInfo.h"
#include "ui_CEGUIContainerWidget.h"

#include <QScrollBar>
#include <QTextEdit>
#include <QTimer>

namespace CEED {
namespace cegui {
namespace container {

QString LogMessageWrapper::asTableRow()
{
    QString stringLevel;
    QString bgColour = "transparent";

    if (m_level == CEGUI::LoggingLevel::Errors) {
        stringLevel = "E";
        bgColour = "#ff5f5f";

    } else if (m_level == CEGUI::LoggingLevel::Warnings) {
        stringLevel = "W";
        bgColour = "#fff76f";

    } else {
        stringLevel = " ";
        bgColour = "transparent";
    }

    return QString("<tr><td style=\"background: %1\">%2</td><td>%3</td></tr>\n").arg(bgColour).arg(stringLevel).arg(m_message);
}

/////

DebugInfo::DebugInfo(ContainerWidget *containerWidget):
    QDialog()
{
    setVisible(false);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    m_containerWidget = containerWidget;
    // update FPS and render time very second
    m_boxUpdateInterval = 1;

    m_ui = new Ui_CEGUIWidgetInfo();
    m_ui->setupUi(this);

    m_currentFPSBox = m_ui->currentFPSBox;
//    m_currentRenderTimeBox = m_ui->currentRenderTimeBox;

    m_errors = 0;
    m_errorsBox = m_ui->errorsBox;

    m_warnings = 0;
    m_warningsBox = m_ui->warningsBox;

    m_others = 0;
    m_othersBox = m_ui->othersBox;

    m_logViewArea = m_ui->logViewArea;
    m_logViewAreaLayout = new QVBoxLayout();

#if 1 // TODO
    m_logView = new QTextEdit();
//    m_logView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_logViewAreaLayout->addWidget(m_logView);
#else
    m_logView = new QWebView();
    m_logView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    m_logViewAreaLayout->addWidget(m_logView);
#endif

    m_logViewArea->setLayout(m_logViewAreaLayout);

    m_logMessagesLimit = settings::getEntry("global/cegui_debug_info/log_limit")->m_value.toInt();
//    m_logMessages = QQueue();

    m_containerWidget->ceguiInstance->m_logger->registerSubscriber([=](const QString& a, CEGUI::LoggingLevel b){ logEvent(a, b); });
}

void DebugInfo::logEvent(const QString &message, CEGUI::LoggingLevel level)
{
    if (level == CEGUI::LoggingLevel::Errors) {
        m_errors += 1;
        m_errorsBox->setText(QString::number(m_errors));

    } else if (level == CEGUI::LoggingLevel::Warnings) {
        m_warnings += 1;
        m_warningsBox->setText(QString::number(m_warnings));

    } else {
        m_others += 1;
        m_othersBox->setText(QString::number(m_others));
    }

    // log info using the logging message, allows debug outputs without GUI
#if 0 // TODO
    logging.info("CEGUI message: %s", message);
#endif

    // remove old messages
    while (m_logMessages.length() >= m_logMessagesLimit)
        m_logMessages.removeFirst();

    m_logMessages.append(LogMessageWrapper(message, level));
}

void DebugInfo::show()
{
#if 1 // TODO
    m_logView->clear();
    for (int row = 0; row < m_logMessages.size(); row++) {
        auto& msg = m_logMessages.at(row);
        m_logView->append(msg.m_message);
    }
#elif 0
    m_logView->clear();
    m_logView->setRowCount(m_logMessages.size());
    m_logView->setColumnCount(2);
    m_logView->setHorizontalHeaderLabels({"Level", "Message"});
    m_logView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeMode::ResizeToContents);
    QString levelStr;
    for (int row = 0; row < m_logMessages.size(); row++) {
        auto& msg = m_logMessages.at(row);
        switch (msg.m_level) {
        case CEGUI::LoggingLevel::Errors: levelStr = QStringLiteral("E"); break;
        case CEGUI::LoggingLevel::Warnings: levelStr = QStringLiteral("W"); break;
        default: levelStr.clear();
        }
        QTableWidgetItem* item0 = new QTableWidgetItem(levelStr);
        QTableWidgetItem* item1 = new QTableWidgetItem(msg.m_message);
        m_logView->setItem(row, 0, item0);
        m_logView->setItem(row, 1, item1);
    }
#else
    QStringList strings;
    for (auto msg : m_logMessages)
        strings += msg.asTableRow();
    QString htmlLog = strings.join("\n");
    m_logView.setHtml(R"(
<html>
<body>
<style type="text/css">
font-size: 10px;
</style>
<table>
<thead>
<th></th><th>Message</th>
</thead>
<tbody>
)" + htmlLog + R"(
</tbody>
</table>
</html>)");
#endif
    QDialog::show();
    updateFPSTick();
}

void DebugInfo::updateFPSTick()
{
    if (!isVisible())
        return;

    float lastRenderDelta = m_containerWidget->ceguiInstance->m_lastRenderTimeDelta;
    if (lastRenderDelta <= 0)
        lastRenderDelta = 1;

    m_currentFPSBox->setText(QString("%1").arg(1.0 / lastRenderDelta, 0, 'g', 6));

    QTimer::singleShot(500, this, &DebugInfo::updateFPSTick);
}

/////

ContainerWidget::ContainerWidget(Instance *ceguiInstance, mainwindow::MainWindow *mainWindow)
    : QWidget()
{
    this->ceguiInstance = ceguiInstance;
    m_mainWindow = mainWindow;

    m_ui = new Ui_CEGUIContainerWidget();
    m_ui->setupUi(this);

    m_currentParentWidget = nullptr;

    m_debugInfo = new DebugInfo(this);
    m_view = m_ui->view;
    ceguiInstance->setGLContextProvider(m_view);
    m_view->setBackgroundRole(QPalette::Dark);
//    m_view->m_containerWidget = this;

    m_resolutionBox = m_ui->resolutionBox;
    connect(m_resolutionBox, &QComboBox::editTextChanged, this, &ContainerWidget::slot_resolutionBoxChanged);

    m_debugInfoButton = m_ui->debugInfoButton;
    connect(m_debugInfoButton, &QPushButton::clicked, this, &ContainerWidget::slot_debugInfoButton);
}

void ContainerWidget::enableInput()
{
    m_view->m_injectInput = true;
}

void ContainerWidget::disableInput()
{

    m_view->m_injectInput = false;
}

void ContainerWidget::makeGLContextCurrent()
{
    static_cast<QGLWidget*>(m_view->viewport())->makeCurrent();
}

void ContainerWidget::setViewFeatures(bool wheelZoom, bool middleButtonScroll, bool continuousRendering)
{
    // always zoom to the original 100% when changing view features
    m_view->zoomOriginal();
    m_view->m_wheelZoomEnabled = wheelZoom;

    m_view->m_middleButtonDragScrollEnabled = middleButtonScroll;

    m_view->m_continuousRendering = continuousRendering;
}

void ContainerWidget::activate(QWidget *parentWidget, qtgraphics::GraphicsScene *scene)
{
    // sometimes things get called in the opposite order, lets be forgiving and robust!
    if (m_currentParentWidget != nullptr)
        deactivate(m_currentParentWidget);

    m_currentParentWidget = parentWidget;

    if (scene == nullptr)
        scene = new qtgraphics::GraphicsScene(ceguiInstance);

    m_currentParentWidget->setUpdatesEnabled(false);
    m_view->setScene(scene);
    // make sure the resolution is set right for the given scene
    slot_resolutionBoxChanged(m_resolutionBox->currentText());

    if (m_currentParentWidget->layout())
        m_currentParentWidget->layout()->addWidget(this);
    else
        setParent(m_currentParentWidget);
    m_currentParentWidget->setUpdatesEnabled(true);

    // cause full redraw of the default context to ensure that nothing gets stuck
    CEGUI::System::getSingleton().getDefaultGUIContext().markAsDirty();

    // and mark the view as dirty to force Qt to redraw it
    m_view->update();

    // finally, set the OpenGL context for CEGUI as current as other code may rely on it
    makeGLContextCurrent();
}

void ContainerWidget::deactivate(QWidget *parentWidget)
{
    if (m_currentParentWidget != parentWidget)
        return;

    m_currentParentWidget->setUpdatesEnabled(false);
    // back to the defaults
    setViewFeatures();
    m_view->setScene(nullptr);

    if (m_currentParentWidget->layout())
        m_currentParentWidget->layout()->removeWidget(this);
    else
        setParent(nullptr); // FIXME: can't be null according to docs
    m_currentParentWidget->setUpdatesEnabled(false);

    m_currentParentWidget = nullptr;
}

void ContainerWidget::updateResolution()
{
    QString text = m_resolutionBox->currentText();

    if (text == "Project default") {
        // special case
        auto project = m_mainWindow->m_project;

        if (project != nullptr)
            text = project->m_CEGUIDefaultResolution;
    }

    if (!text.contains('x'))
        return;

    QStringList res = text.split('x');
    if (res.length() == 2) {
        try {
            // clamp both to 1 - 4096, should suit 99% of all cases
            int width = qBound(1, res[0].toInt(), 4096);
            int height = qBound(1, res[1].toInt(), 4096);

            ceguiInstance->makeGLContextCurrent();
            static_cast<qtgraphics::GraphicsScene*>(m_view->scene())->setCEGUIDisplaySize(width, height, false);

        } catch (ValueError e) {
            // ignore invalid literals
        }
    }
}

void ContainerWidget::setViewState(ViewState *viewState)
{
    m_view->setTransform(viewState->transform);
    m_view->horizontalScrollBar()->setValue(viewState->horizontalScroll);
    m_view->verticalScrollBar()->setValue(viewState->verticalScroll);
}

ViewState *ContainerWidget::getViewState()
{
    ViewState* ret = new ViewState();
    ret->transform = m_view->transform();
    ret->horizontalScroll = m_view->horizontalScrollBar()->value();
    ret->verticalScroll = m_view->verticalScrollBar()->value();

    return ret;
}

} // namespace container
} // namespace cegui
} // namespace CEED

