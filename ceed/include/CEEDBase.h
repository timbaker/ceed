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

#ifndef CEED_BASE_H_
#define CEED_BASE_H_

#include "CEEDConfig.h"

#include "optional.hpp"
using nonstd::optional;

//#include "variant.hpp"

#include "qtorderedmap.h"

#include <QSizeF>

#if 1
#include "CEGUI/CEGUI.h"
#include <glm/gtc/quaternion.hpp>

namespace CEGUI
{
typedef std::vector<String> StringList;
}

#else
// temp until CEGUI imported
namespace CEGUI {
    class Logger
    {
    };

    class Widget
    {
    public:
        Widget* getParent() {}
    };

    class System
    {
    public:
        static System* getSingleton() { return nullptr; }
    };

    struct Sizef
    {
        qreal d_width;
        qreal d_height;
    };

    enum LoggingLevel { Errors, Warnings };
    enum Key {};
    enum MouseButton { LeftButton, RightButton };
}
#endif

#include <QString>
#include <stdexcept>

class RuntimeError : public std::exception
{
public:
    RuntimeError(const QString& s)
        : std::exception(s.toStdString().c_str())
    {

    }

    QString operator()() const { return QLatin1String(this->what()); }
};

typedef RuntimeError Exception;
typedef RuntimeError IOError;
typedef RuntimeError OSError;
typedef RuntimeError LookupError;
typedef RuntimeError ValueError;
typedef RuntimeError NotImplementedError;

#include <QDebug>

struct logging_t
{
    void debug(const QString& s) { qDebug() << s; }
    void error(const QString& s) { qDebug() << s; }
};

extern logging_t logging;

//#define isinstance(var,type) (dynamic_cast<type>(var) != nullptr)

#define TO_QSTR(s) QString::fromUtf8((const char*)(s).data())
#define FROM_QSTR(s) (s).toStdString()

namespace CEED
{

enum class VariantType
{
    NONE, // no value
    Int, // int
    UInt, // unsigned int
    Float, // float
    Bool, // bool
    QString, // QString
    OrderedMap, // OrderedMap<> for DictionaryProperty
    // these are the wrapper classes declared in cegui/ceguitypes.h
    CEED_UDim,
    CEED_USize,
    CEED_UVector2,
    CEED_URect,
    CEED_UBox,
    CEED_AspectMode,
    CEED_HorizontalAlignment,
    CEED_VerticalAlignment,
    CEED_WindowUpdateMode,
    CEED_Quaternion,
    CEED_XYZRotation,
    CEED_HorizontalFormatting,
    CEED_VerticalFormatting,
    CEED_HorizontalTextFormatting,
    CEED_VerticalTextFormatting,
    CEED_SortMode,
    CEED_Colour,
    CEED_ColourRect,
    CEED_FontRef,
    CEED_ImageRef,
    CEED_Property, // propertytree::properties::Property for DictionaryProperty
    CEED_EnumValue,// propertytree::properties::EnumValue
    // CEGUI types
    CEGUI_String,
    CEGUI_UDim,
    CEGUI_USize,
    CEGUI_UVector2,
    CEGUI_URect,
    CEGUI_UBox,
    CEGUI_AspectMode,
    CEGUI_HorizontalAlignment,
    CEGUI_VerticalAlignment,
    CEGUI_WindowUpdateMode,
    CEGUI_Quaternion,
    CEGUI_HorizontalFormatting,
    CEGUI_VerticalFormatting,
    CEGUI_HorizontalTextFormatting,
    CEGUI_VerticalTextFormatting,
    CEGUI_SortMode,
    CEGUI_Colour,
    CEGUI_ColourRect,
    CEGUI_Font,
    CEGUI_Image,
    CEGUI_BasicImage,
};

/* A 'variant' */
class FalagardElement
{
public:
    enum class Type
    {
        NONE,
        WidgetLookFeel,
        PropertyDefinitionBase,
        PropertyInitialiser,
        NamedArea,
        ImagerySection,
        StateImagery,
        WidgetComponent,
        ImageryComponent,
        TextComponent,
        FrameComponent,
        LayerSpecification,
        SectionSpecification,
        ComponentArea,
        Image,
        ColourRect,
    };

protected:
    Type m_type;

    // Types that can't go in a union
    CEGUI::ColourRect colourRect;

