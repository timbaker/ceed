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

#ifndef CEED_qtwidgets_
#define CEED_qtwidgets_

#include "CEEDBase.h"

/**Contains reusable widgets that I haven't found in Qt for some reason
*/

#include <QColorDialog>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>

#include <functional>

class QComboBox;
class QDoubleSpinBox;
class QToolButton;

class Ui_FileLineEdit;
class Ui_KeySequenceButtonDialog;
class Ui_PenButtonDialog;

namespace CEED {
namespace qtwidgets {

class FileLineEdit : public QWidget
{
public:
    static const int ExistingFileMode = 1;
    static const int NewFileMode = 2;
    static const int ExistingDirectoryMode = 3;

    Ui_FileLineEdit* m_ui;
    QString m_filter;
    QLineEdit* m_lineEdit;
    QToolButton* m_browseButton;
    int m_mode;
    bool m_directoryMode;
    std::function<QString()> startDirectory;

    FileLineEdit(QWidget* parent = nullptr);

    void setText(const QString& text);

    QString text();

    void slot_browse();
};

class ColourButton : public QPushButton
{
    Q_OBJECT
public:
#if 0
    colourChanged = QtCore.Signal(QColor)

    colour = property(fset = lambda button, colour: button.setColour(colour),
                      fget = lambda button: button._colour);
#endif

    QColor m_colour;

    ColourButton(QWidget* parent = nullptr);

    void setColour(const QColor& colour);

signals:
    void colourChanged(const QColor& colour);

private slots:
    void slot_clicked();
};

class PenButton : public QPushButton
{
    Q_OBJECT
public:
    class Dialog : public QDialog
    {
    public:
        Ui_PenButtonDialog* m_ui;
        ColourButton* m_colour;
        QComboBox* m_lineStyle;
        QDoubleSpinBox* m_lineWidth;

        Dialog(QWidget* parent = nullptr);

        void setPen(const QPen& pen);

        QPen getPen();
    };

signals:
    // TODO: This is not implemented at all pretty much
    void penChanged(const QPen& pen);
#if 0
    pen = property(fset = lambda button, pen: button.setPen(pen),
                   fget = lambda button: button._pen);
#endif
public:
    QPen m_pen;

    PenButton(QWidget* parent = nullptr);

    void setPen(const QPen& pen);

private slots:
    void slot_clicked();
};

class KeySequenceButton : public QPushButton
{
    Q_OBJECT
public:
    class Dialog : public QDialog
    {
    public:
        Ui_KeySequenceButtonDialog* m_ui;
        QKeySequence m_keySequence;
        QLineEdit* m_keyCombination;

        Dialog(QWidget* parent = nullptr);

        void setKeySequence(const QKeySequence& keySequence);

        void keyPressEvent(QKeyEvent* event) override;
    };

signals:
    void keySequenceChanged(const QKeySequence& keySequence);
#if 0
    keySequence = property(fset = lambda button, keySequence: button.setKeySequence(keySequence),
                           fget = lambda button: button._keySequence);
#endif
public:
    QKeySequence m_keySequence;

    KeySequenceButton(QWidget* parent = nullptr);

    void setKeySequence(const QKeySequence& keySequence, bool force = false);

private slots:
    void slot_clicked();
};

/*!
\brief LineEditWithClearButton

A QLineEdit with an inline clear button.

    Hitting Escape in the line edit clears it.

    Based on http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button/

*/
class LineEditWithClearButton : public QLineEdit
{
    Q_OBJECT
public:
    QToolButton* m_button;

    LineEditWithClearButton(QWidget* parent = nullptr);

    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateCloseButton(const QString& text);
};

/**Small helper function that generates a brush usually seen in graphics
editing tools. The checkerboard brush that draws background seen when
edited images are transparent
*/
QBrush getCheckerboardBrush(int halfWidth = 5, int halfHeight = 5,
                          const QColor& firstColour = QColor(Qt::darkGray),
                          const QColor& secondColour = QColor(Qt::gray));

} // namespace qtwidgets
} // namespace CEED

#endif
