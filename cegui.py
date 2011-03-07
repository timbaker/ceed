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

from PySide.QtOpenGL import QGLWidget

import os.path

import PyCEGUI
import PyCEGUIOpenGLRenderer

class QtCEGUILogger(PyCEGUI.Logger):
    """A simple log message redirector (CEGUI->Qt)."""
    # This will allow us to view logs in Qt in the future
    
    def __init__(self):
        super(QtCEGUILogger, self).__init__()
        
    def logEvent(self, message, level):
        print message.c_str()
        print level
        
    def setLogFilename(self, name, append):
        pass

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
        
        self.logger = QtCEGUILogger()
    
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
        
        self.setDefaultResourceGroups()
        
        self.createAllSchemes()
        
        root = PyCEGUI.WindowManager.getSingleton().createWindow("DefaultWindow")
        root.setPosition(PyCEGUI.UVector2(PyCEGUI.UDim(0, 0), PyCEGUI.UDim(0, 0)))
        root.setSize(PyCEGUI.UVector2(PyCEGUI.UDim(1, 0), PyCEGUI.UDim(1, 0)))
        self.system.setGUISheet(root)
        
        wnd = PyCEGUI.WindowManager.getSingleton().createWindow("TaharezLook/FrameWindow")
        wnd.setSize(PyCEGUI.UVector2(PyCEGUI.UDim(0.5, 0), PyCEGUI.UDim(0.5, 0)))
        root.addChild(wnd)
        
        cmb = PyCEGUI.WindowManager.getSingleton().createWindow("WindowsLook/Combobox")
        cmb.setSize(PyCEGUI.UVector2(PyCEGUI.UDim(1, 0), PyCEGUI.UDim(1, 0)))
        wnd.addChild(cmb)
        
        self.item = PyCEGUI.ListboxTextItem("Something Something")
        cmb.addItem(self.item)        
    
    def resizeGL(self, width, height):
        self.system.notifyDisplaySizeChanged(PyCEGUI.Sizef(width, height))
    
    def paintGL(self):
        self.system.renderGUI()
        
        # immediately after rendering, we mark the contents as dirty to force Qt to update this
        # as much as possible
        #
        # TODO: This will strain the CPU and GPU for no good reason, we might want to cap the FPS
        #       or something like that
        self.update()
    