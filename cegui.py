################################################################################
#   CEED - A unified CEGUI editor
#   Copyright (C) 2011 Martin Preisler <preisler.m@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

from PySide.QtGui import QDockWidget, QLineEdit
from PySide.QtOpenGL import *

from OpenGL.GL import *

import ui.ceguidebuginfo

import os.path
import time

import PyCEGUI
import PyCEGUIOpenGLRenderer

class CEGUIQtLogger(PyCEGUI.Logger):
    """Redirects CEGUI log info to CEGUIWidgetInfo"""

    # This is a separate class from CEGUIWidgetInfo because PySide and PyCEGUI
    # don't like mixing base classes at all
    
    def __init__(self, widgetInfo):
        super(CEGUIQtLogger, self).__init__()
        
        self.widgetInfo = widgetInfo
        
    def logEvent(self, message, level):
        self.widgetInfo.logEvent(message, level)
        
    def setLogFilename(self, name, append):
        pass

class CEGUIDebugInfo(QDockWidget):
    """A debugging/info widget about the embedded CEGUI instance"""
    
    # This will allow us to view logs in Qt in the future
    
    def __init__(self, ceguiWidget):
        super(CEGUIDebugInfo, self).__init__()
        
        self.ceguiWidget = ceguiWidget
        # update FPS and render time very second
        self.boxUpdateInterval = 1
        
        self.ui = ui.ceguidebuginfo.Ui_CEGUIWidgetInfo()
        self.ui.setupUi(self)
        
        self.currentFPSBox = self.findChild(QLineEdit, "currentFPSBox")
        self.currentRenderTimeBox = self.findChild(QLineEdit, "currentRenderTimeBox")
        
        self.errors = 0
        self.errorsBox = self.findChild(QLineEdit, "errorsBox")
        
        self.warnings = 0
        self.warningsBox = self.findChild(QLineEdit, "warningsBox")
        
        self.others = 0
        self.othersBox = self.findChild(QLineEdit, "othersBox")
        
    def logEvent(self, message, level):
        if level == PyCEGUI.LoggingLevel.Errors:
            self.errors += 1
            self.errorsBox.setText(str(self.errors))
            
        elif level == PyCEGUI.LoggingLevel.Warnings:
            self.warnings += 1
            self.warningsBox.setText(str(self.warnings))
        else:
            self.others += 1
            self.othersBox.setText(str(self.others))
        
        print message.c_str()

