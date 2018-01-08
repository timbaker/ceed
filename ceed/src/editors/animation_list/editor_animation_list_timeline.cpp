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

#include "editors/animation_list/editor_animation_list_timeline.h"

#include <QApplication>
#include <QComboBox>
#include <QGraphicsScene>
#include <QTimer>

namespace CEED {
namespace editors {
namespace animation_list {
namespace timeline {

TimecodeLabel::TimecodeLabel(QGraphicsItem *parentItem)
    : QGraphicsRectItem(parentItem)
{
    setRange(1);

    setFlags(QGraphicsItem::ItemIsFocusable);
}

void TimecodeLabel::setRange(qreal range)
{
    Q_ASSERT(range > 0);

    setRect(QRectF(0, 0, range * pixelsPerSecond, 15));
}

void TimecodeLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //super(TimecodeLabel, self).paint(painter, option, widget)

    painter->save();

    QTransform transform = painter->worldTransform();
    qreal xScale = transform.m11();

    Q_ASSERT(xScale > 0);

    qreal minLabelWidth = 50;
    qreal maxLabelsPerSecond = pixelsPerSecond * xScale / minLabelWidth;

    QMap<qreal,bool> timeLabels;

    // always draw start and end
    timeLabels[0] = true;
    timeLabels[getRange()] = true;

    if (maxLabelsPerSecond >= 0.1) {
        for (qreal i : frange(0, getRange(), 10)) {
            // 0, 10, 20, ...
            timeLabels[i] = true;
        }
    }

    if (maxLabelsPerSecond >= 0.2) {
        for (qreal i : frange(0, getRange(), 5)) {
            // 0, 5, 10, ...
            timeLabels[i] = true;
        }
    }

    if (maxLabelsPerSecond >= 1.0) {
        for (qreal i : frange(0, getRange())) {
            // 0, 1, 2, ...
            timeLabels[i] = true;
        }
    }

    if (maxLabelsPerSecond >= 2.0) {
        for (qreal i : frange(0, getRange(), 0.5)) {
            // 0, 0.5, 1.0, ...
            timeLabels[i] = true;
        }
    }

    if (maxLabelsPerSecond >= 10.0) {
        for (qreal i : frange(0, getRange(), 0.1)) {
            // 0, 0.1, 0.2, ...
            timeLabels[i] = true;
        }
    }

    painter->scale(1.0 / xScale, 1);
    QFont font;
    font.setPixelSize(8);
    painter->setFont(font);
    painter->setPen(QPen());

    for (qreal key : timeLabels.keys()) {
        if (timeLabels[key]) {
            // round(key, 6) because we want at most 6 digits, %g to display it as compactly as possible
            painter->drawText(key * pixelsPerSecond * xScale - minLabelWidth * 0.5, 0, minLabelWidth, 10, Qt::AlignHCenter | Qt::AlignBottom, QString("%1").arg(key, 6));
            painter->drawLine(QPointF(key * pixelsPerSecond * xScale, 10), QPointF(pixelsPerSecond * key * xScale, 15));
        }
    }

    painter->restore();
}

/////

AffectorTimelineKeyFrame::AffectorTimelineKeyFrame(AffectorTimelineSection *timelineSection, CEGUI::KeyFrame *keyFrame)
    : QGraphicsRectItem(timelineSection)
{
    m_timelineSection = timelineSection;

    setKeyFrame(keyFrame);

    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIgnoresTransformations |
             QGraphicsItem::ItemSendsGeometryChanges);

    // the parts between keyframes are z-value 0 so this makes key frames always "stand out"
    setZValue(1);
    setRect(0, 1, 15, 29);

    //palette = QApplication.palette()
    setPen(QPen(QColor(Qt::GlobalColor::transparent)));
    setBrush(QBrush(QColor(Qt::GlobalColor::lightGray)));

    m_moveInProgress = false;
    m_oldPosition = 0.0;
}

/////

void AffectorTimelineKeyFrame::refresh()
{
    setPos(m_keyFrame ? pixelsPerSecond * m_keyFrame->getPosition() : 0.0, 0.0);
}

void AffectorTimelineKeyFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    //super(AffectorTimelineKeyFrame, self).paint(painter, option, widget)
    painter->save();

    QPalette palette = QApplication::palette();

    painter->setPen(pen());
    painter->setBrush(brush());

    painter->drawRect(rect());

    // draw the circle representing the keyframe
    setPen(QPen(QColor(Qt::GlobalColor::transparent)));
    painter->setBrush(QBrush(palette.color(QPalette::Normal, QPalette::ButtonText)));
    painter->drawEllipse(QPointF(7.5, 14.5), 4, 4);

