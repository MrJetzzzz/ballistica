cmake_minimum_required(VERSION 3.12)
project(BallisticaCore)
include(CheckIncludeFile)

option(HEADLESS "build headless server" OFF)
option(TEST_BUILD "include testing features" OFF)

# Requiring minimum of C++17 currently.
set(CMAKE_CXX_STANDARD 17)

if (APPLE)
  # Seems as of Mojave we need to explicitly pull in homebrew paths.
  # Just hard-coding recommended homebrew install paths for now.
  # Is there a more elegant way to do this?
  if (CMAKE_SYSTEM_PROCESSOR MATCHES arm64)
    list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
    include_directories("/opt/homebrew/include")
    link_directories("/opt/homebrew/lib")
  else()
    list(APPEND CMAKE_PREFIX_PATH /usr/local)
    include_directories("/usr/local/include")
    link_directories("/usr/local/lib")
  endif()

  # On Mac with homebrew it seems that Requesting 3.X when we've got
  # 3.(X+1) installed will point us at the 3.(X+1) framework but will attempt
  # to load a 3.X library from within it which doesn't exist. So we need
  # to be a bit more explicit telling it where to look. Note: this was last
  # tested with 3.7; should revisit sometime to make sure still applies.
  execute_process(COMMAND "python3.10-config" "--prefix"
    OUTPUT_VARIABLE Python_ROOT_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

endif ()
find_package (Python 3.10 REQUIRED EXACT COMPONENTS Development)
  

if (HEADLESS)
  add_definitions(-DBA_HEADLESS_BUILD=1)
 else ()
  find_package(SDL2 QUIET)
  if (SDL2_FOUND)
    if ("${SDL2_LIBRARIES}" STREQUAL "")
      message(WARNING "SDL2_LIBRARIES wasn't set, manually setting to SDL2::SDL2")
      set(SDL2_LIBRARIES "SDL2::SDL2")
    endif ()
    # Getting complaint about space at the end of this on ubuntu16.
    string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
  else ()
    message(FATAL_ERROR "SDL2 not found")
  endif ()
  find_package(OpenGL REQUIRED)
  find_package(OpenAL REQUIRED)
  if (APPLE)
    # On mac this sets an include path that we don't need since
    # we're using the system framework... should clean this up.
    set(OPENAL_INCLUDE_DIR "")
  endif ()
  find_library(OGG_LIBRARY ogg)
  find_library(VORBISFILE_LIBRARY vorbisfile)
  if (NOT OGG_LIBRARY)
    message(FATAL_ERROR "ogg library not found")
  endif ()
  if (NOT VORBISFILE_LIBRARY)
    message(FATAL_ERROR "vorbisfile library not found")
  endif ()
  set(EXTRA_INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
  set(EXTRA_LIBRARIES ogg vorbisfile ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY})
endif ()

if (TEST_BUILD)
  add_definitions(-DBA_TEST_BUILD=1)
endif ()

# Currently seeing warnings about parameter order changing in GCC 7.1
# on Raspberry Pi builds. We never need to care about C++ abi compatibility
# so just silencing them for now. Can maybe remove this later if they stop.
if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

set(BA_SRC_ROOT ../src)
include_directories(${BA_SRC_ROOT})
add_compile_options(-include ballistica/config/config_cmake.h)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DBA_DEBUG_BUILD=1)
else ()
  # It seems that cmake can choose -O2 sometimes and -O3 sometimes
  # for release builds (depending on Release vs RelWithDebInfo, etc).
  # Let's keep all our non-debug builds consistent at -O3 for now; can
  # revisit if it causes problems.
  add_definitions(-O3)
endif ()

set(ODE_SRC_ROOT ${BA_SRC_ROOT}/external/open_dynamics_engine-ef)

