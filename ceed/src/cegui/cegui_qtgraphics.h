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
   along with this program.  if (not, see <http) {//www.gnu.org/licenses/>.
*/

#ifndef CEED_cegui_qtgraphics_
#define CEED_cegui_qtgraphics_

#include "CEEDBase.h"

#include "cegui/cegui_init.h"
#include "qtwidgets.h"
#include "resizable.h"

#include <QEvent>
#include <QGL>
#include <QGLFramebufferObject>
#include <QtMath>
#include <QTime>

namespace CEED {
namespace cegui {
namespace qtgraphics {

/*!
\brief GraphicsScene

A scene that draws CEGUI as it's background.

    Subclass this to be able to show Qt graphics items and widgets
    on top of the embedded CEGUI widget!

    Interaction is also supported

*/
class GraphicsScene : public QGraphicsScene
{
public:
    Instance* m_ceguiInstance;
    int m_scenePadding;
    CEGUI::Sizef m_ceguiDisplaySize;
    QTime m_timeOfLastRender;
    float m_lastDelta;
    QGLFramebufferObject* m_fbo;
    settings::declaration::Entry* m_checkerWidth;
    settings::declaration::Entry* m_checkerHeight;
    settings::declaration::Entry* m_checkerFirstColour;
    settings::declaration::Entry* m_checkerSecondColour;

    GraphicsScene(Instance* ceguiInstance);

    virtual void setCEGUIDisplaySize(int width, int height, bool lazyUpdate = true);

    /**We override this and draw CEGUI instead of the whole background.
    This method uses a FBO to implement zooming, scrolling around, etc...

    FBOs are therefore required by CEED and it won't run without a GPU that supports them.
    */
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

class GraphicsView : public resizable::GraphicsView, public cegui::GLContextProvider
{
public:
    /**This is a final class, not suitable for subclassing. This views given scene
    using QGLWidget. It's designed to work with cegui.GraphicsScene derived classes.
    */

    QGLFormat m_glFormat;
    bool m_injectInput;
    bool m_continuousRendering;
    int m_continuousRenderingTargetFPS;

    GraphicsView(QWidget* parent = nullptr);

    void makeGLContextCurrent()
    {
        static_cast<QGLWidget*>(viewport())->makeCurrent();
    }

    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void updateSelfAndScene();

    void mouseMoveEvent(QMouseEvent *event);

    optional<CEGUI::MouseButton> translateQtMouseButton(Qt::MouseButton button);

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    optional<CEGUI::Key::Scan> translateQtKeyboardButton(Qt::Key button);

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;
};

} // namespace qtgraphcis
} // namespace cegui
} // namespace CEED

#endif
