TARGET = codename_f1
TEMPLATE = app

!include(../../Boss2D/project/boss2d.pri) {
    error("Couldn't find the boss2d.pri file...")
}

INCLUDEPATH += ../source
HEADERS += ../source/boss_config.h
SOURCES += ../source/classes.cpp
HEADERS += ../source/classes.hpp
SOURCES += ../source/codename_f1.cpp
HEADERS += ../source/codename_f1.hpp
SOURCES += ../source/main.cpp
SOURCES += ../source/r.cpp
HEADERS += ../source/r.hpp
SOURCES += ../source/ingame.cpp
HEADERS += ../source/ingame.hpp
SOURCES += ../source/maptool.cpp
HEADERS += ../source/maptool.hpp
SOURCES += ../source/stagetool.cpp
HEADERS += ../source/stagetool.hpp
HEADERS += ../source/spine_for_zay/cheetah_texture_packer/imagepacker.h
HEADERS += ../source/spine_for_zay/cheetah_texture_packer/maxrects.h
HEADERS += ../source/spine_for_zay/zay_animatable.h
HEADERS += ../source/spine_for_zay/zay_animation.h
HEADERS += ../source/spine_for_zay/zay_animation_container.h
HEADERS += ../source/spine_for_zay/zay_animation_keyframe.h
HEADERS += ../source/spine_for_zay/zay_animation_state.h
HEADERS += ../source/spine_for_zay/zay_animation_state_set.h
HEADERS += ../source/spine_for_zay/zay_animation_track.h
HEADERS += ../source/spine_for_zay/zay_any_type.h
HEADERS += ../source/spine_for_zay/zay_any_value.h
HEADERS += ../source/spine_for_zay/zay_array_buffer.h
HEADERS += ../source/spine_for_zay/zay_atlas.h
HEADERS += ../source/spine_for_zay/zay_base.h
HEADERS += ../source/spine_for_zay/zay_colour_value.h
HEADERS += ../source/spine_for_zay/zay_forward_multiply_renderer.h
HEADERS += ../source/spine_for_zay/zay_image.h
HEADERS += ../source/spine_for_zay/zay_image_data.h
HEADERS += ../source/spine_for_zay/zay_lerp.h
HEADERS += ../source/spine_for_zay/zay_math.h
HEADERS += ../source/spine_for_zay/zay_matrix3.h
HEADERS += ../source/spine_for_zay/zay_matrix4.h
HEADERS += ../source/spine_for_zay/zay_mesh_data.h
HEADERS += ../source/spine_for_zay/zay_mesh_instance.h
HEADERS += ../source/spine_for_zay/zay_node.h
HEADERS += ../source/spine_for_zay/zay_object_base.h
HEADERS += ../source/spine_for_zay/zay_opengl.h
HEADERS += ../source/spine_for_zay/zay_plane.h
HEADERS += ../source/spine_for_zay/zay_quaternion.h
HEADERS += ../source/spine_for_zay/zay_random_generator.h
HEADERS += ../source/spine_for_zay/zay_renderer.h
HEADERS += ../source/spine_for_zay/zay_render_command.h
HEADERS += ../source/spine_for_zay/zay_render_priority.h
HEADERS += ../source/spine_for_zay/zay_scene_data.h
HEADERS += ../source/spine_for_zay/zay_scene_instance.h
HEADERS += ../source/spine_for_zay/zay_scene_node.h
HEADERS += ../source/spine_for_zay/zay_scene_slot_data.h
HEADERS += ../source/spine_for_zay/zay_scene_slot_instance.h
HEADERS += ../source/spine_for_zay/zay_skeleton_bone.h
HEADERS += ../source/spine_for_zay/zay_skeleton_bone_render_instance.h
HEADERS += ../source/spine_for_zay/zay_skeleton_data.h
HEADERS += ../source/spine_for_zay/zay_skeleton_instance.h
HEADERS += ../source/spine_for_zay/zay_spine_builder.hpp
HEADERS += ../source/spine_for_zay/zay_texture2d.h
HEADERS += ../source/spine_for_zay/zay_texture_atlas.h
HEADERS += ../source/spine_for_zay/zay_texture_atlas_set.h
HEADERS += ../source/spine_for_zay/zay_touch.h
HEADERS += ../source/spine_for_zay/zay_touch_area_data.h
HEADERS += ../source/spine_for_zay/zay_touch_area_instance.h
HEADERS += ../source/spine_for_zay/zay_touch_event.h
HEADERS += ../source/spine_for_zay/zay_touch_event_dispatcher.h
HEADERS += ../source/spine_for_zay/zay_types.h
HEADERS += ../source/spine_for_zay/zay_vector2.h
HEADERS += ../source/spine_for_zay/zay_vector3.h
HEADERS += ../source/spine_for_zay/zay_vector4.h