    union
    {
        CEGUI::WidgetLookFeel* widgetLookFeel;
        CEGUI::PropertyDefinitionBase* propertyDefinitionBase;
        CEGUI::PropertyInitialiser* propertyInitialiser;
        CEGUI::NamedArea* namedArea;
        CEGUI::ImagerySection* imagerySection;
        CEGUI::StateImagery* stateImagery;
        CEGUI::WidgetComponent* widgetComponent;
        CEGUI::ImageryComponent* imageryComponent;
        CEGUI::TextComponent* textComponent;
        CEGUI::FrameComponent* frameComponent;
        CEGUI::LayerSpecification* layerSpecification;
        CEGUI::SectionSpecification* sectionSpecification;
        const CEGUI::ComponentArea* componentArea;
        CEGUI::Image* image;
    };

public:
    FalagardElement()
        : m_type(Type::NONE)
    {

    }

    FalagardElement(const FalagardElement& other);

    FalagardElement& operator=(const FalagardElement& other);

    bool operator==(const FalagardElement& other) const;

    FalagardElement(CEGUI::WidgetLookFeel* instance) : m_type(Type::WidgetLookFeel), widgetLookFeel(instance) {}
    FalagardElement(CEGUI::PropertyDefinitionBase* instance) : m_type(Type::PropertyDefinitionBase), propertyDefinitionBase(instance) {}
    FalagardElement(CEGUI::PropertyInitialiser* instance) : m_type(Type::PropertyInitialiser), propertyInitialiser(instance) {}
    FalagardElement(CEGUI::NamedArea* instance) : m_type(Type::NamedArea), namedArea(instance) {}
    FalagardElement(CEGUI::ImagerySection* instance) : m_type(Type::ImagerySection), imagerySection(instance) {}
    FalagardElement(CEGUI::StateImagery* instance) : m_type(Type::StateImagery), stateImagery(instance) {}
    FalagardElement(CEGUI::WidgetComponent* instance) : m_type(Type::WidgetComponent), widgetComponent(instance) {}
    FalagardElement(CEGUI::ImageryComponent* instance) : m_type(Type::ImageryComponent), imageryComponent(instance) {}
    FalagardElement(CEGUI::TextComponent* instance) : m_type(Type::TextComponent), textComponent(instance) {}
    FalagardElement(CEGUI::FrameComponent* instance) : m_type(Type::FrameComponent), frameComponent(instance) {}
    FalagardElement(CEGUI::LayerSpecification* instance) : m_type(Type::LayerSpecification), layerSpecification(instance) {}
    FalagardElement(CEGUI::SectionSpecification* instance) : m_type(Type::SectionSpecification), sectionSpecification(instance) {}
    FalagardElement(const CEGUI::ComponentArea* instance) : m_type(Type::ComponentArea), componentArea(instance) {}
    FalagardElement(CEGUI::Image* instance) : m_type(Type::Image), image(instance) {}
    FalagardElement(const CEGUI::ColourRect& v) : m_type(Type::ColourRect), colourRect(v) {}

    Type type() { return this->m_type; }
    bool isType(Type type) { return this->m_type == type; }

    CEGUI::WidgetLookFeel* toWidgetLookFeel() { return isType(Type::WidgetLookFeel) ? widgetLookFeel : nullptr; }
    CEGUI::PropertyDefinitionBase* toPropertyDefinitionBase() { return isType(Type::PropertyDefinitionBase) ? propertyDefinitionBase : nullptr; }
    CEGUI::PropertyInitialiser* toPropertyInitialiser() { return isType(Type::PropertyInitialiser) ? propertyInitialiser : nullptr; }
    CEGUI::NamedArea* toNamedArea() { return isType(Type::NamedArea) ? namedArea : nullptr; }
    CEGUI::ImagerySection* toImagerySection() { return isType(Type::ImagerySection) ? imagerySection : nullptr; }
    CEGUI::StateImagery* toStateImagery() { return isType(Type::StateImagery) ? stateImagery : nullptr; }
    CEGUI::WidgetComponent* toWidgetComponent() { return isType(Type::WidgetComponent) ? widgetComponent : nullptr; }
    CEGUI::ImageryComponent* toImageryComponent() { return isType(Type::ImageryComponent) ? imageryComponent : nullptr; }
    CEGUI::TextComponent* toTextComponent() { return isType(Type::TextComponent) ? textComponent : nullptr; }
    CEGUI::FrameComponent* toFrameComponent() { return isType(Type::FrameComponent) ? frameComponent : nullptr; }
    CEGUI::LayerSpecification* toLayerSpecification() { return isType(Type::LayerSpecification) ? layerSpecification : nullptr; }
    CEGUI::SectionSpecification* toSectionSpecification() { return isType(Type::SectionSpecification) ? sectionSpecification : nullptr; }
    const CEGUI::ComponentArea* toComponentArea() { return isType(Type::ComponentArea) ? componentArea : nullptr; }
    CEGUI::Image* toImage() { return isType(Type::Image) ? image : nullptr; }
    CEGUI::ColourRect toColourRect() { return isType(Type::ColourRect) ? colourRect : CEGUI::ColourRect(); }
};

} // namespace CEED

