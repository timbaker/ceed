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

#include "qtwidgets.h"

#include <QtEvents>
#include <QFileDialog>
#include <QPushButton>

#include "ui_FileLineEdit.h"
#include "ui_KeySequenceButtonDialog.h"
#include "ui_PenButtonDialog.h"

namespace CEED {
namespace qtwidgets {

FileLineEdit::FileLineEdit(QWidget *parent)
    : QWidget(parent)
{
    m_ui = new Ui_FileLineEdit();
    m_ui->setupUi(this);

    m_filter = "Any file (*.*)";

    m_lineEdit = m_ui->lineEdit;
    m_browseButton = m_ui->browseButton;

    connect(m_browseButton, &QToolButton::pressed, this, &FileLineEdit::slot_browse);

    m_mode = FileLineEdit::ExistingFileMode;
    m_directoryMode = false;

    startDirectory = [](){ return QString(); };
}

void FileLineEdit::setText(const QString &text)
{
    m_lineEdit->setText(text);
}

QString FileLineEdit::text()
{
    return m_lineEdit->text();
}

void FileLineEdit::slot_browse()
{
    QString path;

    if (m_mode == FileLineEdit::ExistingFileMode) {
        path = QFileDialog::getOpenFileName(this,
                                            "Choose a path",
                                            startDirectory(),
                                            m_filter);

    } else if (m_mode == FileLineEdit::NewFileMode) {
        path = QFileDialog::getSaveFileName(this,
                                            "Choose a path",
                                            startDirectory(),
                                            m_filter);

    } else if (m_mode == FileLineEdit::ExistingDirectoryMode) {
        path = QFileDialog::getExistingDirectory(this,
                                                 "Choose a directory",
                                                 startDirectory());
    }

    if (path != "")
        m_lineEdit->setText(path);
}

/////

ColourButton::ColourButton(QWidget *parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);

    // seems to look better on most systems
    // commented out because style sheet color is ignored with it on
//    setFlat(true);

    setColour(QColor(255, 255, 255, 255));

    connect(this, &ColourButton::clicked, this, &ColourButton::slot_clicked);
}

void ColourButton::setColour(const QColor &colour)
{
    if (colour != m_colour) {
        m_colour = colour;
        setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4)")
                      .arg(colour.red())
                      .arg(colour.green())
                      .arg(colour.blue())
                      .arg(colour.alpha()));
        setText(QString("R: %1, G: %2, B: %3, A: %4")
                .arg(colour.red(), 3)
                .arg(colour.green(), 3)
                .arg(colour.blue(), 3)
                .arg(colour.alpha(), 3));
        emit colourChanged(colour);
    }
}

void ColourButton::slot_clicked()
{
    QColor colour = QColorDialog::getColor(m_colour, parentWidget(), "",
                                           QColorDialog::ColorDialogOption::ShowAlphaChannel |
                                           QColorDialog::ColorDialogOption::DontUseNativeDialog);

    if (colour.isValid())
        setColour(colour);
}

/////

PenButton::Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui = new Ui_PenButtonDialog();
    m_ui->setupUi(this);

    m_colour = m_ui->colour;
    m_lineStyle = m_ui->lineStyle;
    m_lineWidth = m_ui->lineWidth;
}

void PenButton::Dialog::setPen(const QPen &pen)
{
    m_colour->setColour(pen.color());

    switch (pen.style()) {
    case Qt::SolidLine:
        m_lineStyle->setCurrentIndex(0);
        break;
    case  Qt::DashLine:
        m_lineStyle->setCurrentIndex(1);
        break;
    case  Qt::DotLine:
        m_lineStyle->setCurrentIndex(2);
        break;
    case  Qt::DashDotLine:
        m_lineStyle->setCurrentIndex(3);
        break;
    case  Qt::DashDotDotLine:
        m_lineStyle->setCurrentIndex(4);
        break;
    default:
        // unknown line style
        Q_ASSERT(false);
        break;
    }

    m_lineWidth->setValue(pen.widthF());
}

QPen PenButton::Dialog::getPen()
{
    QPen ret;

    ret.setColor(m_colour->m_colour);

    Qt::PenStyle style = Qt::SolidLine;
    if (m_lineStyle->currentIndex() == 0)
        style = Qt::SolidLine;
    else if (m_lineStyle->currentIndex() == 1)
        style = Qt::DashLine;
    else if (m_lineStyle->currentIndex() == 2)
        style = Qt::DotLine;
    else if (m_lineStyle->currentIndex() == 3)
        style = Qt::DashDotLine;
    else if (m_lineStyle->currentIndex() == 4)
        style = Qt::DashDotDotLine;
    else
        // unknown combobox index
        Q_ASSERT(false);

    ret.setStyle(style);
    ret.setWidth(m_lineWidth->value());

    return ret;
}

/////

PenButton::PenButton(QWidget *parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    // seems to look better on most systems
//    setFlat(true);
    m_pen.setWidthF(666.0);
    setPen(QPen());

    connect(this, &PenButton::clicked, this, &PenButton::slot_clicked);
}

