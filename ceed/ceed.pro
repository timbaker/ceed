FORMS += \
    ui/editors/animation_list/AnimationListEditorDockWidget.ui \
    ui/editors/animation_list/AnimationListEditorKeyFramePropertiesDockWidget.ui \
    ui/editors/animation_list/AnimationListEditorTimelineDockWidget.ui \
    ui/editors/animation_list/AnimationListEditorVisualEditing.ui \
    ui/editors/imageset/ImagesetEditorDockWidget.ui \
    ui/editors/layout/LayoutEditorCreateWidgetDockWidget.ui \
    ui/editors/layout/LayoutEditorHierarchyDockWidget.ui \
    ui/editors/looknfeel/LookNFeelEditorHierarchyDockWidget.ui \
    ui/editors/looknfeel/LookNFeelEditorPropertyEditorDockWidget.ui \
    ui/editors/looknfeel/LookNFeelEditorWidgetLookSelectorWidget.ui \
    ui/editors/MultiplePossibleFactoriesDialog.ui \
    ui/editors/MultipleTypesDetectedDialog.ui \
    ui/editors/NoTypeDetectedDialog.ui \
    ui/widgets/FileLineEdit.ui \
    ui/widgets/KeySequenceButtonDialog.ui \
    ui/widgets/PenButtonDialog.ui \
    ui/AboutDialog.ui \
    ui/BitmapEditor.ui \
    ui/CEGUIContainerWidget.ui \
    ui/CEGUIWidgetInfo.ui \
    ui/ExceptionDialog.ui \
    ui/FileSystemBrowser.ui \
    ui/LicenseDialog.ui \
    ui/MainWindow.ui \
    ui/NewProjectDialog.ui \
    ui/ProjectManager.ui \
    ui/ProjectSettingsDialog.ui

HEADERS += \
    src/action/declaration.h \
    src/cegui/ceguitype_editor_properties.h \
    src/cegui/ceguitypes.h \
    src/editors/looknfeel/falagard_element_editor.h \
    src/editors/looknfeel/falagard_element_inspector.h \
    src/editors/looknfeel/falagard_element_interface.h \
    src/editors/looknfeel/hierarchy_dock_widget.h \
    src/editors/looknfeel/hierarchy_tree_item.h \
    src/editors/looknfeel/hierarchy_tree_model.h \
    src/editors/looknfeel/hierarchy_tree_view.h \
    src/editors/looknfeel/tabbed_editor.h \
    src/editors/looknfeel/undoable_commands.h \
    src/editors/code_edit_restoring_view.h \
    src/metaimageset/rectanglepacking.h \
    src/about.h \
    src/application.h \
    include/CEEDBase.h \
    include/CEEDConfig.h \
    src/commands.h \
    src/compileuifiles.h \
    src/error.h \
    src/fake.h \
    src/filesystembrowser.h \
    src/mainwindow.h \
    src/messages.h \
    src/paths.h \
    src/prerequisites.h \
    src/project.h \
    src/propertymapping.h \
    src/propertysetinspector.h \
    src/qtwidgets.h \
    src/recentlyused.h \
    src/resizable.h \
    src/version.h \
    src/xmledit.h \
    tests/compatibility/layout_cegui.h \
    tests/cli.h \
    src/qtorderedmap.h \
    include/optional.hpp \
    src/elementtree.h \
    src/action/action__init__.h \
    src/cegui/cegui_init.h \
    src/compatibility/animation_list/compat_animation_list_init.h \
    src/compatibility/compatibility_init.h \
    src/compatibility/font/compat_font_init.h \
    src/compatibility/imageset/compat_imageset_init.h \
    src/compatibility/layout/compat_layout_init.h \
    src/compatibility/looknfeel/compat_looknfeel_init.h \
    src/compatibility/project/compat_project_init.h \
    src/compatibility/property_mappings/compat_property_mappings_init.h \
    src/compatibility/scheme/compat_scheme_init.h \
    src/compatibility/font/compat_font_cegui.h \
    src/compatibility/imageset/compat_imageset_cegui.h \
    src/compatibility/layout/compat_layout_cegui.h \
    src/compatibility/looknfeel/compat_looknfeel_cegui.h \
    src/compatibility/scheme/compat_scheme_cegui.h \
    src/editors/animation_list/editor_animation_list_init.h \
    src/editors/animation_list/editor_animation_list_code.h \
    src/editors/animation_list/editor_animation_list_timeline.h \
    src/editors/animation_list/editor_animation_list_undo.h \
    src/editors/animation_list/editor_animation_list_visual.h \
    src/editors/bitmap/editor_bitmap_init.h \
    src/editors/imageset/editor_imageset_init.h \
    src/editors/imageset/editor_imageset_action_decl.h \
    src/editors/imageset/editor_imageset_code.h \
    src/editors/imageset/editor_imageset_elements.h \
    src/editors/imageset/editor_imageset_settings_decl.h \
    src/editors/imageset/editor_imageset_undo.h \
    src/editors/imageset/editor_imageset_visual.h \
    src/editors/layout/editor_layout_init.h \
    src/editors/layout/editor_layout_action_decl.h \
    src/editors/layout/editor_layout_code.h \
    src/editors/layout/editor_layout_preview.h \
    src/editors/layout/editor_layout_settings_decl.h \
    src/editors/layout/editor_layout_undo.h \
    src/editors/layout/editor_layout_visual.h \
    src/editors/layout/editor_layout_widgethelpers.h \
    src/editors/looknfeel/editor_looknfeel_init.h \
    src/editors/looknfeel/editor_looknfeel_action_decl.h \
    src/editors/looknfeel/editor_looknfeel_code.h \
    src/editors/looknfeel/editor_looknfeel_preview.h \
    src/editors/looknfeel/editor_looknfeel_settings_decl.h \
    src/editors/looknfeel/editor_looknfeel_visual.h \
    src/editors/property_mappings/editor_property_mappings_init.h \
    src/editors/text/editor_text_init.h \
    src/editors/editors_init.h \
    src/metaimageset/inputs/metaimageset_inputs_init.h \
    src/metaimageset/inputs/metaimageset_inputs_bitmap.h \
    src/metaimageset/inputs/metaimageset_inputs_imageset.h \
    src/metaimageset/inputs/metaimageset_inputs_inkscape_svg.h \
    src/metaimageset/inputs/metaimageset_inputs_qsvg.h \
    src/metaimageset/inputs/metaimageset_inputs_registry.h \
    src/metaimageset/metaimageset_init.h \
    src/metaimageset/metaimageset_compiler.h \
    src/propertytree/propertytree_init.h \
    src/propertytree/propertytree_compositeproperties.h \
    src/propertytree/propertytree_editors.h \
    src/propertytree/propertytree_parsers.h \
    src/propertytree/propertytree_properties.h \
    src/propertytree/propertytree_ui.h \
    src/propertytree/propertytree_utility.h \
    src/settings/settings_init.h \
    src/settings/settings_declaration.h \
    src/settings/settings_interface.h \
    src/settings/settings_interface_types.h \
    src/settings/settings_persistence.h \
    src/editors/editors_htmlview.h \
    src/editors/editors_multi.h \
    src/cegui/cegui_widgethelpers.h \
    src/cegui/cegui_settings_decl.h \
    src/cegui/cegui_qtgraphics.h \
    src/cegui/cegui_container.h \
    src/compatibility/imageset/compat_imageset_gorilla.h \
    src/compatibility/compatibility_ceguihelpers.h