#if 1

namespace ElementTree {
class Element;
}

namespace CEED {

    namespace action {
        class ActionManager;
        class Connection;
        class ConnectionGroup;
        namespace declaration {
            class Action;
        }
    }

    namespace application {
        class Application;
    }

    namespace cegui {
        class Instance;
        namespace container {
            class ContainerWidget;
            class ViewState;
        }
        namespace qtgraphics {
            class GraphicsScene;
            class GraphicsView;
        }
        namespace widgethelpers {
            class GraphicsScene;
        }
    }

    namespace commands {
        class UndoCommand;
        class UndoViewer;
    }

    namespace compatibility {
        class Manager;
    }

    namespace editors {
        class TabbedEditor;
        class TabbedEditorFactory;
        class UndoStackTabbedEditor;
        namespace animation_list {
            class AnimationListTabbedEditor;
            namespace timeline {
                class AffectorTimelineKeyFrame;
                class AffectorTimelineSection;
                class AnimationTimeline;
            }
            namespace visual {
                class VisualEditing;
            }
        }
        namespace imageset {
            class ImagesetTabbedEditor;
            namespace elements {
                class ImageEntry;
                class ImagesetEntry;
            }
            namespace visual {
                class FakeVisual;
                class ImagesetEditorDockWidget;
                class ImagesetEditorItem;
                class VisualEditing;
            }
        }
        namespace layout {
            class LayoutTabbedEditor;
            namespace widgethelpers {
                class Manipulator;
                class SerialisationData;
            }
            namespace visual {
                class FakeVisual;
                class HierarchyDockWidget;
                class VisualEditing;
                class WidgetHierarchyItem;
            }
        }
        namespace looknfeel {
            namespace code {
                class CodeEditing;
            }
            namespace falagard_element_editor {
                class LookNFeelFalagardElementEditorDockWidget;
            }
            namespace falagard_element_inspector {
                class FalagardElementAttributesManager;
                class FalagardElementSettingCategory;
                class PropertyInspectorWidget;
            }
            namespace hierarchy_dock_widget {
                class LookNFeelHierarchyDockWidget;
            }
            namespace hierarchy_tree_model {
                class LookNFeelHierarchyTreeModel;
            }
            namespace preview {
                class LookNFeelPreviewer;
            }
            namespace tabbed_editor {
                class LookNFeelTabbedEditor;
            }
            namespace visual {
                class LookNFeelVisualEditing;
            }
        }
        namespace metaimageset {
            namespace inputs {
                class Image;
                class Input;
            }
            class MetaImageset;
        }
    }

    namespace error {
        class ErrorHandler;
    }

    namespace filesystembrowser {
        class FileSystemBrowser;
    }

    namespace mainwindow {
        class MainWindow;
    }

    namespace project {
        class Project;
        class ProjectManager;
    }

    namespace propertymapping {
        class PropertyMap;
    }

    namespace propertysetinspector {
        class PropertyFactoryBase;
        class PropertyInspectorWidget;
    }

    namespace propertytree {
        namespace properties {
            class EnumValue;
            class MultiPropertyWrapper;
            class Property;
        }
        namespace ui {
            class PropertyTreeWidget;
        }
    }

    namespace qtwidgets {
        class FileLineEdit;
        class LineEditWithClearButton;
    }

    namespace recentlyused {
        class RecentlyUsedMenuEntry;
    }

    namespace settings {
        class Settings;
        namespace declaration {
            class Category;
            class Entry;
            class Settings;
        }
        namespace interface_ {
            class QtSettingsInterface;
        }
    }
}

#else

