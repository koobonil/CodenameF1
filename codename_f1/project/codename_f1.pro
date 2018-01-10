TARGET = codename_f1
TEMPLATE = app

!include(../../Boss2D/project/boss2d.pri) {
    error("Couldn't find the boss2d.pri file...")
}

INCLUDEPATH += ../source
HEADERS += ../source/boss_config.h
HEADERS += ../source/classes_f1.hpp
HEADERS += ../source/codename_f1.hpp
HEADERS += ../source/resource.hpp
HEADERS += ../source/ingame_f1.hpp
HEADERS += ../source/maptool_f1.hpp
HEADERS += ../source/stagetool_f1.hpp
SOURCES += ../source/classes_f1.cpp
SOURCES += ../source/codename_f1.cpp
SOURCES += ../source/main.cpp
SOURCES += ../source/resource.cpp
SOURCES += ../source/ingame_f1.cpp
SOURCES += ../source/maptool_f1.cpp
SOURCES += ../source/stagetool_f1.cpp

INCLUDEPATH += ../../codename_fx/source
HEADERS += ../../codename_fx/source/classes.hpp
HEADERS += ../../codename_fx/source/outgame.hpp
SOURCES += ../../codename_fx/source/classes.cpp
SOURCES += ../../codename_fx/source/outgame.cpp

HEADERS += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/imagepacker.h
HEADERS += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/maxrects.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animatable.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation_container.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation_keyframe.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation_state.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation_state_set.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_animation_track.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_any_type.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_any_value.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_array_buffer.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_atlas.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_base.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_colour_value.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_forward_multiply_renderer.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_image.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_image_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_lerp.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_math.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_matrix3.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_matrix4.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_mesh_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_mesh_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_node.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_object_base.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_opengl.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_plane.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_quaternion.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_random_generator.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_renderer.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_render_command.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_render_priority.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_scene_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_scene_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_scene_node.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_scene_slot_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_scene_slot_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_skeleton_bone.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_skeleton_bone_render_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_skeleton_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_skeleton_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_spine_builder.hpp
HEADERS += ../../codename_fx/source/spine_for_zay/zay_texture2d.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_texture_atlas.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_texture_atlas_set.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_touch.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_touch_area_data.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_touch_area_instance.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_touch_event.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_touch_event_dispatcher.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_types.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_vector2.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_vector3.h
HEADERS += ../../codename_fx/source/spine_for_zay/zay_vector4.h
SOURCES += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/imagecompare.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/imagecrop.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/imagepacker.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/imagesort.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/cheetah_texture_packer/maxrects.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animatable.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation_container.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation_keyframe.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation_state.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation_state_set.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_animation_track.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_any_type.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_any_value.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_array_buffer.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_atlas.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_base.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_colour_value.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_forward_multiply_renderer.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_image.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_image_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_lerp.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_math.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_matrix3.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_matrix4.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_mesh_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_mesh_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_node.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_object_base.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_opengl.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_plane.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_quaternion.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_random_generator.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_renderer.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_render_command.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_render_priority.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_scene_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_scene_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_scene_node.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_scene_slot_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_scene_slot_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_skeleton_bone.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_skeleton_bone_render_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_skeleton_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_skeleton_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_spine_builder.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_texture2d.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_texture_atlas.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_texture_atlas_set.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_touch.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_touch_area_data.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_touch_area_instance.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_touch_event.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_touch_event_dispatcher.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_types.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_vector2.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_vector3.cpp
SOURCES += ../../codename_fx/source/spine_for_zay/zay_vector4.cpp

ASSETS_IMAGE.files += ../assets/f1/image
ASSETS_IMAGE.path = /assets/f1
ASSETS_TABLE.files += ../assets/f1/table
ASSETS_TABLE.path = /assets/f1
ASSETS_SPINE.files += ../assets/f1/spine
ASSETS_SPINE.path = /assets/f1
ASSETS_IMAGE_FX.files += ../../codename_fx/assets/fx/image
ASSETS_IMAGE_FX.path = /assets/fx
ASSETS_TABLE_FX.files += ../../codename_fx/assets/fx/table
ASSETS_TABLE_FX.path = /assets/fx
ASSETS_SPINE_FX.files += ../../codename_fx/assets/fx/spine
ASSETS_SPINE_FX.path = /assets/fx

win32{
    RC_ICONS += ../common/windows/main.ico
}

android{
    INSTALLS += ASSETS_IMAGE
    INSTALLS += ASSETS_TABLE
    INSTALLS += ASSETS_SPINE
    INSTALLS += ASSETS_IMAGE_FX
    INSTALLS += ASSETS_TABLE_FX
    INSTALLS += ASSETS_SPINE_FX
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../common/android
}

ios|macx{
    QMAKE_BUNDLE_DATA += ASSETS_IMAGE
    QMAKE_BUNDLE_DATA += ASSETS_TABLE
    QMAKE_BUNDLE_DATA += ASSETS_SPINE
    QMAKE_BUNDLE_DATA += ASSETS_IMAGE_FX
    QMAKE_BUNDLE_DATA += ASSETS_TABLE_FX
    QMAKE_BUNDLE_DATA += ASSETS_SPINE_FX
    QMAKE_BUNDLE_DATA += MAIN_ICON
    QMAKE_INFO_PLIST = $$PWD/../common/ios/Info.plist
    MAIN_ICON.files += $$PWD/../common/ios/res/icon.icns
    ios: MAIN_ICON.path = /
    macx: MAIN_ICON.path = /Contents/Resources
}