    painter->restore();
}

QVariant AffectorTimelineKeyFrame::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        //palette = QApplication.palette()
        setBrush(value.toBool() ? QColor(114, 159, 207) : QColor(Qt::GlobalColor::lightGray));
#if 0 // FIXME, update() instead
        parentItem()->prepareGeometryChange();
#endif
        return QGraphicsRectItem::itemChange(change, value);
    }

    else if (change == QGraphicsItem::ItemPositionChange) {
        qreal newPosition = qMax(qreal(0.0), qMin(value.toPointF().x() / pixelsPerSecond, (qreal)m_keyFrame->getParent()->getParent()->getDuration()));

        if (!m_moveInProgress) {
            m_moveInProgress = true;
            m_oldPosition = m_keyFrame->getPosition();
        }
        while (m_keyFrame->getParent()->hasKeyFrameAtPosition(newPosition)) {
            // FIXME: we want newPosition * epsilon, but how do we get epsilon in python?
            newPosition += 0.00001;
        }
#if 0 // FIXME, update() instead
        m_timelineSection->prepareGeometryChange();
#endif
        m_keyFrame->moveToPosition(newPosition);

        return QPointF(newPosition * pixelsPerSecond, pos().y());
    }

    return QGraphicsRectItem::itemChange(change, value);
}

AffectorTimelineSection::AffectorTimelineSection(AnimationTimeline *timeline, CEGUI::Affector *affector)
    : QGraphicsRectItem(timeline)
{
    m_timeline = timeline;

    setFlags(QGraphicsItem::ItemIsFocusable);

    //palette = QApplication.palette()
    setPen(QPen(QColor(Qt::GlobalColor::lightGray)));
    setBrush(QBrush(QColor(Qt::GlobalColor::transparent)));

    setAffector(affector);
}

void AffectorTimelineSection::refresh()
{
    setRect(0, 0, m_timeline->m_animation->getDuration() * pixelsPerSecond, 30);

    for (QGraphicsItem* item : childItems()) {
        // refcount drops and python should destroy that item
        scene()->removeItem(item);
    }

    if (m_affector == nullptr)
        return;

    int i = 0;
    while (i < m_affector->getNumKeyFrames()) {
        new AffectorTimelineKeyFrame(this, m_affector->getKeyFrameAtIdx(i));

        // affector timeline key frame will refresh itself automatically to the right position
        // upon construction

        i += 1;
    }
}

void AffectorTimelineSection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);

    painter->save();

    //palette = QApplication.palette()

    QTransform transform = painter->worldTransform();
    qreal xScale = transform.m11();

    painter->setRenderHint(QPainter::RenderHint::Antialiasing, true);

    if (m_affector->getNumKeyFrames() == 0) {
        painter->restore();
        return;
    }

    auto drawArrow = [=](qreal x, qreal y)
    {
        painter->setPen(Qt::SolidLine);
        painter->drawLine(QPointF(x, y), QPointF(x - 3 / xScale, y - 3));
        painter->drawLine(QPointF(x, y), QPointF(x - 3 / xScale, y + 3));
    };

    CEGUI::KeyFrame* previousKeyFrame = m_affector->getKeyFrameAtIdx(0);
    int i = 1;
    while (i < m_affector->getNumKeyFrames()) {
        CEGUI::KeyFrame* keyFrame = m_affector->getKeyFrameAtIdx(i);

        // previousKeyFrame -------- line we will draw -------> keyFrame

        float previousPosition = previousKeyFrame->getPosition() * pixelsPerSecond;
        float currentPosition = keyFrame->getPosition() * pixelsPerSecond;
        float span = currentPosition - previousPosition;
        Q_ASSERT(span >= 0);

        bool selected = false;
        for (QGraphicsItem* item_ : childItems()) {
            auto item = dynamic_cast<AffectorTimelineKeyFrame*>(item_);
            if (item->m_keyFrame->getPosition() == previousKeyFrame->getPosition()) {
                selected = item->isSelected();
                break;
            }
        }

        painter->setPen(QColor(Qt::GlobalColor::transparent));
        painter->setBrush(QBrush(selected ? QColor(114, 159, 207) : QColor(255, 255, 255)));
        painter->drawRect(QRectF(previousPosition, 1, span, 29));
        painter->setBrush(QBrush());

        qreal lineStartPosition = qMin(previousPosition + 15 / (float)xScale, currentPosition);

        if (keyFrame->getProgression() == CEGUI::KeyFrame::Progression::P_Linear) {
            // just a straight line in this case
            painter->setPen(Qt::DashLine);
            painter->drawLine(QPointF(lineStartPosition, 15), QPointF(currentPosition, 15));
            drawArrow(currentPosition, 15);

        } else if (keyFrame->getProgression() == CEGUI::KeyFrame::Progression::P_QuadraticAccelerating) {
            QPainterPath path;
            path.moveTo(lineStartPosition, 27);
            path.quadTo(previousPosition + span * 3/4, 30, currentPosition, 5);
            painter->setPen(Qt::DashLine);
            painter->drawPath(path);

            drawArrow(currentPosition, 5);

        } else if (keyFrame->getProgression() == CEGUI::KeyFrame::Progression::P_QuadraticDecelerating) {
            QPainterPath path;
            path.moveTo(lineStartPosition, 3);
            path.quadTo(previousPosition + span * 3/4, 0, currentPosition, 25);
            painter->setPen(Qt::DashLine);
            painter->drawPath(path);

            drawArrow(currentPosition, 25);

        } else if (keyFrame->getProgression() == CEGUI::KeyFrame::Progression::P_Discrete) {
            painter->setPen(Qt::DashLine);
            painter->drawLine(QPointF(lineStartPosition, 27), QPointF(previousPosition + span / 2, 27));
            painter->drawLine(QPointF(previousPosition + span / 2, 5), QPointF(previousPosition + span / 2, 27));
            painter->drawLine(QPointF(previousPosition + span / 2, 5), QPointF(currentPosition, 5));

            drawArrow(currentPosition, 5);
        } else {
            // wrong progression?
            Q_ASSERT(false);
        }

        previousKeyFrame = keyFrame;
        i += 1;
    }

    painter->restore();
}