class AboutDialog;
class Action;
class ActionCategory;
class ActionManager;
class AffectorTimelineKeyFrame;
class AffectorTimelineLabel;
class AffectorTimelineSection;
class AnimationDefinitionWrapper;
class AnimationList1TypeDetector;
class AnimationListDockWidget;
class AnimationListTabbedEditor;
class AnimationListTabbedEditorFactory;
class AnimationTimeline;
class Application;
class AspectMode;
class AstHelper;
class Base;
class BaseProperty;
class Bitmap;
class BitmapTabbedEditor;
class BitmapTabbedEditorFactory;
class BoolPropertyEditor;
class BottomEdgeResizingHandle;
class BottomLeftCornerResizingHandle;
class BottomRightCornerResizingHandle;
class CEGUI1ToCEGUI2Layer;
class CEGUI1ToGorillaLayer;
class CEGUI2ToCEGUI1Layer;
class CEGUI4ToCEGUI5Layer;
class CEGUI5ToCEGUI4Layer;
class CEGUIPropertyManager;
class CEGUIWidgetPropertyManager;
class Category;
class ChangeCurrentAnimationDefinitionCommand;
class CodeEditMode;
class CodeEditModeCommand;
class CodeEditing;
class CodeEditingWithViewRestore;
class Colour;
class ColourButton;
class ColourProperty;
class ColourRect;
class ColourRectProperty;
class ColourValuePropertyEditor;
class CompilerInstance;
class Component;
class ConnectionGroup;
class ContainerWidget;
class CornerResizingHandle;
class CreateCommand;
class CreateWidgetDockWidget;
class CygonRectanglePacker;
class DebugInfo;
class DeleteCommand;
class DictionaryProperty;
class DuplicateCommand;
class DynamicChoicesEditor;
class EdgeResizingHandle;
class EditMode;
class EditingScene;
class Entry;
class EnumBase;
class EnumValue;
class EnumValuePropertyEditor;
class ErrorHandler;
class ExceptionDialog;
class FalagardElementAttributeEdit;
class FalagardElementAttributesManager;
class FalagardElementEditorProperty;
class FalagardElementInterface;
class FalagardElementSettingCategory;
class FileLineEdit;
class FileSystemBrowser;
class Font2ToFont3Layer;
class Font2TypeDetector;
class Font3ToFont2Layer;
class Font3TypeDetector;
class FontEditor;
class FontRef;
class FrameComponent;
class GLContextProvider;
class GeometryChangeCommand;
class GorillaToCEGUI1Layer;
class GorillaTypeDetector;
class GraphicsScene;
class GraphicsView;
class HTMLViewTabbedEditor;
class HierarchyDockWidget;
class HorizontalAlignCommand;
class HorizontalAlignment;
class HorizontalFormatting;
class HorizontalTextFormatting;
class Image;
class ImageEditor;
class ImageEntry;
class ImageEntryItemDelegate;
class ImageInstance;
class ImageLabel;
class ImageOffset;
class ImageRef;
class Imageset;
class Imageset1TypeDetector;
class Imageset2TypeDetector;
class ImagesetChangeAutoScaledCommand;
class ImagesetChangeImageCommand;
class ImagesetChangeNativeResolutionCommand;
class ImagesetEditorDockWidget;
class ImagesetEntry;
class ImagesetRenameCommand;
class ImagesetTabbedEditor;
class ImagesetTabbedEditorFactory;
class InkscapeSVG;
class Input;
class Instance;
class InterfaceCategory;
class InterfaceEntry;
class InterfaceEntryCheckbox;
class InterfaceEntryColour;
class InterfaceEntryCombobox;
class InterfaceEntryFloat;
class InterfaceEntryInt;
class InterfaceEntryKeySequence;
class InterfaceEntryPen;
class InterfaceEntryString;
class InterfaceSection;
class Item;
class KeyFramePropertiesDockWidget;
class KeySequenceButton;
class Layer;
class LayerNotFoundError;
class Layout2TypeDetector;
class Layout3To4Layer;
class Layout3TypeDetector;
class Layout4To3Layer;
class Layout4TypeDetector;
class LayoutPreviewer;
class LayoutTabbedEditor;
class LayoutTabbedEditorFactory;
class LeftEdgeResizingHandle;
class LicenseDialog;
class LineEditWithClearButton;
class LogMessageWrapper;
class LookNFeel6To7Layer;
class LookNFeel6TypeDetector;
class LookNFeel7To6Layer;
class LookNFeel7TypeDetector;
class LookNFeelFalagardElementEditorDockWidget;
class LookNFeelHierarchyDockWidget;
class LookNFeelHierarchyItem;
class LookNFeelHierarchyTreeModel;
class LookNFeelHierarchyTreeView;
class LookNFeelPreviewer;
class LookNFeelTabbedEditor;
class LookNFeelTabbedEditorFactory;
class LookNFeelVisualEditing;
class LookNFeelWidgetLookSelectorWidget;
class MainWindow;
class Manager;
class Manipulator;
class MessageTabbedEditor;
class MetaImageset;
class ModeSwitchCommand;
class MoveCommand;
class MoveInParentWidgetListCommand;
class MoveKeyFramesCommand;
class MultiModeTabbedEditor;
class MultiPropertyWrapper;
class MultiplePossibleFactoriesDialog;
class MultiplePossibleTypesError;
class MultipleTypesDetectedDialog;
class NewProjectDialog;
class NoPossibleTypesError;
class NoTypeDetectedDialog;
class NormalisePositionCommand;
class NormalisePositionToAbsoluteCommand;
class NormalisePositionToRelativeCommand;
class NormaliseSizeCommand;
class NormaliseSizeToAbsoluteCommand;
class NormaliseSizeToRelativeCommand;
class NumericPropertyEditor;
class OffsetMoveCommand;
class OutOfSpaceError;
class PasteCommand;
class PenButton;
class PersistenceProvider;
class Point;
class Project;
class Project1TypeDetector;
class ProjectManager;
class ProjectSettingsDialog;
class PropertiesDockWidget;
class Property;
class PropertyCategory;
class PropertyCategoryRow;
class PropertyEditCommand;
class PropertyEditor;
class PropertyEditorRegistry;
class PropertyEvent;
class PropertyEventSubscription;
class PropertyInspectorWidget;
class PropertyMap;
class PropertyMappingEntry;
class PropertyMappings1TypeDetector;
class PropertyMappingsTabbedEditor;
class PropertyMappingsTabbedEditorFactory;
class PropertyRow;
class PropertyTreeItem;
class PropertyTreeItemDelegate;
class PropertyTreeItemModel;
class PropertyTreeRow;
class PropertyTreeView;
class PropertyTreeWidget;
class QSVG;
class QSettingsPersistenceProvider;
class QtSettingsInterface;
class Quaternion;
class QuaternionProperty;
class RecentlyUsed;
class RecentlyUsedMenuEntry;
class RectanglePacker;
class RedirectingCEGUILogger;
class RenameCommand;
class ReparentCommand;
class ResizableRectItem;
class ResizeCommand;
class ResizingHandle;
class RightEdgeResizingHandle;
class RoundPositionCommand;
class RoundSizeCommand;
class Scheme4TypeDetector;
class Scheme5TypeDetector;
class Section;
class SerialisationData;
class Settings;
class SettingsInterface;
class SinglePropertyWrapper;
class SortMode;
class SplashScreen;
class StringPropertyEditor;
class StringWrapper;
class StringWrapperProperty;
class StringWrapperValidator;
class TabbedEditor;
class TabbedEditorFactory;
class TargetWidgetChangeCommand;
class TextTabbedEditor;
class TextTabbedEditorFactory;
class TimecodeLabel;
class TimelineDockWidget;
class TimelineGraphicsView;
class TimelinePositionBar;
class TopEdgeResizingHandle;
class TopLeftCornerResizingHandle;
class TopRightCornerResizingHandle;
class TypeDetector;
class UDim;
class UDimProperty;
class URect;
class URectProperty;
class USize;
class USizeProperty;
class UVector2;
class UVector2Property;
class UndoCommand;
class UndoStackTabbedEditor;
class UndoViewer;
class VerticalAlignCommand;
class VerticalAlignment;
class VerticalFormatting;
class VerticalTextFormatting;
class ViewState;
class VisualEditing;
class WidgetHierarchyItem;
class WidgetHierarchyTreeModel;
class WidgetHierarchyTreeView;
class WidgetLookHighlighter;
class WidgetMultiPropertyWrapper;
class WidgetTypeTreeWidget;
class WindowUpdateMode;
class XMLEditWidget;
class XMLSyntaxHighlighter;
class XYZRotation;
class XYZRotationProperty;

#endif

#endif
