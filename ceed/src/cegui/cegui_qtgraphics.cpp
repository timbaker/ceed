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

#define USE_OPENGL3 0
//#include "CEGUI/RendererModules/OpenGL/GL3StateChangeWrapper.h"
//#include "CEGUI/RendererModules/OpenGL/GL3Renderer.h"

#include "cegui_qtgraphics.h"

#include "mainwindow.h"

#include <QtEvents>
#include <QOpenGLFunctions>
#include <QTimer>

namespace CEED {
namespace cegui {
namespace qtgraphics {

GraphicsScene::GraphicsScene(Instance *ceguiInstance)
    : QGraphicsScene()
{
    m_ceguiInstance = ceguiInstance;
    m_scenePadding = 100;

    m_ceguiDisplaySize = CEGUI::Sizef();

    m_timeOfLastRender = QTime::currentTime();
    m_lastDelta = 0;

    m_fbo = nullptr;

    m_checkerWidth = settings::getEntry("cegui/background/checker_width");
    m_checkerHeight = settings::getEntry("cegui/background/checker_height");
    m_checkerFirstColour = settings::getEntry("cegui/background/first_colour");
    m_checkerSecondColour = settings::getEntry("cegui/background/second_colour");

    // reasonable defaults I think
    setCEGUIDisplaySize(800, 600, true);
}

void GraphicsScene::setCEGUIDisplaySize(int width, int height, bool lazyUpdate)
{
    m_ceguiDisplaySize = CEGUI::Sizef(width, height);
    setSceneRect(QRectF(-m_scenePadding, -m_scenePadding,
                        width + 2 * m_scenePadding, height + 2 * m_scenePadding));

    if (!lazyUpdate) {
        CEGUI::System::getSingleton().notifyDisplaySizeChanged(m_ceguiDisplaySize);
    }

    delete m_fbo; // ???
    m_fbo = nullptr;
}

void GraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{

    // be robust, this is usually caused by recursive repainting
    if (painter->paintEngine() == nullptr) {
        return;
    }

    QPaintEngine::Type painterType = painter->paintEngine()->type();
    if (painterType != QPaintEngine::OpenGL && painterType != QPaintEngine::OpenGL2) {
        qWarning("cegui.GraphicsScene: drawBackground needs a "
                 "QGLWidget to be set as viewport on the "
                 "graphics view");

        return;
    }

    auto& system = CEGUI::System::getSingleton();
    m_lastDelta = m_timeOfLastRender.msecsTo(QTime::currentTime()) / 1000.0f;
    m_ceguiInstance->m_lastRenderTimeDelta = m_lastDelta;
    system.injectTimePulse(m_lastDelta);
    system.getDefaultGUIContext().injectTimePulse(m_lastDelta);
    m_timeOfLastRender = QTime::currentTime();

    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(qtwidgets::getCheckerboardBrush(m_checkerWidth->m_value.toInt(), m_checkerHeight->m_value.toInt(),
                                                      m_checkerFirstColour->m_value.value<QColor>(), m_checkerSecondColour->m_value.value<QColor>()));
#if !USE_OPENGL3
    painter->drawRect(0, 0, m_ceguiDisplaySize.d_width, m_ceguiDisplaySize.d_height);
#endif

    painter->beginNativePainting();

    m_ceguiInstance->ensureIsInitialised();

    if (m_ceguiDisplaySize != CEGUI::System::getSingleton().getRenderer()->getDisplaySize()) {
        // FIXME: Change when multi root is in CEGUI core
        CEGUI::System::getSingleton().notifyDisplaySizeChanged(m_ceguiDisplaySize);
    }

    // markAsDirty is called on the default GUI context to work around potential issues with dangling
    // references in the rendering code for some versions of CEGUI.
    system.getDefaultGUIContext().markAsDirty();

    // we have to render to FBO and then scale/translate that since CEGUI doesn't allow
    // scaling the whole rendering root directly

    // this makes sure the FBO is the correct size
    if (m_fbo == nullptr) {
        QSize desiredSize(qCeil(m_ceguiDisplaySize.d_width), qCeil(m_ceguiDisplaySize.d_height));
        m_fbo = new QGLFramebufferObject(desiredSize, GL_TEXTURE_2D);
    }

    m_fbo->bind();

    QOpenGLFunctions GL(QOpenGLContext::currentContext());

    GLint fbo = -1;
    GL.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    GL.glClearColor(0, 0, 0, 0);
    GL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // See comment in OpenGL3Renderer::beginRendering() about deprecated OpenGL2 client states
#if 0
    system.getDefaultGUIContext().getRootWindow()->invalidate(true);
    system.renderAllGUIContexts();
#else
    system.getRenderer()->beginRendering();
    system.getDefaultGUIContext().draw();
    system.getRenderer()->endRendering();
#endif

    m_fbo->release();

#if 1
    painter->endNativePainting();

    QImage image = m_fbo->toImage();
    painter->drawImage(0, 0, image);
#else
    // the stretch and translation should be done automatically by QPainter at this point so just
    // this code will do
    if (bool(GL.glActiveTexture)) {
        GL.glActiveTexture(GL_TEXTURE0);
    }

    GL.glEnable(GL_TEXTURE_2D);
    GL.glBindTexture(GL_TEXTURE_2D, m_fbo->texture());

    GL.glEnable(GL_BLEND);
    GL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: I was told that this is the slowest method to draw with OpenGL,
    //       with which I certainly agree.
    //
    //       No profiling has been done at all and I don't suspect this to be
    //       a painful performance problem.
    //
    //       Still, changing this to a less pathetic rendering method would be great.

    GL.glBegin(GL_TRIANGLES);

    // top left
    GL.glTexCoord2f(0, 1);
    GL.glVertex3f(0, 0, 0);

    // top right
    GL.glTexCoord2f(1, 1);
    GL.glVertex3f(m_fbo->size().width(), 0, 0);

    // bottom right
    GL.glTexCoord2f(1, 0);
    GL.glVertex3f(m_fbo->size().width(), m_fbo->size().height(), 0);

    // bottom right
    GL.glTexCoord2f(1, 0);
    GL.glVertex3f(m_fbo->size().width(), m_fbo->size().height(), 0);

    // bottom left
    GL.glTexCoord2f(0, 0);
    GL.glVertex3f(0, m_fbo->size().height(), 0);

    // top left
    GL.glTexCoord2f(0, 1);
    GL.glVertex3f(0, 0, 0);

    system.getDefaultGUIContext().markAsDirty();

    GL.glEnd();

    painter->endNativePainting();
#endif
}

/////

GraphicsView::GraphicsView(QWidget *parent)
    : resizable::GraphicsView(parent)
    , cegui::GLContextProvider()
{
    // mainly to tone down potential antialiasing
    m_glFormat.setSampleBuffers(true);
    m_glFormat.setSamples(2);

    setViewport(new QGLWidget(m_glFormat));
    // OpenGL doesn't do partial redraws (it isn't practical anyways)
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // make things slightly faster
    setOptimizationFlags(QGraphicsView::DontClipPainter |
                         QGraphicsView::DontAdjustForAntialiasing);

    m_injectInput = false;
    // we might want mouse events
    setMouseTracking(true);
    // we might want key events
    setFocusPolicy(Qt::ClickFocus);

    // if true, we render always (possibly capped to some FPS) - suitable for live preview
    // if false, we render only when update() is called - suitable for visual editing
    m_continuousRendering = true;
    // only applies when we are rendering continuously, it's the max FPS that we will try to achieve
    m_continuousRenderingTargetFPS = 60;
}

void GraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    resizable::GraphicsView::drawBackground(painter, rect);

