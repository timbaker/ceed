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

#include "cegui/cegui_init.h"

#include "compatibility/font/compat_font_init.h"
#include "compatibility/imageset/compat_imageset_init.h"
#include "compatibility/looknfeel//compat_looknfeel_init.h"
#include "compatibility/scheme/compat_scheme_init.h"

#include "editors/looknfeel/tabbed_editor.h"

#include "mainwindow.h"
#include "project.h"

#include "ceed_paths.h"

#include "CEGUI/CommonDialogs/Module.h"

#define USE_OPENGL3 0
#if USE_OPENGL3
#include "CEGUI/RendererModules/OpenGL/GL3Renderer.h"
#else
#include "CEGUI/RendererModules/OpenGL/GLRenderer.h"
#endif
#include "CEGUI/RendererModules/OpenGL/ViewportTarget.h"

#include <QGLFramebufferObject>
#include <QMessageBox>
#include <QProgressDialog>

namespace CEED {
namespace cegui {

Instance::Instance(GLContextProvider *contextProvider)
{
    m_contextProvider = contextProvider;

    m_logger = new RedirectingCEGUILogger();

    m_initialised = false;
    m_lastRenderTimeDelta = 0.0f;
}

void Instance::makeGLContextCurrent()
{
    m_contextProvider->makeGLContextCurrent();
}

void Instance::ensureIsInitialised()
{
    if (!m_initialised) {
        makeGLContextCurrent();

        // we don't want CEGUI Exceptions to output to stderr every time
        // they are constructed
        CEGUI::Exception::setStdErrEnabled(false);
        // FBOs are for sure supported at this point because CEED uses them internally
#if USE_OPENGL3
        CEGUI::OpenGL3Renderer::bootstrapSystem();
#else
        CEGUI::OpenGLRenderer::bootstrapSystem(CEGUI::OpenGLRenderer::TextureTargetType::TTT_FBO);
#endif
        m_initialised = true;

        setDefaultResourceGroups();

        // Added in C++ version
        initialiseCEGUICommonDialogs();
    }
}

void Instance::setResourceGroupDirectory(const QString &resourceGroup, const QString &absoluteDirPath)
{
    ensureIsInitialised();

    auto rp = CEGUI::System::getSingleton().getResourceProvider();
    auto defaultRP = dynamic_cast<CEGUI::DefaultResourceProvider*>(rp);
    defaultRP->setResourceGroupDirectory(FROM_QSTR(resourceGroup), FROM_QSTR(absoluteDirPath));
}

void Instance::setDefaultResourceGroups()
{
    ensureIsInitialised();

    // reasonable default directories
    QString defaultBaseDirectory = os.path.join(os.path.curdir(), "datafiles");

    setResourceGroupDirectory("imagesets",
                              os.path.join(defaultBaseDirectory, "imagesets"));
    setResourceGroupDirectory("fonts",
                              os.path.join(defaultBaseDirectory, "fonts"));
    setResourceGroupDirectory("schemes",
                              os.path.join(defaultBaseDirectory, "schemes"));
    setResourceGroupDirectory("looknfeels",
                              os.path.join(defaultBaseDirectory, "looknfeel"));
    setResourceGroupDirectory("layouts",
                              os.path.join(defaultBaseDirectory, "layouts"));
    setResourceGroupDirectory("xml_schemas",
                              os.path.join(defaultBaseDirectory, "xml_schemas"));

    // all this will never be set to anything else again
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");

    auto parser = CEGUI::System::getSingleton().getXMLParser();
    if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
        parser->setProperty("SchemaDefaultResourceGroup", "xml_schemas");
}

void Instance::cleanCEGUIResources()
{
    // destroy all previous resources (if any)
    if (m_initialised) {
        CEGUI::WindowManager::getSingleton().destroyAllWindows();
        // we need to ensure all windows are destroyed, dangling pointers would
        // make us segfault later otherwise
        CEGUI::WindowManager::getSingleton().cleanDeadPool();
        CEGUI::FontManager::getSingleton().destroyAll();
        CEGUI::ImageManager::getSingleton().destroyAll();
        CEGUI::SchemeManager::getSingleton().destroyAll();
        CEGUI::WidgetLookManager::getSingleton().eraseAllWidgetLooks();
        CEGUI::AnimationManager::getSingleton().destroyAllAnimations();
        CEGUI::WindowFactoryManager::getSingleton().removeAllFalagardWindowMappings();
        CEGUI::WindowFactoryManager::getSingleton().removeAllWindowTypeAliases();
        CEGUI::WindowFactoryManager::getSingleton().removeAllFactories();
        // the previous call removes all Window factories, including the stock ones like DefaultWindow
        // lets add them back
        CEGUI::System::getSingleton().addStandardWindowFactories();
        // Added in C++ version
        initialiseCEGUICommonDialogs();
        CEGUI::System::getSingleton().getRenderer()->destroyAllTextures();
    }
}

void Instance::syncToProject(project::Project* project, mainwindow::MainWindow* mainWindow)
{
    QProgressDialog progress(mainWindow);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Synchronising embedded CEGUI with the project");
    progress.setCancelButton(nullptr);
    progress.resize(400, 100);
    progress.show();

    ensureIsInitialised();
    makeGLContextCurrent();

    QStringList schemeFiles;
    QString absoluteSchemesPath = project->getAbsolutePathOf(project->m_schemesPath);
    if (os.path.exists(absoluteSchemesPath)) {
        for (QString file_ : os.listdir(absoluteSchemesPath)) {
            if (file_.endsWith(".scheme")) {
                schemeFiles.append(file_);
            }
        }
    } else {
        progress.reset();
        throw IOError(QString(QString("Can't list scheme path '%1'").arg(absoluteSchemesPath)));
    }

    // VanillaCommonDialogs.scheme must come after VanillaSkin.scheme
    {
        int index = schemeFiles.indexOf(QRegExp(".*VanillaCommonDialogs\\.scheme"));
        if (index != -1)
            schemeFiles.move(index, schemeFiles.size() - 1);
    }

    progress.setMinimum(0);
    progress.setMaximum(2 + 9 * schemeFiles.length());

    progress.setLabelText("Purging all resources...");
    progress.setValue(0);
    QApplication::instance()->processEvents();

    // destroy all previous resources (if any)
    cleanCEGUIResources();

    progress.setLabelText("Setting resource paths...");
    progress.setValue(1);
    QApplication::instance()->processEvents();

    setResourceGroupDirectory("imagesets", project->getAbsolutePathOf(project->m_imagesetsPath));
    setResourceGroupDirectory("fonts", project->getAbsolutePathOf(project->m_fontsPath));
    setResourceGroupDirectory("schemes", project->getAbsolutePathOf(project->m_schemesPath));
    setResourceGroupDirectory("looknfeels", project->getAbsolutePathOf(project->m_looknfeelsPath));
    setResourceGroupDirectory("layouts", project->getAbsolutePathOf(project->m_layoutsPath));
    setResourceGroupDirectory("xml_schemas", project->getAbsolutePathOf(project->m_xmlSchemasPath));

    progress.setLabelText("Recreating all schemes...");
    progress.setValue(2);
    QApplication::instance()->processEvents();

    // we will load resources manually to be able to use the compatibility layer machinery
    CEGUI::SchemeManager::getSingleton().setAutoLoadResources(false);

    struct finally_t
    {
        QProgressDialog& progress;

        finally_t(QProgressDialog& progress)
            : progress(progress)
        {

        }

        ~finally_t()
        {
            // put SchemeManager into the default state again
            CEGUI::SchemeManager::getSingleton().setAutoLoadResources(true);

            progress.reset();
            QApplication::instance()->processEvents();
        }
    };
    finally_t finally(progress);

    try {
        for (QString schemeFile : schemeFiles) {
            auto updateProgress = [&](const QString& message)
            {
                progress.setValue(progress.value() + 1);
                progress.setLabelText(QString("Recreating all schemes... (%1)\n\n%2").arg(schemeFile).arg(message));

                QApplication::instance()->processEvents();
            };

            updateProgress("Parsing the scheme file");
            QString schemeFilePath = project->getResourceFilePath(schemeFile, TO_QSTR(CEGUI::Scheme::getDefaultResourceGroup()));
            QFile file(schemeFilePath);
            if (!file.open(QFile::ReadOnly | QFile::Text))
                throw IOError(QString("QFile::open failed ") + schemeFilePath);
            QString rawData = QString::fromUtf8(file.readAll());
            file.close();
            QString rawDataType = compatibility::scheme::manager->EditorNativeType;

            try {
                rawDataType = compatibility::scheme::manager->guessType(rawData, schemeFilePath);

            } catch (compatibility::NoPossibleTypesError e) {
                QMessageBox::warning(nullptr, "Scheme doesn't match any known data type",
                                     QString("The scheme '%1' wasn't recognised by CEED as any scheme data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!").arg(schemeFilePath));
                return;

            } catch (compatibility::MultiplePossibleTypesError e) {
                QString suitableVersion = compatibility::scheme::manager->getSuitableDataTypeForCEGUIVersion(project->m_CEGUIVersion);

                if (!e.m_possibleTypes.contains(suitableVersion)) {
                    QMessageBox::warning(nullptr, "Incorrect scheme data type",
                                         QString("The scheme '%1' checked out as some potential data types, however not any of these is suitable for your project's target CEGUI version '%2', please check your project settings! CEGUI instance synchronisation aborted!").arg(schemeFilePath).arg(suitableVersion));
                    return;
                }
                rawDataType = suitableVersion;
            }

            QString nativeData = compatibility::scheme::manager->transform(rawDataType, compatibility::scheme::manager->EditorNativeType, rawData);
            CEGUI::Scheme& scheme = CEGUI::SchemeManager::getSingleton().createFromString(FROM_QSTR(nativeData));

            // NOTE: This is very CEGUI implementation specific unfortunately!
            //
            //       However I am not really sure how to do this any better.

            updateProgress("Loading XML imagesets");
            auto xmlImagesetIterator = scheme.getXMLImagesets();
            while (!xmlImagesetIterator.isAtEnd()) {
                auto loadableUIElement = xmlImagesetIterator.getCurrentValue();
                QString imagesetFilePath = project->getResourceFilePath(
                            TO_QSTR(loadableUIElement.filename),
                            TO_QSTR((loadableUIElement.resourceGroup != "") ?
                                loadableUIElement.resourceGroup :
                                CEGUI::ImageManager::getImagesetDefaultResourceGroup()));
                QFile file(imagesetFilePath);
                if (!file.open(QFile::ReadOnly | QFile::Text))
                    throw IOError(QString("QFile::open failed for %1").arg(imagesetFilePath));
                QByteArray data = file.readAll();
                file.close();
                QString imagesetRawData = QString::fromUtf8(data);
                QString imagesetRawDataType = compatibility::imageset::manager->EditorNativeType;

                try {
                    imagesetRawDataType = compatibility::imageset::manager->guessType(imagesetRawData, imagesetFilePath);

                } catch (compatibility::NoPossibleTypesError e) {
                    QMessageBox::warning(nullptr, "Imageset doesn't match any known data type",
                                         QString("The imageset '%1' wasn't recognised by CEED as any imageset data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!").arg(imagesetFilePath));
                    return;
                } catch (compatibility::MultiplePossibleTypesError e) {
                    QString suitableVersion = compatibility::imageset::manager->getSuitableDataTypeForCEGUIVersion(project->m_CEGUIVersion);

                    if (!e.m_possibleTypes.contains(suitableVersion)) {
                        QMessageBox::warning(nullptr, "Incorrect imageset data type",
                                             QString("The imageset '%1' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%2', please check your project settings! CEGUI instance synchronisation aborted!").arg(imagesetFilePath, suitableVersion));
                        return;
                    }

                    imagesetRawDataType = suitableVersion;
                }

                QString imagesetNativeData = compatibility::imageset::manager->transform(imagesetRawDataType, compatibility::imageset::manager->EditorNativeType, imagesetRawData);

                CEGUI::ImageManager::getSingleton().loadImagesetFromString(FROM_QSTR(imagesetNativeData));
                xmlImagesetIterator++;
            }

            updateProgress("Loading image file imagesets");
            scheme.loadImageFileImagesets();

            updateProgress("Loading fonts");
            auto fontIterator = scheme.getFonts();
            while (!fontIterator.isAtEnd()) {
                auto loadableUIElement = fontIterator.getCurrentValue();
                QString fontFilePath = project->getResourceFilePath(TO_QSTR(loadableUIElement.filename),
                                                                    TO_QSTR((loadableUIElement.resourceGroup != "") ?
                                                                        loadableUIElement.resourceGroup :
                                                                        CEGUI::Font::getDefaultResourceGroup()));
                QFile file(fontFilePath);
                if (!file.open(QFile::ReadOnly | QFile::Text))
                    throw IOError(QString("QFile::open failed for %1").arg(fontFilePath));
                QByteArray data = file.readAll();
                file.close();
                QString fontRawData = QString::fromUtf8(data);
                QString fontRawDataType = compatibility::font::manager->EditorNativeType;

                try {
                    fontRawDataType = compatibility::font::manager->guessType(fontRawData, fontFilePath);

                } catch (compatibility::NoPossibleTypesError e) {
                    QMessageBox::warning(nullptr, "Font doesn't match any known data type",
                                         QString("The font '%1' wasn't recognised by CEED as any font data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!").arg(fontFilePath));
                    return;
                }

                catch (compatibility::MultiplePossibleTypesError e) {
                    QString suitableVersion = compatibility::font::manager->getSuitableDataTypeForCEGUIVersion(project->m_CEGUIVersion);

                    if (!e.m_possibleTypes.contains(suitableVersion)) {
                        QMessageBox::warning(nullptr, "Incorrect font data type",
                                             QString("The font '%1' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%2', please check your project settings! CEGUI instance synchronisation aborted!").arg(fontFilePath).arg(suitableVersion));
                        return;
                    }

                    fontRawDataType = suitableVersion;
                }

                QString fontNativeData = compatibility::font::manager->transform(fontRawDataType, compatibility::font::manager->EditorNativeType, fontRawData);

                CEGUI::FontManager::getSingleton().createFromString(FROM_QSTR(fontNativeData));
                fontIterator++;
            }

            updateProgress("Loading looknfeels");
            auto looknfeelIterator = scheme.getLookNFeels();
            while (!looknfeelIterator.isAtEnd()) {
                auto loadableUIElement = looknfeelIterator.getCurrentValue();
                QString looknfeelFilePath = project->getResourceFilePath(
                            TO_QSTR(loadableUIElement.filename),
                            TO_QSTR((loadableUIElement.resourceGroup != "") ?
                                loadableUIElement.resourceGroup :
                                CEGUI::WidgetLookManager::getDefaultResourceGroup()));
                QFile file(looknfeelFilePath);
                if (!file.open(QFile::ReadOnly | QFile::Text))
                    throw IOError(QString("QFile::open failed for %1").arg(looknfeelFilePath));
                QByteArray data = file.readAll();
                file.close();
                QString looknfeelRawData = QString::fromUtf8(data);
                QString looknfeelRawDataType = compatibility::looknfeel::manager->EditorNativeType;
                try {
                    looknfeelRawDataType = compatibility::looknfeel::manager->guessType(looknfeelRawData, looknfeelFilePath);

                } catch (compatibility::NoPossibleTypesError e) {
                    QMessageBox::warning(nullptr, "LookNFeel doesn't match any known data type",
                                         QString("The looknfeel '%1' wasn't recognised by CEED as any looknfeel data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!").arg(looknfeelFilePath));
                    return;

                } catch (compatibility::MultiplePossibleTypesError e) {
                    QString suitableVersion = compatibility::looknfeel::manager->getSuitableDataTypeForCEGUIVersion(project->m_CEGUIVersion);

                    if (!e.m_possibleTypes.contains(suitableVersion)) {
                        QMessageBox::warning(nullptr, "Incorrect looknfeel data type",
                                             QString("The looknfeel '%1' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%2', please check your project settings! CEGUI instance synchronisation aborted!").arg(looknfeelFilePath).arg(suitableVersion));
                        return;
                    }

                    looknfeelRawDataType = suitableVersion;
                }

                QString looknfeelNativeData = compatibility::looknfeel::manager->transform(looknfeelRawDataType, compatibility::looknfeel::manager->EditorNativeType, looknfeelRawData);

                CEGUI::WidgetLookManager::getSingleton().parseLookNFeelSpecificationFromString(FROM_QSTR(looknfeelNativeData));
                looknfeelIterator++;
            }

            updateProgress("Loading window renderer factory modules");
            scheme.loadWindowRendererFactories();
            updateProgress("Loading window factories");
            scheme.loadWindowFactories();
            updateProgress("Loading factory aliases");
            scheme.loadFactoryAliases();
            updateProgress("Loading falagard mappings");
            scheme.loadFalagardMappings();
        }
    }
    catch (...) {
        cleanCEGUIResources();
        throw;
    }
}

QStringList Instance::getAvailableSkins()
{
    QStringList skins;

    auto it = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
    while (!it.isAtEnd()) {
        QString currentSkin = TO_QSTR(it.getCurrentValue().d_windowType).split('/')[0];

        QString ceedInternalEditingPrefix = editors::looknfeel::tabbed_editor::LookNFeelTabbedEditor::getEditorIDStringPrefix();
        bool ceedInternalLNF = false;
        if (currentSkin.startsWith(ceedInternalEditingPrefix))
            ceedInternalLNF = true;

        if (!skins.contains(currentSkin) && !ceedInternalLNF)
            skins.append(currentSkin);

        it++;
    }

    std::sort(skins.begin(), skins.end());
    return skins;
}

QStringList Instance::getAvailableFonts()
{
    QStringList fonts;

    auto it = CEGUI::FontManager::getSingleton().getIterator();
    while (!it.isAtEnd()) {
        fonts.append(TO_QSTR(it.getCurrentKey()));
        it++;
    }

    std::sort(fonts.begin(), fonts.end());
    return fonts;
}

QStringList Instance::getAvailableImages()
{
    QStringList images;

    auto it = CEGUI::ImageManager::getSingleton().getIterator();
    while (!it.isAtEnd()) {
        images.append(TO_QSTR(it.getCurrentKey()));
        it++;
    }

    std::sort(images.begin(), images.end());
    return images;
}

OrderedMap<QString, QStringList> Instance::getAvailableWidgetsBySkin()
{
    OrderedMap<QString, QStringList> ret;
    QStringList strings = { "DefaultWindow", "DragContainer",
            "VerticalLayoutContainer", "HorizontalLayoutContainer",
            "GridLayoutContainer" };
    ret["__no_skin__"] = strings;

    auto it = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
    while (!it.isAtEnd()) {
        //base = it.getCurrentValue().d_baseType
        QString mappedType0 = TO_QSTR(it.getCurrentValue().d_windowType).section('/', 0, 0);
        QString mappedType1 = TO_QSTR(it.getCurrentValue().d_windowType).section('/', 1);
        Q_ASSERT(!mappedType0.isEmpty() && !mappedType1.isEmpty());

        QString look = mappedType0;
        QString widget = mappedType1;

        QString ceedInternalEditingPrefix = editors::looknfeel::tabbed_editor::LookNFeelTabbedEditor::getEditorIDStringPrefix();
        bool ceedInternalLNF = false;
        if (look.startsWith(ceedInternalEditingPrefix))
            ceedInternalLNF = true;

        if (!ceedInternalLNF) {
            // insert empty list for the look if it's a new look
            if (!ret.contains(look))
                ret[look] = QStringList();

            // append widget name to the list for its look
            ret[look].append(widget);
        }

        it++;
    }

    // sort the lists
    for (QString look : ret.keys()) {
        ret[look].sort();
    }

    return ret;
}

QImage Instance::getWidgetPreviewImage(const QString &widgetType, int previewWidth, int previewHeight)
{
    ensureIsInitialised();
    makeGLContextCurrent();

    auto& system = CEGUI::System::getSingleton();

    auto* renderer = system.getRenderer();

    CEGUI::OpenGLViewportTarget renderTarget(*dynamic_cast<CEGUI::OpenGLRendererBase*>(renderer));
    renderTarget.setArea(CEGUI::Rectf(0, 0, previewWidth, previewHeight));
    CEGUI::RenderingSurface renderingSurface(renderTarget);

    CEGUI::Window* widgetInstance = CEGUI::WindowManager::getSingleton().createWindow(FROM_QSTR(widgetType), "preview");
    widgetInstance->setRenderingSurface(&renderingSurface);
    // set it's size and position so that it shows up
    widgetInstance->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0)));
    widgetInstance->setSize(CEGUI::USize(CEGUI::UDim(0, previewWidth), CEGUI::UDim(0, previewHeight)));
    // fake update to ensure everything is set
    widgetInstance->update(1);

    QGLFramebufferObject temporaryFBO(previewWidth, previewHeight, GL_TEXTURE_2D);
    temporaryFBO.bind();

    renderingSurface.invalidate();

    renderer->beginRendering();

    try {
        widgetInstance->render();
    } catch (...) {
    }

    // no matter what happens we have to clean after ourselves!
    renderer->endRendering();
    temporaryFBO.release();
    CEGUI::WindowManager::getSingleton().destroyWindow(widgetInstance);

    return temporaryFBO.toImage();
}

} // namespace cegui
} // namespace CEED