/////

AffectorTimelineLabel::AffectorTimelineLabel(AnimationTimeline *timeline, CEGUI::Affector *affector)
    : QGraphicsProxyWidget(timeline)
{
    setFlags(QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemIgnoresTransformations);

    m_widget = new QComboBox();
    m_widget->setEditable(true);
    m_widget->resize(150, 25);
    setWidget(m_widget);

    setAffector(affector);
}

void AffectorTimelineLabel::refresh()
{
    m_widget->setEditText(TO_QSTR(m_affector->getTargetProperty()));
}

/////

TimelinePositionBar::TimelinePositionBar(AnimationTimeline *timeline)
    : QGraphicsRectItem(timeline)
{
    m_timeline = timeline;

    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIgnoresTransformations |
             QGraphicsItem::ItemSendsGeometryChanges);

    setPen(QPen(QColor(Qt::GlobalColor::transparent)));
    setBrush(QBrush(QColor(255, 0, 0, 128)));

    // keep on top of everything
    setZValue(1);

    setHeight(1);

    setCursor(Qt::SplitHCursor);
}

QVariant TimelinePositionBar::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        qreal oldPosition = pos().x() / pixelsPerSecond;
        qreal newPosition = value.toPointF().x() / pixelsPerSecond;

        qreal animationDuration = m_timeline->m_animation ? m_timeline->m_animation->getDuration() : 0.0;

        newPosition = qMax(qreal(0.0), qMin(newPosition, animationDuration));

        emit m_timeline->timePositionChanged(oldPosition, newPosition);
        return QPointF(newPosition * pixelsPerSecond, pos().y());
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void TimelinePositionBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);

    painter->save();

    painter->setPen(pen());
    painter->setBrush(brush());
    QVector<QPointF> points = {QPointF(-6, 0), QPointF(6, 0),
                        QPointF(1, 6), QPointF(-1, 6) };
    painter->drawConvexPolygon(points);

    painter->restore();
}

QPainterPath TimelinePositionBar::shape() const
{
    QPainterPath ret;
    // the arrow on top so that it's easier to grab for for usability
    ret.addRect(QRectF(-6, 0, 12, 6));
    // the rest
    ret.addRect(rect());

    return ret;
}

/////

AnimationTimeline::AnimationTimeline(QGraphicsItem *parentItem, CEGUI::Animation *animation)
    : QGraphicsRectItem(parentItem)
    , QObject()
{
    m_playDelta = 1.0 / 60.0;
    m_lastPlayInjectTime = QTime::currentTime();
    m_playing = false;

    setFlags(QGraphicsItem::ItemIsFocusable);

    m_timecode = new TimecodeLabel(this);
    m_timecode->setPos(QPointF(0, 0));
    m_timecode->setRange(1);

    m_positionBar = new TimelinePositionBar(this);

    m_animation = nullptr;
    m_animationInstance = nullptr;

    setAnimation(animation);

    m_ignoreTimelineChanges = false;
    connect(this, SIGNAL(timePositionChanged(qreal, qreal)), this, SLOT(slot_timePositionChanged(qreal, qreal)));
}