    if (m_continuousRendering) {
        if (m_continuousRenderingTargetFPS <= 0) {
            updateSelfAndScene();

        } else {
            // * 1000 because QTimer thinks in milliseconds
            GraphicsScene* gscene = static_cast<GraphicsScene*>(scene());
            float lastDelta = gscene ? gscene->m_lastDelta : 0;
            QTimer::singleShot(qMax(0.0, ((1.0 / m_continuousRenderingTargetFPS) - lastDelta) * 1000),
                               this, &GraphicsView::updateSelfAndScene);
        }
    } else {
        // we don't mark ourselves as dirty if user didn't request continuous rendering
    }
}

void GraphicsView::updateSelfAndScene()
{
    update();

    if (auto* gscene = static_cast<GraphicsScene*>(scene())) {
        gscene->update();
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    bool handled = false;

    if (m_injectInput) {
        QPointF point = mapToScene(QPoint(event->x(), event->y()));
        handled = CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition(point.x(), point.y());
    }

    if (!handled) {
        resizable::GraphicsView::mouseMoveEvent(event);
    }
}

optional<CEGUI::MouseButton> GraphicsView::translateQtMouseButton(Qt::MouseButton button)
{
    optional<CEGUI::MouseButton> ret;

    if (button == Qt::LeftButton) {
        ret = CEGUI::MouseButton::LeftButton;
    }
    if (button == Qt::RightButton) {
        ret = CEGUI::MouseButton::RightButton;
    }

    return ret;
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    // FIXME: Somehow, if you drag on the Live preview in layout editing on Linux,
    //        it drag moves the whole window

    bool handled = false;

    if (m_injectInput) {
        optional<CEGUI::MouseButton> button = translateQtMouseButton(event->button());

        if (button) {
            handled = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(*button);
        }
    }

    if (!handled) {
        resizable::GraphicsView::mousePressEvent(event);
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    bool handled = false;

    if (m_injectInput) {
        optional<CEGUI::MouseButton> button = translateQtMouseButton(event->button());

        if (button) {
            handled = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(*button);
        }
    }

    if (!handled) {
        resizable::GraphicsView::mouseReleaseEvent(event);
    }
}

optional<CEGUI::Key::Scan> GraphicsView::translateQtKeyboardButton(Qt::Key button)
{
    // Shame this isn't standardised :-/ Was a pain to write down

    if (button == Qt::Key_Escape) {
        return CEGUI::Key::Scan::Escape;
    } else if (button == Qt::Key_Tab) {
        return CEGUI::Key::Scan::Tab;
        // missing Backtab
    } else if (button == Qt::Key_Backspace) {
        return CEGUI::Key::Scan::Backspace;
    } else if (button == Qt::Key_Return || button == Qt::Key_Enter) {
        return CEGUI::Key::Scan::Return;
    } else if (button == Qt::Key_Insert) {
        return CEGUI::Key::Scan::Insert;
    } else if (button == Qt::Key_Delete) {
        return CEGUI::Key::Scan::Delete;
    } else if (button == Qt::Key_Pause) {
        return CEGUI::Key::Scan::Pause;
        // missing Print
    } else if (button == Qt::Key_SysReq) {
        return CEGUI::Key::Scan::SysRq;
    } else if (button == Qt::Key_Home) {
        return CEGUI::Key::Scan::Home;
    } else if (button == Qt::Key_End) {
        return CEGUI::Key::Scan::End;
    } else if (button == Qt::Key_Left) {
        return CEGUI::Key::Scan::ArrowLeft;
    } else if (button == Qt::Key_Up) {
        return CEGUI::Key::Scan::ArrowUp;
    } else if (button == Qt::Key_Right) {
        return CEGUI::Key::Scan::ArrowRight;
    } else if (button == Qt::Key_Down) {
        return CEGUI::Key::Scan::ArrowDown;
    } else if (button == Qt::Key_PageUp) {
        return CEGUI::Key::Scan::PageUp;
    } else if (button == Qt::Key_PageDown) {
        return CEGUI::Key::Scan::PageDown;
    } else if (button == Qt::Key_Shift) {
        return CEGUI::Key::Scan::LeftShift;
    } else if (button == Qt::Key_Control) {
        return CEGUI::Key::Scan::LeftControl;
    } else if (button == Qt::Key_Meta) {
        return CEGUI::Key::Scan::LeftWindows;
    } else if (button == Qt::Key_Alt) {
        return CEGUI::Key::Scan::LeftAlt;
        // missing AltGr
        // missing CapsLock
        // missing NumLock
        // missing ScrollLock
    } else if (button == Qt::Key_F1) {
        return CEGUI::Key::Scan::F1;
    } else if (button == Qt::Key_F2) {
        return CEGUI::Key::Scan::F2;
    } else if (button == Qt::Key_F3) {
        return CEGUI::Key::Scan::F3;
    } else if (button == Qt::Key_F4) {
        return CEGUI::Key::Scan::F4;
    } else if (button == Qt::Key_F5) {
        return CEGUI::Key::Scan::F5;
    } else if (button == Qt::Key_F6) {
        return CEGUI::Key::Scan::F6;
    } else if (button == Qt::Key_F7) {
        return CEGUI::Key::Scan::F7;
    } else if (button == Qt::Key_F8) {
        return CEGUI::Key::Scan::F8;
    } else if (button == Qt::Key_F9) {
        return CEGUI::Key::Scan::F9;
    } else if (button == Qt::Key_F10) {
        return CEGUI::Key::Scan::F10;
    } else if (button == Qt::Key_F11) {
        return CEGUI::Key::Scan::F11;
    } else if (button == Qt::Key_F12) {
        return CEGUI::Key::Scan::F12;
    } else if (button == Qt::Key_F13) {
        return CEGUI::Key::Scan::F13;
    } else if (button == Qt::Key_F14) {
        return CEGUI::Key::Scan::F14;
    } else if (button == Qt::Key_F15) {
        return CEGUI::Key::Scan::F15;
        // missing F16 - F35
        // Qt::Key_Super_L    0x01000053
        // Qt::Key_Super_R    0x01000054
        // Qt::Key_Menu    0x01000055
        // Qt::Key_Hyper_L    0x01000056
        // Qt::Key_Hyper_R    0x01000057
        // Qt::Key_Help    0x01000058
        // Qt::Key_Direction_L    0x01000059
        // Qt::Key_Direction_R    0x01000060
    } else if (button == Qt::Key_Space) {
        return CEGUI::Key::Scan::Space;
        // missing Exclam
        // Qt::Key_QuoteDbl    0x22
        // Qt::Key_NumberSign    0x23
        // Qt::Key_Dollar    0x24
        // Qt::Key_Percent    0x25
        // Qt::Key_Ampersand    0x26
    } else if (button == Qt::Key_Apostrophe) {
        return CEGUI::Key::Scan::Apostrophe;
        // Qt::Key_ParenLeft    0x28
        // Qt::Key_ParenRight    0x29
        // Qt::Key_Asterisk    0x2a
        // Qt::Key_Plus    0x2b
    } else if (button == Qt::Key_Comma) {
        return CEGUI::Key::Scan::Comma;
    } else if (button == Qt::Key_Minus) {
        return CEGUI::Key::Scan::Minus;
    } else if (button == Qt::Key_Period) {
        return CEGUI::Key::Scan::Period;
    } else if (button == Qt::Key_Slash) {
        return CEGUI::Key::Scan::Slash;
    } else if (button == Qt::Key_Backslash) {
        return CEGUI::Key::Scan::Backslash;
    } else if (button == Qt::Key_0) {
        return CEGUI::Key::Scan::Zero;
    } else if (button == Qt::Key_1) {
        return CEGUI::Key::Scan::One;
    } else if (button == Qt::Key_2) {
        return CEGUI::Key::Scan::Two;
    } else if (button == Qt::Key_3) {
        return CEGUI::Key::Scan::Three;
    } else if (button == Qt::Key_4) {
        return CEGUI::Key::Scan::Four;
    } else if (button == Qt::Key_5) {
        return CEGUI::Key::Scan::Five;
    } else if (button == Qt::Key_6) {
        return CEGUI::Key::Scan::Six;
    } else if (button == Qt::Key_7) {
        return CEGUI::Key::Scan::Seven;
    } else if (button == Qt::Key_8) {
        return CEGUI::Key::Scan::Eight;
    } else if (button == Qt::Key_9) {
        return CEGUI::Key::Scan::Nine;
    } else if (button == Qt::Key_Colon) {
        return CEGUI::Key::Scan::Colon;
    } else if (button == Qt::Key_Semicolon) {
        return CEGUI::Key::Scan::Semicolon;
        // missing Key_Less
    } else if (button == Qt::Key_Equal) {
        return CEGUI::Key::Scan::Equals;
        // missing Key_Greater
        // missing Key_Question
    } else if (button == Qt::Key_At) {
        return CEGUI::Key::Scan::At;
    } else if (button == Qt::Key_A) {
        return CEGUI::Key::Scan::A;
    } else if (button == Qt::Key_B) {
        return CEGUI::Key::Scan::B;
    } else if (button == Qt::Key_C) {
        return CEGUI::Key::Scan::C;
    } else if (button == Qt::Key_D) {
        return CEGUI::Key::Scan::D;
    } else if (button == Qt::Key_E) {
        return CEGUI::Key::Scan::E;
    } else if (button == Qt::Key_F) {
        return CEGUI::Key::Scan::F;
    } else if (button == Qt::Key_G) {
        return CEGUI::Key::Scan::G;
    } else if (button == Qt::Key_H) {
        return CEGUI::Key::Scan::H;
    } else if (button == Qt::Key_I) {
        return CEGUI::Key::Scan::I;
    } else if (button == Qt::Key_J) {
        return CEGUI::Key::Scan::J;
    } else if (button == Qt::Key_K) {
        return CEGUI::Key::Scan::K;
    } else if (button == Qt::Key_L) {
        return CEGUI::Key::Scan::L;
    } else if (button == Qt::Key_M) {
        return CEGUI::Key::Scan::M;
    } else if (button == Qt::Key_N) {
        return CEGUI::Key::Scan::N;
    } else if (button == Qt::Key_O) {
        return CEGUI::Key::Scan::O;
    } else if (button == Qt::Key_P) {
        return CEGUI::Key::Scan::P;
    } else if (button == Qt::Key_Q) {
        return CEGUI::Key::Scan::Q;
    } else if (button == Qt::Key_R) {
        return CEGUI::Key::Scan::R;
    } else if (button == Qt::Key_S) {
        return CEGUI::Key::Scan::S;
    } else if (button == Qt::Key_T) {
        return CEGUI::Key::Scan::T;
    } else if (button == Qt::Key_U) {
        return CEGUI::Key::Scan::U;
    } else if (button == Qt::Key_V) {
        return CEGUI::Key::Scan::V;
    } else if (button == Qt::Key_W) {
        return CEGUI::Key::Scan::W;
    } else if (button == Qt::Key_X) {
        return CEGUI::Key::Scan::X;
    } else if (button == Qt::Key_Y) {
        return CEGUI::Key::Scan::Y;
    } else if (button == Qt::Key_Z) {
        return CEGUI::Key::Scan::Z;
    }

    // The rest are weird keys I refuse to type here
    return nonstd::nullopt;
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    bool handled = false;

    if (m_injectInput) {
        optional<CEGUI::Key::Scan> button = translateQtKeyboardButton((Qt::Key)event->key());

        if (button) {
            handled = CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown(*button);
        }

        QString chars = event->text();
        if (!chars.isEmpty()) {
            handled = handled || CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(chars[0].unicode());
        }
    }

    if (!handled) {
        resizable::GraphicsView::keyPressEvent(event);
    }
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    bool handled = false;

    if (m_injectInput) {
        auto button = translateQtKeyboardButton((Qt::Key)event->key());

        if (button) {
            handled = CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp(*button);
        }
    }

    if (!handled) {
        resizable::GraphicsView::keyPressEvent(event);
    }
}

} // namespace qtgraphics
} // namespace cegui
} // namespace CEED
