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

#ifndef CEED_action___init___
#define CEED_action___init___

#include "CEEDBase.h"

/** Uses the declaration API to declare generic actions in the application.

Also provides ConnectionGroup, which can be used to make mass connecting and
disconnecting more convenient.
*/

#include "declaration.h"

#include "editors/imageset/editor_imageset_action_decl.h"
#include "editors/layout/editor_layout_action_decl.h"
#include "editors/looknfeel/editor_looknfeel_action_decl.h"

#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QMetaMethod>

#include <functional>

namespace CEED {
namespace action {

class ActionManager;

/** Very lightweight holding object that represents a signal slot connection.
Receiver is the Python callable (free function, bound method, lambda function, anything really)

Not intended for use outside the ConnectionGroup class, should be considered internal!
*/
class Connection : public QObject
{
public:
    declaration::Action* m_action;
    QString m_signalName;
    Qt::ConnectionType m_connectionType;
    QMetaObject::Connection m_connection;
    bool m_connected;

    virtual void connect() = 0;

    void disconnect()
    {
        if (!m_connected)
            throw RuntimeError("Can't disconnect this Connection, it isn't connected at the moment.");

        QObject::disconnect(m_connection);
    //    QObject::disconnect(m_action, m_signalName.toStdString().c_str(), this, SLOT(slotFunc()));
    //    signal = getattr(m_action, m_signalName);
    //    signal.disconnect(m_receiver);

        m_connected = false;
    }
};

// You win again, templates!
class ConnectionVoid : public Connection
{
    Q_OBJECT
public:
    std::function<void()> m_receiver;

    ConnectionVoid(declaration::Action* action, std::function<void()> receiver,
                const QString& signalName = "triggered()",
               Qt::ConnectionType connectionType = Qt::AutoConnection)
    {
        m_action = action;
        m_signalName = signalName;
        m_receiver = receiver;
        m_connectionType = connectionType;

        m_connected = false;

        int index = action->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalName.toStdString().c_str()));
        if (index == -1) {
            throw RuntimeError(QString("Given action doesn't have signal called '%1'!").arg(m_signalName));
        }
    }

    void connect() override
    {
        if (m_connected)
            throw RuntimeError("This Connection was already connected!");

        int index = m_action->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(m_signalName.toStdString().c_str()));
        QMetaMethod method = m_action->metaObject()->method(index);

        index = metaObject()->indexOfSlot(QMetaObject::normalizedSignature("slotFunc()"));
        QMetaMethod method2 = metaObject()->method(index);

//   m_connection = QObject::connect(m_action, QMetaObject::normalizedSignature(m_signalName.toStdString().c_str()), SLOT(slotFunc()), m_connectionType);
        m_connection = QObject::connect(m_action, method, this, method2, m_connectionType);

//            signal = getattr(m_action, m_signalName);
//            signal.connect(m_receiver, m_connectionType);

        m_connected = true;
    }

private slots:
    void slotFunc()
    {
        m_receiver();
    }
};

class ConnectionBool : public Connection
{
    Q_OBJECT
public:
    std::function<void(bool)> m_receiver;

    ConnectionBool(declaration::Action* action, std::function<void(bool)> receiver,
                   const QString& signalName = "toggled(bool)",
                   Qt::ConnectionType connectionType = Qt::AutoConnection)
    {
        m_action = action;
        m_signalName = signalName;
        m_receiver = receiver;
        m_connectionType = connectionType;

        m_connected = false;

        int index = action->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalName.toStdString().c_str()));
        if (index == -1) {
            throw RuntimeError(QString("Given action doesn't have signal called '%1'!").arg(m_signalName));
        }
    }

    void connect() override
    {
        if (m_connected)
            throw RuntimeError("This Connection was already connected!");

        int index = m_action->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(m_signalName.toStdString().c_str()));
        QMetaMethod method = m_action->metaObject()->method(index);

        index = metaObject()->indexOfSlot(QMetaObject::normalizedSignature("slotFunc(bool)"));
        QMetaMethod method2 = metaObject()->method(index);

//   m_connection = QObject::connect(m_action, QMetaObject::normalizedSignature(m_signalName.toStdString().c_str()), SLOT(slotFunc()), m_connectionType);
        m_connection = QObject::connect(m_action, method, this, method2, m_connectionType);

//            signal = getattr(m_action, m_signalName);
//            signal.connect(m_receiver, m_connectionType);

        m_connected = true;
    }

private slots:
    void slotFunc(bool b)
    {
       m_receiver(b);
    }
};

/*!
\brief ConnectionGroup

This object allows you to group signal slot connections and
    disconnect them and connect them again en masse.

    Very useful when switching editors

*/
class ConnectionGroup
{
public:
    ActionManager* m_actionManager;
    QList<Connection*> m_connections;

    ConnectionGroup(ActionManager* actionManager = nullptr)
        : m_actionManager(actionManager)
    {
    }

    // allow adding actions by their full names/paths for convenience
    Connection* add(const QString& path, std::function<void()> receiver,
                    const QString& signalName = "triggered()",
                    Qt::ConnectionType connectionType = Qt::AutoConnection);

    Connection* add(const QString& path, std::function<void(bool)> receiver,
                    const QString& signalName = "triggered()",
                    Qt::ConnectionType connectionType = Qt::AutoConnection);

    Connection* add(declaration::Action* action, std::function<void()> receiver,
                    const QString& signalName = "triggered()",
                    Qt::ConnectionType connectionType = Qt::AutoConnection);

    Connection* add(declaration::Action* action, std::function<void(bool)> receiver,
                    const QString& signalName = "toggled(bool)",
                    Qt::ConnectionType connectionType = Qt::AutoConnection);

    void remove(Connection* connection, bool ensureDisconnected = true);

    void connectAll(bool skipConnected = true);

    void disconnectAll(bool skipDisconnected = true);
};

/*!
\brief ActionManager

This is the CEED's action manager, all the "global" actions are declared in it.

    Includes general actions (like Quit, Undo & Redo, File Open, etc...) and also editor specific
    actions (layout align left, ...) - you should use ConnectionGroup for these to connect them when
    your editor is activated and disconnect them when it's deactivated.

    See ConnectionGroup

*/
class ActionManager : public declaration::ActionManager
{
public:
    static ActionManager* instance;

    ActionManager(mainwindow::MainWindow *mainWindow, settings::Settings *settings);
};

/**This is a convenience method to make action retrieval easier
"*/
declaration::Action* getAction(const QString& path);

} // namesapce action
} // namespace CEED

#endif