add_library(ode
  ${ODE_SRC_ROOT}/ode/IceAABB.cpp
  ${ODE_SRC_ROOT}/ode/IceContainer.cpp
  ${ODE_SRC_ROOT}/ode/IceHPoint.cpp
  ${ODE_SRC_ROOT}/ode/IceIndexedTriangle.cpp
  ${ODE_SRC_ROOT}/ode/IceMatrix3x3.cpp
  ${ODE_SRC_ROOT}/ode/IceMatrix4x4.cpp
  ${ODE_SRC_ROOT}/ode/IceOBB.cpp
  ${ODE_SRC_ROOT}/ode/IcePlane.cpp
  ${ODE_SRC_ROOT}/ode/IcePoint.cpp
  ${ODE_SRC_ROOT}/ode/IceRandom.cpp
  ${ODE_SRC_ROOT}/ode/IceRay.cpp
  ${ODE_SRC_ROOT}/ode/IceRevisitedRadix.cpp
  ${ODE_SRC_ROOT}/ode/IceSegment.cpp
  ${ODE_SRC_ROOT}/ode/IceTriangle.cpp
  ${ODE_SRC_ROOT}/ode/IceUtils.cpp
  ${ODE_SRC_ROOT}/ode/ode.cpp
  ${ODE_SRC_ROOT}/ode/ode_array.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_cylinder_box.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_cylinder_plane.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_cylinder_sphere.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_cylinder_trimesh.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_kernel.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_quadtreespace.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_sapspace.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_space.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_std.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_transform.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_box.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_ccylinder.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_distance.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_plane.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_ray.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_sphere.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_trimesh_trimesh.cpp
  ${ODE_SRC_ROOT}/ode/ode_collision_util.cpp
  ${ODE_SRC_ROOT}/ode/ode_error.cpp
  ${ODE_SRC_ROOT}/ode/ode_export-diff.cpp
  ${ODE_SRC_ROOT}/ode/ode_fastdot.cpp
  ${ODE_SRC_ROOT}/ode/ode_fastldlt.cpp
  ${ODE_SRC_ROOT}/ode/ode_fastlsolve.cpp
  ${ODE_SRC_ROOT}/ode/ode_fastltsolve.cpp
  ${ODE_SRC_ROOT}/ode/ode_joint.cpp
  ${ODE_SRC_ROOT}/ode/ode_lcp.cpp
  ${ODE_SRC_ROOT}/ode/ode_mass.cpp
  ${ODE_SRC_ROOT}/ode/ode_mat.cpp
  ${ODE_SRC_ROOT}/ode/ode_math.cpp
  ${ODE_SRC_ROOT}/ode/ode_matrix.cpp
  ${ODE_SRC_ROOT}/ode/ode_memory.cpp
  ${ODE_SRC_ROOT}/ode/ode_misc.cpp
  ${ODE_SRC_ROOT}/ode/ode_obstack.cpp
  ${ODE_SRC_ROOT}/ode/ode_quickstep.cpp
  ${ODE_SRC_ROOT}/ode/ode_rotation.cpp
  ${ODE_SRC_ROOT}/ode/ode_step.cpp
  ${ODE_SRC_ROOT}/ode/ode_stepfast.cpp
  ${ODE_SRC_ROOT}/ode/ode_timer.cpp
  ${ODE_SRC_ROOT}/ode/ode_util.cpp
  ${ODE_SRC_ROOT}/ode/OPC_AABBCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_AABBTree.cpp
  ${ODE_SRC_ROOT}/ode/OPC_BaseModel.cpp
  ${ODE_SRC_ROOT}/ode/OPC_BoxPruning.cpp
  ${ODE_SRC_ROOT}/ode/OPC_Collider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_HybridModel.cpp
  ${ODE_SRC_ROOT}/ode/OPC_LSSCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_MeshInterface.cpp
  ${ODE_SRC_ROOT}/ode/OPC_Model.cpp
  ${ODE_SRC_ROOT}/ode/OPC_OBBCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_OptimizedTree.cpp
  ${ODE_SRC_ROOT}/ode/OPC_PlanesCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_RayCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_SphereCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_SweepAndPrune.cpp
  ${ODE_SRC_ROOT}/ode/OPC_TreeBuilders.cpp
  ${ODE_SRC_ROOT}/ode/OPC_TreeCollider.cpp
  ${ODE_SRC_ROOT}/ode/OPC_VolumeCollider.cpp
  ${ODE_SRC_ROOT}/ode/Opcode.cpp
  )