SOURCES += \
    src/action/declaration.cpp \
    src/cegui/ceguitype_editor_properties.cpp \
    src/cegui/ceguitypes.cpp \
    src/editors/looknfeel/falagard_element_editor.cpp \
    src/editors/looknfeel/falagard_element_inspector.cpp \
    src/editors/looknfeel/falagard_element_interface.cpp \
    src/editors/looknfeel/hierarchy_dock_widget.cpp \
    src/editors/looknfeel/hierarchy_tree_item.cpp \
    src/editors/looknfeel/hierarchy_tree_model.cpp \
    src/editors/looknfeel/hierarchy_tree_view.cpp \
    src/editors/looknfeel/tabbed_editor.cpp \
    src/editors/looknfeel/undoable_commands.cpp \
    src/editors/code_edit_restoring_view.cpp \
    src/metaimageset/rectanglepacking.cpp \
    src/about.cpp \
    src/application.cpp \
    src/commands.cpp \
    src/compileuifiles.cpp \
    src/error.cpp \
    src/fake.cpp \
    src/filesystembrowser.cpp \
    src/mainwindow.cpp \
    src/messages.cpp \
    src/paths.cpp \
    src/prerequisites.cpp \
    src/project.cpp \
    src/propertymapping.cpp \
    src/propertysetinspector.cpp \
    src/qtwidgets.cpp \
    src/recentlyused.cpp \
    src/resizable.cpp \
    src/version.cpp \
    src/xmledit.cpp \
    src/elementtree.cpp \
    src/ceed_paths.cpp \
    src/action/action__init__.cpp \
    src/ceedbase.cpp \
    src/cegui/cegui_init.cpp \
    src/compatibility/animation_list/compat_animation_list_init.cpp \
    src/compatibility/compatibility_init.cpp \
    src/compatibility/font/compat_font_init.cpp \
    src/compatibility/imageset/compat_imageset_init.cpp \
    src/compatibility/layout/compat_layout_init.cpp \
    src/compatibility/property_mappings/compat_property_mappings_init__.cpp \
    src/compatibility/project/compat_project_init.cpp \
    src/compatibility/looknfeel/compat_looknfeel_init.cpp \
    src/compatibility/scheme/compat_scheme_init.cpp \
    src/compatibility/font/compat_font_cegui.cpp \
    src/compatibility/imageset/compat_imageset_cegui.cpp \
    src/compatibility/layout/compat_layout_cegui.cpp \
    src/compatibility/scheme/compat_scheme_cegui.cpp \
    src/compatibility/looknfeel/compat_looknfeel_cegui.cpp \
    src/editors/animation_list/editor_animation_list_init.cpp \
    src/editors/animation_list/editor_animation_list_code.cpp \
    src/editors/animation_list/editor_animation_list_timeline.cpp \
    src/editors/animation_list/editor_animation_list_undo.cpp \
    src/editors/animation_list/editor_animation_list_visual.cpp \
    src/editors/bitmap/editor_bitmap_init.cpp \
    src/editors/imageset/editor_imageset_init.cpp \
    src/editors/imageset/editor_imageset_action_decl.cpp \
    src/editors/imageset/editor_imageset_code.cpp \
    src/editors/imageset/editor_imageset_elements.cpp \
    src/editors/imageset/editor_imageset_settings_decl.cpp \
    src/editors/imageset/editor_imageset_undo.cpp \
    src/editors/imageset/editor_imageset_visual.cpp \
    src/editors/layout/editor_layout_init.cpp \
    src/editors/layout/editor_layout_code.cpp \
    src/editors/layout/editor_layout_action_decl.cpp \
    src/editors/layout/editor_layout_preview.cpp \
    src/editors/layout/editor_layout_settings_decl.cpp \
    src/editors/layout/editor_layout_undo.cpp \
    src/editors/layout/editor_layout_visual.cpp \
    src/editors/layout/editor_layout_widgethelpers.cpp \
    src/editors/looknfeel/editor_looknfeel_init.cpp \
    src/editors/looknfeel/editor_looknfeel_action_decl.cpp \
    src/editors/looknfeel/editor_looknfeel_code.cpp \
    src/editors/looknfeel/editor_looknfeel_settings_decl.cpp \
    src/editors/looknfeel/editor_looknfeel_visual.cpp \
    src/editors/property_mappings/editor_property_mappings_init.cpp \
    src/editors/text/editor_text_init.cpp \
    src/editors/editors_init.cpp \
    src/editors/looknfeel/editor_looknfeel_preview.cpp \
    src/metaimageset/inputs/metaimageset_inputs_init.cpp \
    src/metaimageset/inputs/metaimageset_inputs_bitmap.cpp \
    src/metaimageset/inputs/metaimageset_inputs_imageset.cpp \
    src/metaimageset/inputs/metaimageset_inputs_inkscape_svg.cpp \
    src/metaimageset/inputs/metaimageset_inputs_qsvg.cpp \
    src/metaimageset/inputs/metaimageset_inputs_registry.cpp \
    src/metaimageset/metaimageset_init.cpp \
    src/metaimageset/metaimageset_compiler.cpp \
    src/propertytree/propertytree_init.cpp \
    src/propertytree/propertytree_compositeproperties.cpp \
    src/propertytree/propertytree_editors.cpp \
    src/propertytree/propertytree_parsers.cpp \
    src/propertytree/propertytree_properties.cpp \
    src/propertytree/propertytree_ui.cpp \
    src/propertytree/propertytree_utility.cpp \
    src/settings/settings_init.cpp \
    src/settings/settings_declaration.cpp \
    src/settings/settings_interface.cpp \
    src/settings/settings_interface_types.cpp \
    src/settings/settings_persistence.cpp \
    src/editors/editors_htmlview.cpp \
    src/editors/editors_multi.cpp \
    src/cegui/cegui_container.cpp \
    src/cegui/cegui_qtgraphics.cpp \
    src/cegui/cegui_settings_decl.cpp \
    src/cegui/cegui_widgethelpers.cpp \
    src/compatibility/imageset/compat_imageset_gorilla.cpp \
    src/compatibility/compatibility_ceguihelpers.cpp \
    src/main.cpp \
    tests/compatibility/tests_compat_init.cpp \
    tests/compatibility/tests_layout_cegui.cpp \
    tests/tests_init.cpp \
    tests/tests_cli.cpp

