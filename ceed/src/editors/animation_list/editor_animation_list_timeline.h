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

#ifndef CEED_editors_animation_list_timeline_
#define CEED_editors_animation_list_timeline_

#include "CEEDBase.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QTime>

class QComboBox;

namespace CEED {
namespace editors {
namespace animation_list {
namespace timeline {

// default amount of pixels per second for 100% zoom
const int pixelsPerSecond = 1000;

/*!
\brief TimecodeLabel

Simply displays time position labels depending on the zoom level

*/
class TimecodeLabel : public QGraphicsRectItem
{
public:
#if 0
    range = property(lambda self: self._getRange(),
                     lambda self, value: self._setRange(value));
#endif
    TimecodeLabel(QGraphicsItem* parentItem = nullptr);

    void setRange(qreal range);

    qreal getRange()
    {
        return rect().width() / pixelsPerSecond;
    }

    // {{{ http://code.activestate.com/recipes/66472/ (r1)
    /**A range function, that does accept float increments...*/
    QList<qreal> frange(qreal start, qreal end = qreal(-666.0), qreal inc = qreal(1.0))
    {
        if (end == qreal(-666.0)) {
            end = start + qreal(0.0);
            start = qreal(0.0);
        }

        QList<qreal> L;
        while (true) {
            qreal next = start + L.length() * inc;
            if (inc > 0 && next >= end)
                break;
            else if (inc < 0 && next <= end)
                break;
            L += next;
        }

        return L;
    }
    // end of http://code.activestate.com/recipes/66472/ }}}

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
};

class AffectorTimelineKeyFrame : public QGraphicsRectItem
{
public:
#if 0
    keyFrame = property(lambda self: data(0),
                        lambda self, value: setData(0, value));
#endif
    AffectorTimelineSection* m_timelineSection;
    bool m_moveInProgress;
    float m_oldPosition;
    CEGUI::KeyFrame* m_keyFrame;

    AffectorTimelineKeyFrame(AffectorTimelineSection* timelineSection = nullptr, CEGUI::KeyFrame *keyFrame = nullptr);

    void setKeyFrame(CEGUI::KeyFrame* keyFrame)
    {
        m_keyFrame = keyFrame;
        refresh();
    }

    void refresh();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
};

class AffectorTimelineSection : public QGraphicsRectItem
{
public:
    AnimationTimeline* m_timeline;
    CEGUI::Affector* m_affector;

    AffectorTimelineSection(AnimationTimeline* timeline = nullptr, CEGUI::Affector* affector = nullptr);

    void setAffector(CEGUI::Affector* affector)
    {
        m_affector = affector;
        refresh();
    }

    void refresh();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
};

class AffectorTimelineLabel : public QGraphicsProxyWidget
{
public:
    QComboBox* m_widget;
    CEGUI::Affector* m_affector;

    AffectorTimelineLabel(AnimationTimeline* timeline = nullptr, CEGUI::Affector* affector = nullptr);

    void setAffector(CEGUI::Affector* affector)
    {
        m_affector = affector;
        refresh();
    }

    void refresh();
};

class TimelinePositionBar : public QGraphicsRectItem
{
public:
    AnimationTimeline* m_timeline;

    TimelinePositionBar(AnimationTimeline* timeline);

    void setHeight(qreal height)
    {
        setRect(QRectF(-1, 6, 3, height - 6));
    }

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    QPainterPath shape() const override;
};

struct KeyFrameMoved
{
    int keyFrameIndex;
    int affectorIndex;
    qreal oldPosition;
    qreal newPosition;
};

/**A timeline widget for just one CEGUI animation*/
class AnimationTimeline : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
signals:
    // old timeline position, new timeline position
    void timePositionChanged(qreal oldPos, qreal newPos);

    void keyFramesMoved(const QList<KeyFrameMoved>& list);

public:
    qreal m_playDelta;
    QTime m_lastPlayInjectTime;
    bool m_playing;
    TimecodeLabel* m_timecode;
    TimelinePositionBar* m_positionBar;
    CEGUI::Animation* m_animation;
    CEGUI::AnimationInstance* m_animationInstance;
    bool m_ignoreTimelineChanges;

    // we only inherit from QObject to be able to define QtCore.Signals in our class
    AnimationTimeline(QGraphicsItem* parentItem = nullptr, CEGUI::Animation* animation = nullptr);

    void setAnimation(CEGUI::Animation* animation);

    void refresh();

    void notifyZoomChanged(qreal zoom);

    void setTimelinePosition(qreal position);

    void playTick();

    void play();

    void pause();

    void stop();

private slots:
    void slot_timePositionChanged(qreal oldPosition, qreal newPosition);

public:
    /**Called when mouse is released, grabs all moved keyframes
    and makes a list of old and new positions for them.
    */
    void notifyMouseReleased();
};

} // namespace timeline
} // namespace animation_list
} // namespace editors
} // namespace CEED

#endif
