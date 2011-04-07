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

from PySide.QtCore import *
from PySide.QtGui import *
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
    
    def __init__(self, ceguiContainerWidget):
        super(CEGUIDebugInfo, self).__init__()
        
        self.ceguiContainerWidget = ceguiContainerWidget
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
        
        self.initialised = False
        self.injectInput = False
        # we might want mouse events
        self.setMouseTracking(True)
        # we might want key events
        self.setFocusPolicy(Qt.ClickFocus)
        
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
        self.makeCurrent()
        
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
        if not self.initialised:
            self.renderer = PyCEGUIOpenGLRenderer.OpenGLRenderer.bootstrapSystem()
            self.system = PyCEGUI.System.getSingleton()
            
            self.setDefaultResourceGroups()
            
            self.initialised = True
        
        self.lastRenderTime = time.time()
        #self.lastBoxUpdateTime = time.time() - self.debugInfo.boxUpdateInterval
        self.lastBoxUpdateTime = time.time()
        
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
        
        #if afterRenderTime - self.lastBoxUpdateTime >= self.debugInfo.boxUpdateInterval:
        #    self.lastBoxUpdateTime = afterRenderTime
        
            #self.debugInfo.currentRenderTimeBox.setText(str(renderTime))
            #self.debugInfo.currentFPSBox.setText(str(1.0 / fpsInverse))
        
        # immediately after rendering, we mark the contents as dirty to force Qt to update this
        # as much as possible
        #
        # TODO: This will strain the CPU and GPU for no good reason, we might want to cap the FPS
        #       or something like that
        self.update()
    
    def mouseMoveEvent(self, event):
        handled = False
        
        if self.injectInput:
            handled = PyCEGUI.System.getSingleton().injectMousePosition(event.x(), event.y())
        
        if not handled:    
            super(CEGUIWidget, self).mouseMoveEvent(event)
        
    def translateQtMouseButton(self, button):
        ret = None
        
        if button == Qt.LeftButton:
            ret = PyCEGUI.MouseButton.LeftButton
        if button == Qt.RightButton:
            ret = PyCEGUI.MouseButton.RightButton
            
        return ret
    
    def mousePressEvent(self, event):
        handled = False
        
        if self.injectInput:
            button = self.translateQtMouseButton(event.button())

            if button != None:
                handled = PyCEGUI.System.getSingleton().injectMouseButtonDown(button)
                
        if not handled:
            super(CEGUIWidget, self).mousePressEvent(event)
                
    def mouseReleaseEvent(self, event):
        handled = False
        
        if self.injectInput:
            button = self.translateQtMouseButton(event.button())
            
            if button != None:
                handled = PyCEGUI.System.getSingleton().injectMouseButtonUp(button)
                
        if not handled:
            super(CEGUIWidget, self).mouseReleaseEvent(event)
    
    def translateQtKeyboardButton(self, button):
        # Shame this isn't standardised :-/ Was a pain to write down
        
        if button == Qt.Key_Escape:
            return PyCEGUI.Key.Escape
        elif button == Qt.Key_Tab:
            return PyCEGUI.Key.Tab
        # missing Backtab
        elif button == Qt.Key_Backspace:
            return PyCEGUI.Key.Backspace
        elif button in [Qt.Key_Return, Qt.Key_Enter]:
            return PyCEGUI.Key.Return
        elif button == Qt.Key_Insert:
            return PyCEGUI.Key.Insert
        elif button == Qt.Key_Delete:
            return PyCEGUI.Key.Delete
        elif button == Qt.Key_Pause:
            return PyCEGUI.Key.Pause
        # missing Print
        elif button == Qt.Key_SysReq:
            return PyCEGUI.Key.SysRq
        elif button == Qt.Key_Home:
            return PyCEGUI.Key.Home
        elif button == Qt.Key_End:
            return PyCEGUI.Key.End
        elif button == Qt.Key_Left:
            return PyCEGUI.Key.ArrowLeft
        elif button == Qt.Key_Up:
            return PyCEGUI.Key.ArrowUp
        elif button == Qt.Key_Right:
            return PyCEGUI.Key.ArrowRight
        elif button == Qt.Key_Down:
            return PyCEGUI.Key.ArrowDown
        elif button == Qt.Key_PageUp:
            return PyCEGUI.Key.PageUp
        elif button == Qt.Key_PageDown:
            return PyCEGUI.Key.PageDown
        elif button == Qt.Key_Shift:
            return PyCEGUI.Key.LeftShift
        elif button == Qt.Key_Control:
            return PyCEGUI.Key.LeftControl
        elif button == Qt.Key_Meta:
            return PyCEGUI.Key.LeftWindows
        elif button == Qt.Key_Alt:
            return PyCEGUI.Key.LeftAlt
        # missing AltGr
        # missing CapsLock
        # missing NumLock
        # missing ScrollLock
        elif button == Qt.Key_F1:
            return PyCEGUI.Key.F1
        elif button == Qt.Key_F2:
            return PyCEGUI.Key.F2
        elif button == Qt.Key_F3:
            return PyCEGUI.Key.F3
        elif button == Qt.Key_F4:
            return PyCEGUI.Key.F4
        elif button == Qt.Key_F5:
            return PyCEGUI.Key.F5
        elif button == Qt.Key_F6:
            return PyCEGUI.Key.F6
        elif button == Qt.Key_F7:
            return PyCEGUI.Key.F7
        elif button == Qt.Key_F8:
            return PyCEGUI.Key.F8
        elif button == Qt.Key_F9:
            return PyCEGUI.Key.F9
        elif button == Qt.Key_F10:
            return PyCEGUI.Key.F10
        elif button == Qt.Key_F11:
            return PyCEGUI.Key.F11
        elif button == Qt.Key_F12:
            return PyCEGUI.Key.F12
        elif button == Qt.Key_F13:
            return PyCEGUI.Key.F13
        elif button == Qt.Key_F14:
            return PyCEGUI.Key.F14
        elif button == Qt.Key_F15:
            return PyCEGUI.Key.F15
        # missing F16 - F35
        # Qt::Key_Super_L    0x01000053     
        # Qt::Key_Super_R    0x01000054     
        # Qt::Key_Menu    0x01000055     
        # Qt::Key_Hyper_L    0x01000056     
        # Qt::Key_Hyper_R    0x01000057     
        # Qt::Key_Help    0x01000058     
        # Qt::Key_Direction_L    0x01000059     
        # Qt::Key_Direction_R    0x01000060
        elif button == Qt.Key_Space:
            return PyCEGUI.Key.Space
        # missing Exclam
        # Qt::Key_QuoteDbl    0x22     
        # Qt::Key_NumberSign    0x23     
        # Qt::Key_Dollar    0x24     
        # Qt::Key_Percent    0x25     
        # Qt::Key_Ampersand    0x26     
        elif button == Qt.Key_Apostrophe:
            return PyCEGUI.Key.Apostrophe     
        # Qt::Key_ParenLeft    0x28     
        # Qt::Key_ParenRight    0x29
        # Qt::Key_Asterisk    0x2a     
        # Qt::Key_Plus    0x2b
        elif button == Qt.Key_Comma:
            return PyCEGUI.Key.Comma
        elif button == Qt.Key_Minus:
            return PyCEGUI.Key.Minus
        elif button == Qt.Key_Period:
            return PyCEGUI.Key.Period
        elif button == Qt.Key_Slash:
            return PyCEGUI.Key.Slash
        elif button == Qt.Key_0:
            return PyCEGUI.Key.Zero
        elif button == Qt.Key_1:
            return PyCEGUI.Key.One
        elif button == Qt.Key_2:
            return PyCEGUI.Key.Two
        elif button == Qt.Key_3:
            return PyCEGUI.Key.Three
        elif button == Qt.Key_4:
            return PyCEGUI.Key.Four
        elif button == Qt.Key_5:
            return PyCEGUI.Key.Five
        elif button == Qt.Key_6:
            return PyCEGUI.Key.Six
        elif button == Qt.Key_7:
            return PyCEGUI.Key.Seven
        elif button == Qt.Key_8:
            return PyCEGUI.Key.Eight
        elif button == Qt.Key_9:
            return PyCEGUI.Key.Nine
        elif button == Qt.Key_Colon:
            return PyCEGUI.Key.Colon
        elif button == Qt.Key_Semicolon:
            return PyCEGUI.Key.Semicolon
        # missing Key_Less
        elif button == Qt.Key_Equal:
            return PyCEGUI.Key.Equals
        # missing Key_Greater
        # missing Key_Question
        elif button == Qt.Key_At:
            return PyCEGUI.Key.At
        elif button == Qt.Key_A:
            return PyCEGUI.Key.A
        elif button == Qt.Key_B:
            return PyCEGUI.Key.B
        elif button == Qt.Key_C:
            return PyCEGUI.Key.C
        elif button == Qt.Key_D:
            return PyCEGUI.Key.D
        elif button == Qt.Key_E:
            return PyCEGUI.Key.E
        elif button == Qt.Key_F:
            return PyCEGUI.Key.F
        elif button == Qt.Key_G:
            return PyCEGUI.Key.G
        elif button == Qt.Key_H:
            return PyCEGUI.Key.H
        elif button == Qt.Key_I:
            return PyCEGUI.Key.I
        elif button == Qt.Key_J:
            return PyCEGUI.Key.J
        elif button == Qt.Key_K:
            return PyCEGUI.Key.K
        elif button == Qt.Key_L:
            return PyCEGUI.Key.L
        elif button == Qt.Key_M:
            return PyCEGUI.Key.M
        elif button == Qt.Key_N:
            return PyCEGUI.Key.N
        elif button == Qt.Key_O:
            return PyCEGUI.Key.O
        elif button == Qt.Key_P:
            return PyCEGUI.Key.P
        elif button == Qt.Key_Q:
            return PyCEGUI.Key.Q
        elif button == Qt.Key_R:
            return PyCEGUI.Key.R
        elif button == Qt.Key_S:
            return PyCEGUI.Key.S
        elif button == Qt.Key_T:
            return PyCEGUI.Key.T
        elif button == Qt.Key_U:
            return PyCEGUI.Key.U
        elif button == Qt.Key_V:
            return PyCEGUI.Key.V
        elif button == Qt.Key_W:
            return PyCEGUI.Key.W
        elif button == Qt.Key_X:
            return PyCEGUI.Key.X
        elif button == Qt.Key_Y:
            return PyCEGUI.Key.Y
        elif button == Qt.Key_Z:
            return PyCEGUI.Key.Z
        
        # The rest are weird keys I refuse to type here
        
    def keyPressEvent(self, event):
        handled = False
        
        if self.injectInput:
            button = self.translateQtKeyboardButton(event.key())
            
            if button != None:
                handled = PyCEGUI.System.getSingleton().injectKeyDown(button)
                
            char = event.text()
            if len(char) > 0:
                handled = handled or PyCEGUI.System.getSingleton().injectChar(ord(char[0]))
                
        if not handled:
            super(CEGUIWidget, self).keyPressEvent(event)
    
    def keyReleaseEvent(self, event):
        handled = False
        
        if self.injectInput:
            button = self.translateQtKeyboardButton(event.key())
            
            if button != None:
                handled = PyCEGUI.System.getSingleton().injectKeyUp(button)
                
        if not handled:
            super(CEGUIWidget, self).keyPressEvent(event)