void PenButton::setPen(const QPen &pen)
{
    if (pen != m_pen) {
        m_pen = pen;

        QString lineStyleStr = "";
        switch (pen.style()) {
        case Qt::SolidLine:
            lineStyleStr = "solid";
            break;
        case Qt::DashLine:
            lineStyleStr = "dash";
            break;
        case Qt::DotLine:
            lineStyleStr = "dot";
            break;
        case Qt::DashDotLine:
            lineStyleStr = "dash dot";
            break;
        case Qt::DashDotDotLine:
            lineStyleStr = "dash dot dot";
            break;
        case Qt::CustomDashLine:
            lineStyleStr = "custom dash";
            break;
        default:
            throw RuntimeError("Unknown pen line style!");
            break;
        }

        /*
            capStyleStr = ""
            if pen.capStyle() == Qt::FlatCap:
                capStyleStr = "flat"
            else if (pen.capStyle() == Qt::RoundCap:
                capStyleStr = "round"
            else if (pen.capStyle() == Qt::SquareCap:
                capStyleStr = "square"
            else:
                raise RuntimeError("Unknown pen cap style!")

            joinStyleStr = ""
            if pen.joinStyle() == Qt::MiterJoin:
                joinStyleStr = "miter"
            else if (pen.joinStyle() == Qt::BevelJoin:
                joinStyleStr = "bevel"
            else if (pen.joinStyle() == Qt::RoundJoin:
                joinStyleStr = "round"
            else if (pen.joinStyle() == Qt::SvgMiterJoin:
                joinStyleStr = "svg miter"
            else:
                raise RuntimeError("Unknown pen join style!")
            */

        setText(QString("line style: %1, width: %2").arg(lineStyleStr).arg(pen.widthF()));
/*
        QColor colour = pen.color();
        setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4)")
                .arg(colour.red())
                .arg(colour.green())
                .arg(colour.blue())
                .arg(colour.alpha()));
*/
        emit penChanged(pen);
    }
}

void PenButton::slot_clicked()
{
    Dialog dialog(this);
    dialog.setPen(m_pen);

    if (dialog.exec() == QDialog::Accepted)
        setPen(dialog.getPen());
}

/////

KeySequenceButton::Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    m_ui = new Ui_KeySequenceButtonDialog();
    m_ui->setupUi(this);

    m_keyCombination = m_ui->keyCombination;
}

void KeySequenceButton::Dialog::setKeySequence(const QKeySequence &keySequence)
{
    m_keySequence = keySequence;
    m_keyCombination->setText(m_keySequence.toString());
}

void KeySequenceButton::Dialog::keyPressEvent(QKeyEvent *event)
{
    setKeySequence(QKeySequence(event->modifiers() | event->key()));
}

/////

KeySequenceButton::KeySequenceButton(QWidget *parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    setKeySequence(QKeySequence(), true);

    connect(this, &KeySequenceButton::clicked, this, &KeySequenceButton::slot_clicked);
}

void KeySequenceButton::setKeySequence(const QKeySequence &keySequence, bool force)
{
    if (force || (keySequence != m_keySequence)) {
        m_keySequence = keySequence;
        setText(keySequence.toString());
        emit keySequenceChanged(keySequence);
    }
}

void KeySequenceButton::slot_clicked()
{
    Dialog dialog(this);
    dialog.setKeySequence(m_keySequence);

    if (dialog.exec() == QDialog::Accepted)
        setKeySequence(dialog.m_keySequence);
}

/////

LineEditWithClearButton::LineEditWithClearButton(QWidget *parent)
    : QLineEdit(parent)
{
    QToolButton* btn = m_button = new QToolButton(this);
    QPixmap icon = QPixmap(":icons/widgets/edit-clear.png");
    btn->setIcon(icon);
    btn->setIconSize(icon.size());
    btn->setCursor(Qt::ArrowCursor);
    btn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    btn->hide();

    connect(btn, &QToolButton::clicked, this, &LineEditWithClearButton::clear);
    connect(this, &LineEditWithClearButton::textChanged, this, &LineEditWithClearButton::updateCloseButton);

    QAction* clearAction = new QAction(this);
    clearAction->setShortcut(QKeySequence("Esc"));
    clearAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
    connect(clearAction, &QAction::triggered, this, &LineEditWithClearButton::clear);
    addAction(clearAction);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; }").arg((btn->sizeHint().width() + frameWidth + 1)));

    QSize minSizeHint = minimumSizeHint();
    setMinimumSize(qMax(minSizeHint.width(), btn->sizeHint().width() + frameWidth * 2 + 2),
                   qMax(minSizeHint.height(), btn->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEditWithClearButton::resizeEvent(QResizeEvent *event)
{
    QSize sz = m_button->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_button->move(rect().right() - frameWidth - sz.width(),
                   (rect().bottom() + 1 - sz.height()) / 2);
}

void LineEditWithClearButton::updateCloseButton(const QString &text)
{
    m_button->setVisible(!text.isEmpty());
}

QBrush getCheckerboardBrush(int halfWidth, int halfHeight, const QColor &firstColour, const QColor &secondColour)
{
    // disallow too large half sizes to prevent crashes in QPainter
    // and slowness in general
    halfWidth = qMin(halfWidth, 1000);
    halfHeight = qMin(halfHeight, 1000);

    QBrush ret;
    QPixmap texture(2 * halfWidth, 2 * halfHeight);
    QPainter painter(&texture);
    painter.fillRect(0, 0, halfWidth, halfHeight, firstColour);
    painter.fillRect(halfWidth, halfHeight, halfWidth, halfHeight, firstColour);
    painter.fillRect(halfWidth, 0, halfWidth, halfHeight, secondColour);
    painter.fillRect(0, halfHeight, halfWidth, halfHeight, secondColour);
    painter.end();
    ret.setTexture(texture);

    return ret;
}


} // namespace qtwidgets
} // namespace CEED
