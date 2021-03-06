#pragma once
#include <thread>
#include <core/helper_macros.hpp>

namespace grx {
    // Holds global OpenGL state
    class grx_context {
        SINGLETON_IMPL(grx_context);

    public:
        void setup_debug_callback();

        void bind_default_framebuffer();
        void set_wireframe_enabled(bool value);
        void set_depth_test_enabled(bool value);
        void set_depth_mask_enabled(bool value);
        void set_cull_face_enabled(bool value);


        void append_drawed_vertices(size_t count) {
            _drawed_vertices += count;
        }

        void update_states() {
            _drawed_vertices = 0;
        }

    private:
        grx_context();
        ~grx_context();

    private:
        bool _is_wireframe_enabled  = false;
        bool _is_depth_test_enabled = true;
        bool _is_cull_face_enabled  = true;
        bool _is_depth_mask_enabled = true;


        size_t _drawed_vertices = 0;

    public:
        DECLARE_VAL_GET(is_wireframe_enabled)
        DECLARE_VAL_GET(is_depth_test_enabled)
        DECLARE_VAL_GET(is_cull_face_enabled)
        DECLARE_VAL_GET(drawed_vertices)
    };

    inline grx_context& grx_ctx() {
        return grx_context::instance();
    }
} // namespace grx