# we import here to avoid circular dependencies (CEGUIWidget has to be defined at this point)
import ui.ceguicontainerwidget

class CEGUIContainerWidget(QWidget):
    injectInput = property(lambda self: self.ceguiWidget.injectInput,
                           lambda self, value: setattr(self.ceguiWidget, "injectInput", value))
    
    def __init__(self, mainWindow):
        super(CEGUIContainerWidget, self).__init__()
        
        self.ui = ui.ceguicontainerwidget.Ui_CEGUIContainerWidget()
        self.ui.setupUi(self)
        
        self.currentParentWidget = None
        
        self.debugInfo = CEGUIDebugInfo(self)
        self.ceguiWidget = self.findChild(CEGUIWidget, "ceguiWidget")
        
        self.scrollArea = self.findChild(QScrollArea, "scrollArea")
        
        self.autoExpand = self.findChild(QCheckBox, "autoExpand")
        self.autoExpand.stateChanged.connect(self.slot_autoExpandChanged)
        self.resolutionBox = self.findChild(QComboBox, "resolutionBox")
        self.resolutionBox.editTextChanged.connect(self.slot_resolutionBoxChanged)
        
    def syncToProject(self, project):
        self.ceguiWidget.syncToProject(project)
    
    def makeGLContextCurrent(self):
        self.ceguiWidget.makeCurrent()
        
    def activate(self, parentWidget, resourceIdentifier):
        """Activates the CEGUI Widget for the given parentWidget (QWidget derived class).
        resourceIdentifier is usually absolute path of the file and is used to differentiate
        resolution settings
        """
        
        assert(self.currentParentWidget == None)
        self.currentParentWidget = parentWidget
        
        self.currentParentWidget.setUpdatesEnabled(False)
        if self.currentParentWidget.layout():
            self.currentParentWidget.layout().addWidget(self)
        else:
            self.setParent(self.currentParentWidget)
        self.currentParentWidget.setUpdatesEnabled(True)
        
    def deactivate(self):
        if self.currentParentWidget == None:
            return
        
        self.currentParentWidget.setUpdatesEnabled(False)
        if self.currentParentWidget.layout():
            self.currentParentWidget.layout().removeWidget(self)
        else:
            self.setParentWidget(None)
        self.currentParentWidget.setUpdatesEnabled(True)
            
        self.currentParentWidget = None
        
    def slot_autoExpandChanged(self, expand):
        if expand == Qt.Checked:
            self.scrollArea.setWidgetResizable(True)
            self.ceguiWidget.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        else:
            self.scrollArea.setWidgetResizable(False)
            self.ceguiWidget.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
            # set the currently preferred size
            self.slot_resolutionBoxChanged(self.resolutionBox.currentText())
        
    def slot_resolutionBoxChanged(self, text):
        print text
        
        if text == "Project Default":
            # special case
            pass
        else:
            res = text.split("x", 1)
            if len(res) == 2:
                try:
                    width = int(res[0])
                    height = int(res[1])
                    
                    if width < 1:
                        width = 1
                    if height < 1:
                        height = 1
                    
                    self.ceguiWidget.setGeometry(0, 0, width, height)
                    
                except:
                    # ignore invalid literals
                    pass