target_include_directories(ode PRIVATE ${ODE_SRC_ROOT})

# NOTE: There used to be an issue with optimized GCC builds where mesh
# collisions would fail randomly, leading to characters falling through
# floors somewhat regularly. For this reason I was limiting optimization to
# -O1 for the rigid body library. However, as of April 2021, all seems
# well when testing on arm64 and x86-64 linux builds. (I think)
# The last time I remember seeing this bug was around 2016 I believe, but I
# haven't looked for it since. Perhaps GCC was fixed or perhaps the error
# was limited to 32 bit x86 builds; in either case we should be good, as
# we're no longer building any 32 bit x86 builds using GCC.
# Keeping this in here commented out just in case it rears its ugly head
# again though.
# if (CMAKE_BUILD_TYPE MATCHES Release)
#   if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
#     target_compile_options(ode PRIVATE -O1)
#   endif()
# endif ()

# BallisticaCore binary.
add_executable(ballisticacore
  ${BA_SRC_ROOT}/external/qr_code_generator/QrCode.cpp
  # AUTOGENERATED_PUBLIC_BEGIN (this section is managed by the "update_project" tool)
  ${BA_SRC_ROOT}/ballistica/app/app.cc
  ${BA_SRC_ROOT}/ballistica/app/app.h
  ${BA_SRC_ROOT}/ballistica/app/app_config.cc
  ${BA_SRC_ROOT}/ballistica/app/app_config.h
  ${BA_SRC_ROOT}/ballistica/app/app_globals.cc
  ${BA_SRC_ROOT}/ballistica/app/app_globals.h
  ${BA_SRC_ROOT}/ballistica/app/headless_app.cc
  ${BA_SRC_ROOT}/ballistica/app/headless_app.h
  ${BA_SRC_ROOT}/ballistica/app/stress_test.cc
  ${BA_SRC_ROOT}/ballistica/app/stress_test.h
  ${BA_SRC_ROOT}/ballistica/app/vr_app.cc
  ${BA_SRC_ROOT}/ballistica/app/vr_app.h
  ${BA_SRC_ROOT}/ballistica/audio/al_sys.cc
  ${BA_SRC_ROOT}/ballistica/audio/al_sys.h
  ${BA_SRC_ROOT}/ballistica/audio/audio.cc
  ${BA_SRC_ROOT}/ballistica/audio/audio.h
  ${BA_SRC_ROOT}/ballistica/audio/audio_server.cc
  ${BA_SRC_ROOT}/ballistica/audio/audio_server.h
  ${BA_SRC_ROOT}/ballistica/audio/audio_source.cc
  ${BA_SRC_ROOT}/ballistica/audio/audio_source.h
  ${BA_SRC_ROOT}/ballistica/audio/audio_streamer.cc
  ${BA_SRC_ROOT}/ballistica/audio/audio_streamer.h
  ${BA_SRC_ROOT}/ballistica/audio/ogg_stream.cc
  ${BA_SRC_ROOT}/ballistica/audio/ogg_stream.h
  ${BA_SRC_ROOT}/ballistica/ballistica.cc
  ${BA_SRC_ROOT}/ballistica/ballistica.h
  ${BA_SRC_ROOT}/ballistica/config/config_cmake.h
  ${BA_SRC_ROOT}/ballistica/config/config_common.h
  ${BA_SRC_ROOT}/ballistica/config/config_windows_common.h
  ${BA_SRC_ROOT}/ballistica/config/config_windows_generic.h
  ${BA_SRC_ROOT}/ballistica/config/config_windows_headless.h
  ${BA_SRC_ROOT}/ballistica/core/context.cc
  ${BA_SRC_ROOT}/ballistica/core/context.h
  ${BA_SRC_ROOT}/ballistica/core/exception.cc
  ${BA_SRC_ROOT}/ballistica/core/exception.h
  ${BA_SRC_ROOT}/ballistica/core/fatal_error.cc
  ${BA_SRC_ROOT}/ballistica/core/fatal_error.h
  ${BA_SRC_ROOT}/ballistica/core/inline.cc
  ${BA_SRC_ROOT}/ballistica/core/inline.h
  ${BA_SRC_ROOT}/ballistica/core/logging.cc
  ${BA_SRC_ROOT}/ballistica/core/logging.h
  ${BA_SRC_ROOT}/ballistica/core/macros.cc
  ${BA_SRC_ROOT}/ballistica/core/macros.h
  ${BA_SRC_ROOT}/ballistica/core/module.cc
  ${BA_SRC_ROOT}/ballistica/core/module.h
  ${BA_SRC_ROOT}/ballistica/core/object.cc
  ${BA_SRC_ROOT}/ballistica/core/object.h
  ${BA_SRC_ROOT}/ballistica/core/thread.cc
  ${BA_SRC_ROOT}/ballistica/core/thread.h
  ${BA_SRC_ROOT}/ballistica/core/types.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_draw_snapshot.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_fuse.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_fuse.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_fuse_data.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_height_cache.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_height_cache.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_server.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_server.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_shadow.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_shadow.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_shadow_data.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_volume_light.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_volume_light.h
  ${BA_SRC_ROOT}/ballistica/dynamics/bg/bg_dynamics_volume_light_data.h
  ${BA_SRC_ROOT}/ballistica/dynamics/collision.h
  ${BA_SRC_ROOT}/ballistica/dynamics/collision_cache.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/collision_cache.h
  ${BA_SRC_ROOT}/ballistica/dynamics/dynamics.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/dynamics.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/impact_sound_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/impact_sound_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_component.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_component.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_condition_node.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_condition_node.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_context.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/material_context.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_message_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_message_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_mod_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_mod_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_user_message_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/node_user_message_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/part_mod_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/part_mod_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/python_call_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/python_call_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/roll_sound_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/roll_sound_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/skid_sound_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/skid_sound_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/material/sound_material_action.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/material/sound_material_action.h
  ${BA_SRC_ROOT}/ballistica/dynamics/part.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/part.h
  ${BA_SRC_ROOT}/ballistica/dynamics/rigid_body.cc
  ${BA_SRC_ROOT}/ballistica/dynamics/rigid_body.h
  ${BA_SRC_ROOT}/ballistica/game/account.cc
  ${BA_SRC_ROOT}/ballistica/game/account.h
  ${BA_SRC_ROOT}/ballistica/game/client_controller_interface.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_set.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_set.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_client.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_client.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_client_udp.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_client_udp.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_host.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_host.h
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_host_udp.cc
  ${BA_SRC_ROOT}/ballistica/game/connection/connection_to_host_udp.h
  ${BA_SRC_ROOT}/ballistica/game/friend_score_set.h
  ${BA_SRC_ROOT}/ballistica/game/game.cc
  ${BA_SRC_ROOT}/ballistica/game/game.h
  ${BA_SRC_ROOT}/ballistica/game/game_stream.cc
  ${BA_SRC_ROOT}/ballistica/game/game_stream.h
  ${BA_SRC_ROOT}/ballistica/game/host_activity.cc
  ${BA_SRC_ROOT}/ballistica/game/host_activity.h
  ${BA_SRC_ROOT}/ballistica/game/player.cc
  ${BA_SRC_ROOT}/ballistica/game/player.h
  ${BA_SRC_ROOT}/ballistica/game/player_spec.cc
  ${BA_SRC_ROOT}/ballistica/game/player_spec.h
  ${BA_SRC_ROOT}/ballistica/game/score_to_beat.h
  ${BA_SRC_ROOT}/ballistica/game/session/client_session.cc
  ${BA_SRC_ROOT}/ballistica/game/session/client_session.h
  ${BA_SRC_ROOT}/ballistica/game/session/host_session.cc
  ${BA_SRC_ROOT}/ballistica/game/session/host_session.h
  ${BA_SRC_ROOT}/ballistica/game/session/net_client_session.cc
  ${BA_SRC_ROOT}/ballistica/game/session/net_client_session.h
  ${BA_SRC_ROOT}/ballistica/game/session/replay_client_session.cc
  ${BA_SRC_ROOT}/ballistica/game/session/replay_client_session.h
  ${BA_SRC_ROOT}/ballistica/game/session/session.cc
  ${BA_SRC_ROOT}/ballistica/game/session/session.h
  ${BA_SRC_ROOT}/ballistica/generic/base64.cc
  ${BA_SRC_ROOT}/ballistica/generic/base64.h
  ${BA_SRC_ROOT}/ballistica/generic/buffer.h
  ${BA_SRC_ROOT}/ballistica/generic/huffman.cc
  ${BA_SRC_ROOT}/ballistica/generic/huffman.h
  ${BA_SRC_ROOT}/ballistica/generic/json.cc
  ${BA_SRC_ROOT}/ballistica/generic/json.h
  ${BA_SRC_ROOT}/ballistica/generic/lambda_runnable.h
  ${BA_SRC_ROOT}/ballistica/generic/real_timer.h
  ${BA_SRC_ROOT}/ballistica/generic/runnable.cc
  ${BA_SRC_ROOT}/ballistica/generic/runnable.h
  ${BA_SRC_ROOT}/ballistica/generic/timer.cc
  ${BA_SRC_ROOT}/ballistica/generic/timer.h
  ${BA_SRC_ROOT}/ballistica/generic/timer_list.cc
  ${BA_SRC_ROOT}/ballistica/generic/timer_list.h
  ${BA_SRC_ROOT}/ballistica/generic/utf8.cc
  ${BA_SRC_ROOT}/ballistica/generic/utf8.h
  ${BA_SRC_ROOT}/ballistica/generic/utils.cc
  ${BA_SRC_ROOT}/ballistica/generic/utils.h
  ${BA_SRC_ROOT}/ballistica/graphics/area_of_interest.cc
  ${BA_SRC_ROOT}/ballistica/graphics/area_of_interest.h
  ${BA_SRC_ROOT}/ballistica/graphics/camera.cc
  ${BA_SRC_ROOT}/ballistica/graphics/camera.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/empty_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/object_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/object_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/post_process_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/post_process_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/render_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/render_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/shield_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/shield_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/simple_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/simple_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/smoke_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/smoke_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/special_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/special_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/component/sprite_component.cc
  ${BA_SRC_ROOT}/ballistica/graphics/component/sprite_component.h
  ${BA_SRC_ROOT}/ballistica/graphics/frame_def.cc
  ${BA_SRC_ROOT}/ballistica/graphics/frame_def.h
  ${BA_SRC_ROOT}/ballistica/graphics/framebuffer.h
  ${BA_SRC_ROOT}/ballistica/graphics/gl/gl_sys.cc
  ${BA_SRC_ROOT}/ballistica/graphics/gl/gl_sys.h
  ${BA_SRC_ROOT}/ballistica/graphics/gl/renderer_gl.cc
  ${BA_SRC_ROOT}/ballistica/graphics/gl/renderer_gl.h
  ${BA_SRC_ROOT}/ballistica/graphics/graphics.cc
  ${BA_SRC_ROOT}/ballistica/graphics/graphics.h
  ${BA_SRC_ROOT}/ballistica/graphics/graphics_server.cc
  ${BA_SRC_ROOT}/ballistica/graphics/graphics_server.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/image_mesh.cc
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/image_mesh.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_buffer.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_buffer_base.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_buffer_vertex_simple_full.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_buffer_vertex_smoke_full.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_buffer_vertex_sprite.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_data.cc
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_data.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_data_client_handle.cc
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_data_client_handle.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_index_buffer_16.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_index_buffer_32.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_base.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_dual_texture_full.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_object_split.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_simple_full.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_simple_split.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_smoke_full.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_indexed_static_dynamic.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_non_indexed.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/mesh_renderer_data.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/sprite_mesh.h
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/text_mesh.cc
  ${BA_SRC_ROOT}/ballistica/graphics/mesh/text_mesh.h
  ${BA_SRC_ROOT}/ballistica/graphics/net_graph.cc
  ${BA_SRC_ROOT}/ballistica/graphics/net_graph.h
  ${BA_SRC_ROOT}/ballistica/graphics/render_command_buffer.h
  ${BA_SRC_ROOT}/ballistica/graphics/render_pass.cc
  ${BA_SRC_ROOT}/ballistica/graphics/render_pass.h
  ${BA_SRC_ROOT}/ballistica/graphics/render_target.cc
  ${BA_SRC_ROOT}/ballistica/graphics/render_target.h
  ${BA_SRC_ROOT}/ballistica/graphics/renderer.cc
  ${BA_SRC_ROOT}/ballistica/graphics/renderer.h
  ${BA_SRC_ROOT}/ballistica/graphics/text/font_page_map_data.h
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_graphics.cc
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_graphics.h
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_group.cc
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_group.h
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_packer.cc
  ${BA_SRC_ROOT}/ballistica/graphics/text/text_packer.h
  ${BA_SRC_ROOT}/ballistica/graphics/texture/dds.cc
  ${BA_SRC_ROOT}/ballistica/graphics/texture/dds.h
  ${BA_SRC_ROOT}/ballistica/graphics/texture/ktx.cc
  ${BA_SRC_ROOT}/ballistica/graphics/texture/ktx.h
  ${BA_SRC_ROOT}/ballistica/graphics/texture/pvr.cc
  ${BA_SRC_ROOT}/ballistica/graphics/texture/pvr.h
  ${BA_SRC_ROOT}/ballistica/graphics/vr_graphics.cc
  ${BA_SRC_ROOT}/ballistica/graphics/vr_graphics.h
  ${BA_SRC_ROOT}/ballistica/input/device/client_input_device.cc
  ${BA_SRC_ROOT}/ballistica/input/device/client_input_device.h
  ${BA_SRC_ROOT}/ballistica/input/device/input_device.cc
  ${BA_SRC_ROOT}/ballistica/input/device/input_device.h
  ${BA_SRC_ROOT}/ballistica/input/device/joystick.cc
  ${BA_SRC_ROOT}/ballistica/input/device/joystick.h
  ${BA_SRC_ROOT}/ballistica/input/device/keyboard_input.cc
  ${BA_SRC_ROOT}/ballistica/input/device/keyboard_input.h
  ${BA_SRC_ROOT}/ballistica/input/device/test_input.cc
  ${BA_SRC_ROOT}/ballistica/input/device/test_input.h
  ${BA_SRC_ROOT}/ballistica/input/device/touch_input.cc
  ${BA_SRC_ROOT}/ballistica/input/device/touch_input.h
  ${BA_SRC_ROOT}/ballistica/input/input.cc
  ${BA_SRC_ROOT}/ballistica/input/input.h
  ${BA_SRC_ROOT}/ballistica/input/remote_app.cc
  ${BA_SRC_ROOT}/ballistica/input/remote_app.h
  ${BA_SRC_ROOT}/ballistica/input/std_input_module.cc
  ${BA_SRC_ROOT}/ballistica/input/std_input_module.h
  ${BA_SRC_ROOT}/ballistica/math/matrix44f.cc
  ${BA_SRC_ROOT}/ballistica/math/matrix44f.h
  ${BA_SRC_ROOT}/ballistica/math/point2d.h
  ${BA_SRC_ROOT}/ballistica/math/random.cc
  ${BA_SRC_ROOT}/ballistica/math/random.h
  ${BA_SRC_ROOT}/ballistica/math/rect.h
  ${BA_SRC_ROOT}/ballistica/math/vector2f.h
  ${BA_SRC_ROOT}/ballistica/math/vector3f.cc
  ${BA_SRC_ROOT}/ballistica/math/vector3f.h
  ${BA_SRC_ROOT}/ballistica/math/vector4f.h
  ${BA_SRC_ROOT}/ballistica/media/component/collide_model.cc
  ${BA_SRC_ROOT}/ballistica/media/component/collide_model.h
  ${BA_SRC_ROOT}/ballistica/media/component/cube_map_texture.cc
  ${BA_SRC_ROOT}/ballistica/media/component/cube_map_texture.h
  ${BA_SRC_ROOT}/ballistica/media/component/data.cc
  ${BA_SRC_ROOT}/ballistica/media/component/data.h
  ${BA_SRC_ROOT}/ballistica/media/component/media_component.cc
  ${BA_SRC_ROOT}/ballistica/media/component/media_component.h
  ${BA_SRC_ROOT}/ballistica/media/component/model.cc
  ${BA_SRC_ROOT}/ballistica/media/component/model.h
  ${BA_SRC_ROOT}/ballistica/media/component/sound.cc
  ${BA_SRC_ROOT}/ballistica/media/component/sound.h
  ${BA_SRC_ROOT}/ballistica/media/component/texture.cc
  ${BA_SRC_ROOT}/ballistica/media/component/texture.h
  ${BA_SRC_ROOT}/ballistica/media/data/collide_model_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/collide_model_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/data_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/data_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/media_component_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/media_component_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/model_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/model_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/model_renderer_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/sound_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/sound_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/texture_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/texture_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/texture_preload_data.cc
  ${BA_SRC_ROOT}/ballistica/media/data/texture_preload_data.h
  ${BA_SRC_ROOT}/ballistica/media/data/texture_renderer_data.h
  ${BA_SRC_ROOT}/ballistica/media/media.cc
  ${BA_SRC_ROOT}/ballistica/media/media.h
  ${BA_SRC_ROOT}/ballistica/media/media_server.cc
  ${BA_SRC_ROOT}/ballistica/media/media_server.h
  ${BA_SRC_ROOT}/ballistica/networking/network_reader.cc
  ${BA_SRC_ROOT}/ballistica/networking/network_reader.h
  ${BA_SRC_ROOT}/ballistica/networking/network_write_module.cc
  ${BA_SRC_ROOT}/ballistica/networking/network_write_module.h
  ${BA_SRC_ROOT}/ballistica/networking/networking.cc
  ${BA_SRC_ROOT}/ballistica/networking/networking.h
  ${BA_SRC_ROOT}/ballistica/networking/networking_sys.h
  ${BA_SRC_ROOT}/ballistica/networking/sockaddr.cc
  ${BA_SRC_ROOT}/ballistica/networking/sockaddr.h
  ${BA_SRC_ROOT}/ballistica/networking/telnet_server.cc
  ${BA_SRC_ROOT}/ballistica/networking/telnet_server.h
  ${BA_SRC_ROOT}/ballistica/platform/apple/platform_apple.h
  ${BA_SRC_ROOT}/ballistica/platform/linux/platform_linux.cc
  ${BA_SRC_ROOT}/ballistica/platform/linux/platform_linux.h
  ${BA_SRC_ROOT}/ballistica/platform/min_sdl.h
  ${BA_SRC_ROOT}/ballistica/platform/platform.cc
  ${BA_SRC_ROOT}/ballistica/platform/platform.h
  ${BA_SRC_ROOT}/ballistica/platform/sdl/sdl_app.cc
  ${BA_SRC_ROOT}/ballistica/platform/sdl/sdl_app.h
  ${BA_SRC_ROOT}/ballistica/platform/windows/platform_windows.cc
  ${BA_SRC_ROOT}/ballistica/platform/windows/platform_windows.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_activity_data.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_activity_data.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_collide_model.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_collide_model.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_context.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_context.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_context_call.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_context_call.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_data.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_data.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_input_device.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_input_device.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_material.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_material.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_model.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_model.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_node.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_node.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_session_data.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_session_data.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_session_player.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_session_player.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_sound.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_sound.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_texture.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_texture.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_timer.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_timer.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_vec3.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_vec3.h
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_widget.cc
  ${BA_SRC_ROOT}/ballistica/python/class/python_class_widget.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_app.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_app.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_gameplay.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_gameplay.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_graphics.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_graphics.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_input.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_input.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_media.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_media.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_networking.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_networking.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_system.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_system.h
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_ui.cc
  ${BA_SRC_ROOT}/ballistica/python/methods/python_methods_ui.h
  ${BA_SRC_ROOT}/ballistica/python/python.cc
  ${BA_SRC_ROOT}/ballistica/python/python.h
  ${BA_SRC_ROOT}/ballistica/python/python_command.cc
  ${BA_SRC_ROOT}/ballistica/python/python_command.h
  ${BA_SRC_ROOT}/ballistica/python/python_context_call.cc
  ${BA_SRC_ROOT}/ballistica/python/python_context_call.h
  ${BA_SRC_ROOT}/ballistica/python/python_context_call_runnable.h
  ${BA_SRC_ROOT}/ballistica/python/python_ref.cc
  ${BA_SRC_ROOT}/ballistica/python/python_ref.h
  ${BA_SRC_ROOT}/ballistica/python/python_sys.h
  ${BA_SRC_ROOT}/ballistica/scene/node/anim_curve_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/anim_curve_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/bomb_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/bomb_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/combine_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/combine_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/explosion_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/explosion_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/flag_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/flag_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/flash_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/flash_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/globals_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/globals_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/image_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/image_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/light_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/light_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/locator_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/locator_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/math_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/math_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/node_attribute.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/node_attribute.h
  ${BA_SRC_ROOT}/ballistica/scene/node/node_attribute_connection.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/node_attribute_connection.h
  ${BA_SRC_ROOT}/ballistica/scene/node/node_type.h
  ${BA_SRC_ROOT}/ballistica/scene/node/null_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/null_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/player_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/player_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/prop_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/prop_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/region_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/region_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/scorch_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/scorch_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/session_globals_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/session_globals_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/shield_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/shield_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/sound_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/sound_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/spaz_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/spaz_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/terrain_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/terrain_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/text_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/text_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/texture_sequence_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/texture_sequence_node.h
  ${BA_SRC_ROOT}/ballistica/scene/node/time_display_node.cc
  ${BA_SRC_ROOT}/ballistica/scene/node/time_display_node.h
  ${BA_SRC_ROOT}/ballistica/scene/scene.cc
  ${BA_SRC_ROOT}/ballistica/scene/scene.h
  ${BA_SRC_ROOT}/ballistica/ui/console.cc
  ${BA_SRC_ROOT}/ballistica/ui/console.h
  ${BA_SRC_ROOT}/ballistica/ui/root_ui.cc
  ${BA_SRC_ROOT}/ballistica/ui/root_ui.h
  ${BA_SRC_ROOT}/ballistica/ui/ui.cc
  ${BA_SRC_ROOT}/ballistica/ui/ui.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/button_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/button_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/check_box_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/check_box_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/column_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/column_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/container_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/container_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/h_scroll_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/h_scroll_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/image_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/image_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/root_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/root_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/row_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/row_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/scroll_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/scroll_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/stack_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/stack_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/text_widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/text_widget.h
  ${BA_SRC_ROOT}/ballistica/ui/widget/widget.cc
  ${BA_SRC_ROOT}/ballistica/ui/widget/widget.h
  # AUTOGENERATED_PUBLIC_END
  )

target_include_directories(ballisticacore PRIVATE
  ${Python_INCLUDE_DIRS}
  ${BA_SRC_ROOT}/external/open_dynamics_engine-ef
  ${EXTRA_INCLUDE_DIRS}
  )

target_link_libraries(ballisticacore PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/prefablib/libballisticacore_internal.a ode pthread ${Python_LIBRARIES}
  ${SDL2_LIBRARIES} ${EXTRA_LIBRARIES} dl)

# Hack for building on rpi (might be due to my manually built Python 3.8)
# Hopefully can remove later...
if(EXISTS "/home/pi")
target_link_libraries(ballisticacore PRIVATE dl util)
endif()