void AnimationTimeline::setAnimation(CEGUI::Animation *animation)
{
    m_animation = animation;

    if (m_animationInstance != nullptr) {
        CEGUI::AnimationManager::getSingleton().destroyAnimationInstance(m_animationInstance);
        m_animationInstance = nullptr;
    }

    // we only use this animation instance to avoid reimplementing looping modes
    if (m_animation != nullptr) {
        m_animationInstance = CEGUI::AnimationManager::getSingleton().instantiateAnimation(m_animation);
        // we don't want CEGUI's injectTimePulse to step our animation instances, we will step them manually
        m_animationInstance->setAutoSteppingEnabled(false);
    } else {
        // FIXME: Whenever AnimationTimeline is destroyed, one animation instance is leaked!
        m_animationInstance = nullptr;
    }

    refresh();
}

void AnimationTimeline::refresh()
{
    for (QGraphicsItem* item : childItems()) {
        if (item == m_timecode || item == m_positionBar)
            continue;

        // refcount drops and python should destroy that item
        item->setParentItem(nullptr);
        if (scene())
            scene()->removeItem(item);

        delete item;
    }

    if (m_animation == nullptr) {
        m_timecode->setRange(1.0);
        return;
    }

    m_timecode->setRange(m_animation->getDuration());

    int i = 0;
    while (i < m_animation->getNumAffectors()) {
        auto affector = m_animation->getAffectorAtIdx(i);
#if 0 // unused? can't see it in the Python version
        auto label = new AffectorTimelineLabel(this, affector);
        label->setPos(-150, 17 + i * 32 + 2);
        label->setZValue(1);
#endif
        auto section = new AffectorTimelineSection(this, affector);
        section->setPos(QPointF(0, 17 + i * 32));

        i += 1;
    }

    m_positionBar->setHeight(17 + i * 32);
}

void AnimationTimeline::notifyZoomChanged(qreal zoom)
{
    Q_ASSERT(zoom > 0);

    for (QGraphicsItem* item : childItems()) {
        if (dynamic_cast<AffectorTimelineLabel*>(item)) {
            item->setPos(-150 / zoom, item->pos().y());
        }
    }
}

void AnimationTimeline::setTimelinePosition(qreal position)
{
    m_positionBar->setPos(position * pixelsPerSecond, m_positionBar->pos().y());

    if (m_animationInstance != nullptr) {
        m_animationInstance->setPosition(position);
    }
}

void AnimationTimeline::playTick()
{
    if (!m_playing)
        return;

    if (m_animationInstance == nullptr)
        return;

    qreal delta = m_lastPlayInjectTime.msecsTo(QTime::currentTime()) / 1000.0;

    // we get a little CEGUI internals abusive here, brace yourself!
    m_animationInstance->step(delta);
    setTimelinePosition(m_animationInstance->getPosition());
    // end of abuses! :D

    m_lastPlayInjectTime = QTime::currentTime();
    QTimer::singleShot(m_playDelta * 1000, [&](){ playTick(); });
}

void AnimationTimeline::play()
{
    if (m_playing) {
        // if we didn't do this we would spawn multiple playTicks in parallel
        // it would bog the app down to a crawl
        return;
    }

    if (m_animationInstance == nullptr) {
        m_playing = false;
        return;
    }

    m_playing = true;
    m_lastPlayInjectTime = QTime::currentTime();
    m_animationInstance->start();

    playTick();
}

void AnimationTimeline::pause()
{
    if (m_animationInstance == nullptr) {
        m_playing = false;
        return;
    }

    m_animationInstance->togglePause();
    m_playing = m_animationInstance->isRunning();

    playTick();
}

void AnimationTimeline::stop()
{
    if (m_animationInstance == nullptr) {
        m_playing = false;
        return;
    }

    m_playing = false;
    setTimelinePosition(0);
    m_animationInstance->stop();
}

void AnimationTimeline::slot_timePositionChanged(qreal oldPosition, qreal newPosition)
{
    if (m_ignoreTimelineChanges)
        return;

    try {
        m_ignoreTimelineChanges = true;
        setTimelinePosition(newPosition);
    } catch (...) {
        m_ignoreTimelineChanges = false;
        throw;
    }
    m_ignoreTimelineChanges = false;
}

void AnimationTimeline::notifyMouseReleased()
{
    QList<KeyFrameMoved> movedKeyFrames;
    for (QGraphicsItem* item_ : scene()->selectedItems()) {
        if (auto* item = dynamic_cast<AffectorTimelineKeyFrame*>(item_)) {
            if (item->m_moveInProgress) {
                auto affector = item->m_keyFrame->getParent();

                movedKeyFrames += {
                        (int)item->m_keyFrame->getIdxInParent(),
                        (int)affector->getIdxInParent(),
                        item->m_oldPosition,
                        item->m_keyFrame->getPosition()
            };
                item->m_moveInProgress = false;
            }
        }
    }

    if (movedKeyFrames.length() > 0) {
        emit keyFramesMoved(movedKeyFrames);
    }
}


} // namespace timeline
} // namespace animation_list
} // namespace editors
} // namespace CEED