class CEGUIWidget(QGLWidget):
    """Widget containing and displaying a CEGUI instance via OpenGL"""
    
    # !!! You can only spawn one instance of this widget! You have to reuse the one
    #     you spawned before because we can't run multiple CEGUI instances
    #
    # TODO: We could theoretically split this into CEGUIInstance and CEGUIWidget and
    #       the CEGUI widgets would use the instance's textures and other data.
    #       The CEGUIWidgets would basically be window rendering targets.
    
    def __init__(self):
        super(CEGUIWidget, self).__init__()
        
        self.debugInfo = CEGUIDebugInfo(self)
        #self.logger = CEGUIQtLogger(self.debugInfo)
        self.glInit()
    
    def __del__(self):
        #PyCEGUIOpenGLRenderer.OpenGLRenderer.destroySystem()
        pass
    
    def setResourceGroupDirectory(self, resourceGroup, absolutePath):
        rp = PyCEGUI.System.getSingleton().getResourceProvider()
 
        rp.setResourceGroupDirectory(resourceGroup, absolutePath)
    
    def setDefaultResourceGroups(self):
        # reasonable default directories
        defaultBaseDirectory = os.path.join(os.path.curdir, "datafiles")
        
        self.setResourceGroupDirectory("imagesets",
                                       os.path.join(defaultBaseDirectory, "imagesets"))
        self.setResourceGroupDirectory("fonts",
                                       os.path.join(defaultBaseDirectory, "fonts"))
        self.setResourceGroupDirectory("schemes",
                                       os.path.join(defaultBaseDirectory, "schemes"))
        self.setResourceGroupDirectory("looknfeels",
                                       os.path.join(defaultBaseDirectory, "looknfeel"))
        self.setResourceGroupDirectory("layouts",
                                       os.path.join(defaultBaseDirectory, "layouts"))
        
        # all this will never be set to anything else again
        PyCEGUI.ImageManager.setImagesetDefaultResourceGroup("imagesets")
        PyCEGUI.Font.setDefaultResourceGroup("fonts")
        PyCEGUI.Scheme.setDefaultResourceGroup("schemes")
        PyCEGUI.WidgetLookManager.setDefaultResourceGroup("looknfeels")
        PyCEGUI.WindowManager.setDefaultResourceGroup("layouts")
        
        parser = self.system.getXMLParser()
        if parser.isPropertyPresent("SchemaDefaultResourceGroup"):
            parser.setProperty("SchemaDefaultResourceGroup", "schemas")
            
    def syncToProject(self, project):
        # destroy all previous resources (if any)
        PyCEGUI.ImageManager.getSingleton().destroyAll()
        PyCEGUI.FontManager.getSingleton().destroyAll()
        PyCEGUI.SchemeManager.getSingleton().destroyAll()
        #PyCEGUI.WidgetLookManager.getSingleton().destroyAll()
        PyCEGUI.WindowManager.getSingleton().destroyAllWindows()
        
        self.setResourceGroupDirectory("imagesets", project.getAbsolutePathOf(project.imagesetsPath))
        self.setResourceGroupDirectory("fonts", project.getAbsolutePathOf(project.fontsPath))
        self.setResourceGroupDirectory("schemes", project.getAbsolutePathOf(project.schemesPath))
        self.setResourceGroupDirectory("looknfeels", project.getAbsolutePathOf(project.looknfeelsPath))
        self.setResourceGroupDirectory("layouts", project.getAbsolutePathOf(project.layoutsPath))
        
        self.createAllSchemes()
        
    def createAllSchemes(self):
        # I think just creating the schemes should be alright, schemes will
        # case other resources to be loaded as well
        PyCEGUI.SchemeManager.getSingleton().createAll("*.scheme", "schemes")
    
    def getAvailableSkins(self):
        skins = []

        i = PyCEGUI.WindowFactoryManager.getSingleton().getFalagardMappingIterator()

        while not i.isAtEnd():
            current_skin = i.getCurrentValue().d_windowType.split('/')[0]
            if current_skin not in skins:
                skins.append(current_skin)

            i.next()

        skins.sort()

        return skins
    
    def getAvailableFonts(self):
        fonts = []
        font_iter = PyCEGUI.FontManager.getSingleton().getIterator()
        while not font_iter.isAtEnd():
            fonts.append(font_iter.getCurrentValue().getName())
            font_iter.next()

        fonts.sort()

        return fonts
        
    def initializeGL(self):
        self.renderer = PyCEGUIOpenGLRenderer.OpenGLRenderer.bootstrapSystem()
        self.system = PyCEGUI.System.getSingleton()
        self.lastRenderTime = time.time()
        self.lastBoxUpdateTime = time.time() - self.debugInfo.boxUpdateInterval
        
        self.setDefaultResourceGroups()
        
    def resizeGL(self, width, height):
        self.system.notifyDisplaySizeChanged(PyCEGUI.Sizef(width, height))
    
    def paintGL(self):
        renderStartTime = time.time()
        
        glClearColor(0, 0, 0, 1)
        glClear(GL_COLOR_BUFFER_BIT)

        # signalRedraw is called to work around potential issues with dangling
        # references in the rendering code for some versions of CEGUI.
        self.system.signalRedraw()
        self.system.renderGUI()
        
        afterRenderTime = time.time()
        renderTime = afterRenderTime - renderStartTime
        
        fpsInverse = afterRenderTime - self.lastRenderTime
        if fpsInverse <= 0:
            fpsInverse = 1

        self.system.injectTimePulse(fpsInverse)
        
        self.lastRenderTime = afterRenderTime
        
        if afterRenderTime - self.lastBoxUpdateTime >= self.debugInfo.boxUpdateInterval:
            self.lastBoxUpdateTime = afterRenderTime
        
            self.debugInfo.currentRenderTimeBox.setText(str(renderTime))
            self.debugInfo.currentFPSBox.setText(str(1.0 / fpsInverse))
        
        # immediately after rendering, we mark the contents as dirty to force Qt to update this
        # as much as possible
        #
        # TODO: This will strain the CPU and GPU for no good reason, we might want to cap the FPS
        #       or something like that
        self.update()
    