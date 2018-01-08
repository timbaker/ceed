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

#ifndef CEED_cegui___init___
#define CEED_cegui___init___

#include "CEEDBase.h"

#include <QTime>

#include "delegate_list.h"

#include <functional>

namespace CEED {

namespace mainwindow {
class MainWindow;
}

namespace project {
class Project;
}

namespace cegui {

/*!
\brief RedirectingCEGUILogger

Allows us to register subscribers that want CEGUI log info

    This prevents writing CEGUI.log into CWD and will allow log display inside
    the app in the future

*/
class RedirectingCEGUILogger : public CEGUI::Logger
{
public:
    QList<std::function<void(const QString&, CEGUI::LoggingLevel)>> m_subscribers;

    RedirectingCEGUILogger()
        : CEGUI::Logger()
    {
    }

    void registerSubscriber(std::function<void(const QString&, CEGUI::LoggingLevel level)> subscriber)
    {
        m_subscribers.append(subscriber);
    }

    void logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level) override
    {
        for (auto& sub : m_subscribers) {
            sub(TO_QSTR(message), level);
        }
    }

    void setLogFilename(const CEGUI::String& name, bool append) override
    {
        Q_UNUSED(name)
        Q_UNUSED(append)
        // this is just a NOOP to satisfy CEGUI pure virtual method of the same name
    }
};

/*!
\brief GLContextProvider

Interface that provides a method to make OpenGL context
    suitable for CEGUI the current context.

*/
class GLContextProvider
{
public:
    /**Activates the OpenGL context held by this provider*/
    virtual void makeGLContextCurrent() = 0;
};

/*!
\brief Instance

Encapsulates a running CEGUI instance.

    Right now CEGUI can only be instantiated once because it's full of singletons.
    This might change in the future though...

*/
class Instance
{
public:
    GLContextProvider* m_contextProvider;
    RedirectingCEGUILogger* m_logger;
    bool m_initialised;
    float m_lastRenderTimeDelta;

    Instance(GLContextProvider* contextProvider = nullptr);

    void setGLContextProvider(GLContextProvider* contextProvider)
    {
        /**CEGUI instance might need an OpenGL context provider to make sure the right context is active
        (to load textures, render, ...)

        see GLContextProvider
        */

        m_contextProvider = contextProvider;
    }

    /**Activate the right OpenGL context.

        This is usually called internally and you don't need to worry about it, it generally needs to be called
        before any rendering is done, textures are loaded, etc...
        */
    void makeGLContextCurrent();

    /**Ensures this CEGUI instance is properly initialised, if it's not it initialises it right away.
        */
    void ensureIsInitialised();

    /**Sets given resourceGroup to look into given absoluteDirPath
        */
    void setResourceGroupDirectory(const QString& resourceGroup, const QString& absoluteDirPath);

    /**Puts the resource groups to a reasonable default value.

    ./datafiles followed by the respective folder, same as CEGUI stock datafiles
    */
    void setDefaultResourceGroups();

    void cleanCEGUIResources();

    /**Synchronises the instance with given project, respecting it's paths and resources
    */
    void syncToProject(project::Project* project, mainwindow::MainWindow* mainWindow = nullptr);

    /**Retrieves skins (as strings representing their names) that are available
    from the set of schemes that were loaded.

    see syncToProject
    */
    QStringList getAvailableSkins();

    /**Retrieves fonts (as strings representing their names) that are available
    from the set of schemes that were loaded.

    see syncToProject
    */
    QStringList getAvailableFonts();

    /**Retrieves images (as strings representing their names) that are available
    from the set of schemes that were loaded.

    see syncToProject
    */
    QStringList getAvailableImages();

    /**Retrieves all mappings (string names) of all widgets that can be created

        see syncToProject
    */
    OrderedMap<QString, QStringList> getAvailableWidgetsBySkin();

    /**Renders and retrieves a widget preview image (as QImage).

    This is useful for various widget selection lists as a preview.
    */
    QImage getWidgetPreviewImage(const QString& widgetType, int previewWidth = 128, int previewHeight = 64);
};

} // namespace cegui
} // namespace CEED

#endif