# TODO: handle CEEDConfig.h.in

TEMPLATE = app

TARGET = ceed

QT += widgets opengl svg core

#CONFIG += object_parallel_to_source
#CONFIG += no_batch

DEFINES += QT_RESTRICTED_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

INCLUDEPATH += include src
INCLUDEPATH += C:/Programming/cegui/cegui/include/
INCLUDEPATH += C:/Programming/build-cegui-0-8/cegui/include/
INCLUDEPATH += C:/Programming/build-cegui-dependencies-0-8/dependencies/include/

cegui_libs_dir = C:/Programming/build-cegui-0-8/lib
#cegui_deps_dir = C:/Programming/cegui-dependencies/src
#INCLUDEPATH += $$cegui_deps_dir/glew-1.7.0/include
CONFIG(debug, debug|release) {
    win32:LIBS += $$cegui_libs_dir/CEGUIBase-0_d.lib $$cegui_libs_dir/CEGUIOpenGLRenderer-0_d.lib $$cegui_libs_dir/CEGUICommonDialogs-0_d.lib
}
CONFIG(release, debug|release) {
    win32:LIBS += $$cegui_libs_dir/CEGUIBase-0.lib $$cegui_libs_dir/CEGUIOpenGLRenderer-0.lib $$cegui_libs_dir/CEGUICommonDialogs-0.lib
}

RESOURCES += \
    ../ceed.qrc