SOURCES += ../source/spine_for_zay/cheetah_texture_packer/imagecompare.cpp
SOURCES += ../source/spine_for_zay/cheetah_texture_packer/imagecrop.cpp
SOURCES += ../source/spine_for_zay/cheetah_texture_packer/imagepacker.cpp
SOURCES += ../source/spine_for_zay/cheetah_texture_packer/imagesort.cpp
SOURCES += ../source/spine_for_zay/cheetah_texture_packer/maxrects.cpp
SOURCES += ../source/spine_for_zay/zay_animatable.cpp
SOURCES += ../source/spine_for_zay/zay_animation.cpp
SOURCES += ../source/spine_for_zay/zay_animation_container.cpp
SOURCES += ../source/spine_for_zay/zay_animation_keyframe.cpp
SOURCES += ../source/spine_for_zay/zay_animation_state.cpp
SOURCES += ../source/spine_for_zay/zay_animation_state_set.cpp
SOURCES += ../source/spine_for_zay/zay_animation_track.cpp
SOURCES += ../source/spine_for_zay/zay_any_type.cpp
SOURCES += ../source/spine_for_zay/zay_any_value.cpp
SOURCES += ../source/spine_for_zay/zay_array_buffer.cpp
SOURCES += ../source/spine_for_zay/zay_atlas.cpp
SOURCES += ../source/spine_for_zay/zay_base.cpp
SOURCES += ../source/spine_for_zay/zay_colour_value.cpp
SOURCES += ../source/spine_for_zay/zay_forward_multiply_renderer.cpp
SOURCES += ../source/spine_for_zay/zay_image.cpp
SOURCES += ../source/spine_for_zay/zay_image_data.cpp
SOURCES += ../source/spine_for_zay/zay_lerp.cpp
SOURCES += ../source/spine_for_zay/zay_math.cpp
SOURCES += ../source/spine_for_zay/zay_matrix3.cpp
SOURCES += ../source/spine_for_zay/zay_matrix4.cpp
SOURCES += ../source/spine_for_zay/zay_mesh_data.cpp
SOURCES += ../source/spine_for_zay/zay_mesh_instance.cpp
SOURCES += ../source/spine_for_zay/zay_node.cpp
SOURCES += ../source/spine_for_zay/zay_object_base.cpp
SOURCES += ../source/spine_for_zay/zay_opengl.cpp
SOURCES += ../source/spine_for_zay/zay_plane.cpp
SOURCES += ../source/spine_for_zay/zay_quaternion.cpp
SOURCES += ../source/spine_for_zay/zay_random_generator.cpp
SOURCES += ../source/spine_for_zay/zay_renderer.cpp
SOURCES += ../source/spine_for_zay/zay_render_command.cpp
SOURCES += ../source/spine_for_zay/zay_render_priority.cpp
SOURCES += ../source/spine_for_zay/zay_scene_data.cpp
SOURCES += ../source/spine_for_zay/zay_scene_instance.cpp
SOURCES += ../source/spine_for_zay/zay_scene_node.cpp
SOURCES += ../source/spine_for_zay/zay_scene_slot_data.cpp
SOURCES += ../source/spine_for_zay/zay_scene_slot_instance.cpp
SOURCES += ../source/spine_for_zay/zay_skeleton_bone.cpp
SOURCES += ../source/spine_for_zay/zay_skeleton_bone_render_instance.cpp
SOURCES += ../source/spine_for_zay/zay_skeleton_data.cpp
SOURCES += ../source/spine_for_zay/zay_skeleton_instance.cpp
SOURCES += ../source/spine_for_zay/zay_spine_builder.cpp
SOURCES += ../source/spine_for_zay/zay_texture2d.cpp
SOURCES += ../source/spine_for_zay/zay_texture_atlas.cpp
SOURCES += ../source/spine_for_zay/zay_texture_atlas_set.cpp
SOURCES += ../source/spine_for_zay/zay_touch.cpp
SOURCES += ../source/spine_for_zay/zay_touch_area_data.cpp
SOURCES += ../source/spine_for_zay/zay_touch_area_instance.cpp
SOURCES += ../source/spine_for_zay/zay_touch_event.cpp
SOURCES += ../source/spine_for_zay/zay_touch_event_dispatcher.cpp
SOURCES += ../source/spine_for_zay/zay_types.cpp
SOURCES += ../source/spine_for_zay/zay_vector2.cpp
SOURCES += ../source/spine_for_zay/zay_vector3.cpp
SOURCES += ../source/spine_for_zay/zay_vector4.cpp

ASSETS_IMAGE.files += ../assets/image
ASSETS_IMAGE.path = /assets
ASSETS_JSON.files += ../assets/json
ASSETS_JSON.path = /assets
ASSETS_SPINE.files += ../assets/spine
ASSETS_SPINE.path = /assets

win32{
    RC_ICONS += ../common/windows/main.ico
}

android{
    INSTALLS += ASSETS_IMAGE
    INSTALLS += ASSETS_JSON
    INSTALLS += ASSETS_SPINE
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../common/android
}

ios|macx{
    QMAKE_BUNDLE_DATA += ASSETS_IMAGE
    QMAKE_BUNDLE_DATA += ASSETS_JSON
    QMAKE_BUNDLE_DATA += ASSETS_SPINE
    QMAKE_BUNDLE_DATA += MAIN_ICON
    QMAKE_INFO_PLIST = $$PWD/../common/ios/Info.plist
    MAIN_ICON.files += $$PWD/../common/ios/res/icon.icns
    ios: MAIN_ICON.path = /
    macx: MAIN_ICON.path = /Contents/Resources
}
