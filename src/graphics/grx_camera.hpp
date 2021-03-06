#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/constants.hpp>

#include <core/helper_macros.hpp>
#include <core/types.hpp>
#include <core/vec.hpp>

#include "grx_types.hpp"
#include "grx_camera_manipulator.hpp"
#include "grx_joint_animation.hpp"

namespace grx {
    class grx_camera {
    public:
        static constexpr float FOV_MIN    = 10.f;
        static constexpr float FOV_MAX    = 180.f;
        static constexpr float PITCH_LOCK = glm::half_pi<float>() - 0.05f;
        static constexpr float ROLL_LOCK  = glm::half_pi<float>() - 0.1f;

    public:
        static core::shared_ptr<grx_camera>
        make_shared(const core::vec3f& pos = { 0.f, 0.f, 0.f },
                    float aspect_ratio = 16.f / 9.f,
                    float vertical_fov = 73.f,
                    float z_near = 1.1f,
                    float z_far = 1536.f)
        {
            return core::shared_ptr<grx_camera>(
                    new grx_camera(pos, aspect_ratio, vertical_fov, z_near, z_far));
        }

        void look_at(const core::vec3f& pos);
        void calc_orientation();
        void calc_view_projection();
        void update(float timestep, class grx_window* window = nullptr);
        [[nodiscard]]
        auto extract_frustum() const -> grx_aabb_frustum_planes_fast;

        [[nodiscard]]
        auto extract_frustum(float z_near, float z_far, float z_shift) const -> grx_aabb_frustum_planes_fast;

        template <typename T, typename... Ts>
        void create_camera_manipulator(Ts&&... args) {
            _camera_manipulator = core::unique_ptr<T>(new T(core::forward<Ts>(args)...));
        }

        void set_camera_animations(core::shared_ptr<grx_joint_animation_holder> anim_holder) {
            _anim_holder = core::move(anim_holder);
        }

        void play_animation(const core::string& animation_name) {
            _anim_player.play_animation(animation_name);
        }

        grx_joint_animation_player& animation_player() {
            return _anim_player;
        }

    protected:
        grx_camera(const core::vec3f& pos,
                   float aspect_ratio,
                   float fov,
                   float z_near,
                   float z_far
        ): _pos(pos), _aspect_ratio(aspect_ratio), _fov(fov), _z_near(z_near), _z_far(z_far) {}

    private:
        core::vec3f _pos;
        core::vec3f _dir    = { 0.f, 0.f, 1.f };
        core::vec3f _right  = { 1.f, 0.f, 0.f };
        core::vec3f _up     = { 0.f, 1.f, 0.f };

        glm::mat4 _orientation = glm::mat4(1.f);
        glm::mat4 _view        = glm::mat4(1.f);
        glm::mat4 _projection  = glm::mat4(1.f);

        float _aspect_ratio;
        float _fov;
        float _z_near;
        float _z_far;

        vec3f _ypr = {0.f, 0.f, 0.f};
        vec3f _ypr_speed = {0.f, 0.f, 0.f};

        core::unique_ptr<grx_camera_manipulator> _camera_manipulator;
        core::shared_ptr<grx_joint_animation_holder> _anim_holder;
        grx_joint_animation_player _anim_player;

    public:
        DECLARE_GET(view)
        DECLARE_GET(projection)
        DECLARE_GET(orientation)
        DECLARE_VAL_GET(aspect_ratio)
        DECLARE_VAL_GET(z_near)
        DECLARE_VAL_GET(z_far)

        [[nodiscard]]
        float vertical_fov() const {
            return _fov;
        }

        void vertical_fov(float value) {
            _fov = value;
        }

        [[nodiscard]]
        float horizontal_fov() const {
            return _fov * _aspect_ratio;
        }

        void horizontal_fov(float value) {
            _fov = value / _aspect_ratio;
        }

        [[nodiscard]]
        core::vec3f ypr() const {
            auto res = _ypr + _anim_player.rotation();
            return {core::angle::constraint_pi(res.x()),
                    std::clamp(res.y(), -PITCH_LOCK, PITCH_LOCK),
                    std::clamp(res.z(), -ROLL_LOCK, ROLL_LOCK)};
        }

        [[nodiscard]]
        const core::vec3f& ypr_speed() const {
            return _ypr_speed;
        }

        [[nodiscard]]
        glm::mat4 view_projection() const {
            return _projection * _view;
        }

        [[nodiscard]]
        core::vec3f position() const {
            auto& anim_pos = _anim_player.position();
            return _pos + _dir * anim_pos.z() + _right * anim_pos.x() + _up * anim_pos.y();
        }

        [[nodiscard]]
        core::vec3f real_position() const {
            return _pos;
        }

        [[nodiscard]]
        glm::mat4 real_view() const {
            return _orientation * glm::translate(glm::mat4(1.f), to_glm(-real_position()));
        }

        [[nodiscard]]
        core::vec3f directory() const {
            return _dir;
        }

        [[nodiscard]]
        core::vec3f up() const {
            return _up;
        }

        [[nodiscard]]
        core::vec3f right() const {
            return _right;
        }

        void set_position(const core::vec3f& value) {
            _pos = value;
        }

        void set_fov(float value) {
            _fov = std::clamp(value, FOV_MIN, FOV_MAX);
        }

        void set_aspect_ratio(float value) {
            _aspect_ratio = value;
        }

        void set_z_near(float value) {
            _z_near = value;
        }

        void set_z_far(float value) {
            _z_far = value;
        }
    };
} // namespace grx
