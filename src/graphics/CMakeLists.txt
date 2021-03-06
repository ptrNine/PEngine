git_submodule_build(glfw)
git_submodule_build(assimp)
git_submodule_copy_files(stb INCLUDES stb_image.h stb_image_write.h stb_image_resize.h)

find_package(assimp REQUIRED)

add_library(
    grx_asm
    STATIC
    algorithms/asm/x86_64_avx_frustum_culling.asm
    algorithms/asm/x86_64_sse_frustum_culling.asm
)

set(GRX_SOURCES
        algorithms/grx_frustum_culling.cpp
        grx_utils.cpp
        grx_context.cpp
        grx_shader.cpp
        grx_shader_tech.cpp
        grx_window.cpp
        grx_render_target.cpp
        grx_vbo_tuple.cpp
        grx_ssbo.cpp
        grx_color_map.cpp
        grx_texture.cpp
        grx_texture_path_set.cpp
        grx_cpu_mesh_group.cpp
        grx_mesh_gen.cpp
        grx_camera.cpp
        grx_camera_manipulator_fly.cpp
        grx_cascaded_shadow_mapping_tech.cpp
        grx_g_buffer.cpp
        grx_deferred_renderer_light.cpp
        grx_skeleton.cpp
        grx_animation.cpp
        grx_skybox.cpp
)

set(GRX_HEADERS
        algorithms/grx_frustum_culling_asm.hpp
        algorithms/grx_frustum_culling.hpp
        grx_utils.hpp
        grx_context.hpp
        grx_shader.hpp
        grx_shader_mgr.hpp
        grx_window.hpp
        grx_render_target.hpp
        grx_vbo_tuple.hpp
        grx_ssbo.hpp
        grx_postprocess_mgr.hpp
        grx_texture.hpp
        grx_texture_mgr.hpp
        grx_camera.hpp
        grx_camera_manipulator.hpp
        grx_camera_manipulator_fly.hpp
        grx_cascaded_shadow_mapping_tech.hpp
)

add_library(pe_graphics SHARED ${GRX_SOURCES})
target_link_libraries(pe_graphics GL GLEW glfw3 assimp pe_input grx_asm ${PE_LIBS})
