const int MAX_POINT_LIGHTS = 16;
const int MAX_SPOT_LIGHTS  = 16;
const int CSM_MAPS_COUNT   = 3;
const int PCF_STEPS = 3;

const int PCF_LOW   = -PCF_STEPS/2;
const int PCF_UP    = (PCF_STEPS + 1)/2;
const float PCF_DIV = PCF_STEPS * PCF_STEPS;

struct light_base_s {
    vec3  color;
    float ambient_intensity;
    float diffuse_intensity;
};

struct dir_light_s {
    light_base_s base;
    vec3        direction;
};

struct point_light_s {
    light_base_s base;
    vec3         position;
    float        attenuation_constant;
    float        attenuation_linear;
    float        attenuation_quadratic;
};

struct spot_light_s {
    point_light_s base;
    vec3          direction;
    float         cutoff;
};

uniform vec3  eye_pos_ws;
uniform float specular_power;
uniform float specular_intensity;
uniform int   dir_light_enabled;
uniform int   point_lights_count;
uniform int   spot_lights_count;

uniform dir_light_s   dir_light;
uniform point_light_s point_lights[MAX_POINT_LIGHTS];
uniform spot_light_s  spot_lights[MAX_SPOT_LIGHTS];
uniform int           point_lights_idxs[MAX_POINT_LIGHTS];
uniform int           spot_lights_idxs[MAX_SPOT_LIGHTS];

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2DShadow shadow_map[CSM_MAPS_COUNT];
uniform sampler2DShadow spot_shadow_map[MAX_SPOT_LIGHTS];
uniform float           csm_end_cs[CSM_MAPS_COUNT];

const float shadow_bias = 0.0002;
float calc_shadow_factor(sampler2DShadow shadow_map, vec4 position_ls) {
    vec3 uvc        = ((position_ls.xyz - vec3(0.0, 0.0, shadow_bias)) / position_ls.w) * 0.5 + 0.5;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    float shadow = 0.0;

    for (int i = PCF_LOW; i < PCF_UP; ++i)
        for (int j = PCF_LOW; j < PCF_UP; ++j)
            shadow += texture(shadow_map, uvc + vec3(vec2(i, j) * texel_size, 0.0));

    return shadow / PCF_DIV;
}

//float calc_spot_shadow_factor(vec4 position_ls, int spot_idx) {
//    vec3 proj_coords = position_ls.xyz / position_ls.w;
//    proj_coords = proj_coords * 0.5 + 0.5;
//
//    float depth  = proj_coords.z;
//    float bias   = 0.002;
//    float shadow = 0.0;
//    vec2  texel_size = 1.0 / textureSize(spot_lights[spot_idx].shadow_map, 0);
//
//    for (int i = PCF_LOW; i < PCF_UP; ++i) {
//        for (int j = PCF_LOW; j < PCF_UP; ++j) {
//            float pcf_depth = texture2D(spot_lights[spot_idx].shadow_map, proj_coords.xy + vec2(i, j) * texel_size).r;
//            shadow += depth - bias < pcf_depth ? 1.0 : 0.0;
//        }
//    }
//
//    shadow /= PCF_DIV;
//    return shadow;
//}


vec4 calc_light_base(light_base_s light, vec3 light_direction, vec3 normal_cs, vec3 position_ws, float shadow_factor) {
    vec4  ambient_color  = vec4(light.color, 1.0) * light.ambient_intensity;
    float diffuse_factor = dot(normal_cs, -light_direction);
    vec4  diffuse_color  = vec4(0);
    vec4  specular_color = vec4(0);

    if (diffuse_factor > 0) {
        diffuse_color = vec4(light.color, 1.0) * light.diffuse_intensity * diffuse_factor;

        vec3  view_dir    = normalize(eye_pos_ws - position_ws);
        vec3  halfway_dir = normalize(view_dir - light_direction);
        float specular_factor = pow(max(dot(normal_cs, halfway_dir), 0.0), specular_power);
        if (specular_factor > 0)
            specular_color = vec4(light.color, 1.0) * specular_intensity * specular_factor;
    }

    return ambient_color + shadow_factor * (diffuse_color + specular_color);
}

vec4 calc_dir_light(vec3 normal_cs, vec3 position_ws, float shadow_factor) {
    if (dir_light_enabled != 0)
        return calc_light_base(dir_light.base, dir_light.direction, normal_cs, position_ws, shadow_factor);
    else
        return vec4(0);
}

vec4 calc_point_light(point_light_s light, vec3 normal_cs, vec3 position_ws, float shadow_factor) {
    vec3 light_direction = position_ws - light.position;
    float _distance      = length(light_direction);
    light_direction      = normalize(light_direction);

    vec4  color          = calc_light_base(light.base, light_direction, normal_cs, position_ws, shadow_factor);
    float attenuation    = light.attenuation_constant + light.attenuation_linear * _distance +
                                                        light.attenuation_quadratic * _distance * _distance;
    return color / attenuation;
}

vec4 calc_spot_light(spot_light_s light, vec3 normal_cs, vec3 position_ws, float shadow_factor) {
    vec3  light_to_pixel = normalize(position_ws - light.base.position);
    float spot_factor    = dot(light_to_pixel, light.direction);

    if (spot_factor > light.cutoff) {
        vec4 color = calc_point_light(light.base, normal_cs, position_ws, shadow_factor);
        return color * (1.0 - (1.0 - spot_factor) * 1.0/(1.0 - light.cutoff));
    } else {
        return vec4(0);
    }
}

vec3 calc_bumped_normal(vec2 uv, vec3 normal_cs, vec3 tangent_ws, vec3 bitangent_ws) {
    vec3 normal          = normalize(normal_cs);
    vec3 tangent         = normalize(tangent_ws);
    vec3 bitangent       = normalize(bitangent_ws);
    vec3 bump_map_normal = 2.0 * texture(texture1, uv).xyz - vec3(1.0, 1.0, 1.0);
    mat3 TBN             = mat3(tangent, bitangent, normal);
    vec3 new_normal      = TBN * bump_map_normal;
    return normalize(new_normal);
}

in vec2  uv;
in vec3  position_ws;
in vec3  normal_cs;
in vec3  tangent_ws;
in vec3  bitangent_ws;
in vec4  position_ls[CSM_MAPS_COUNT];
in vec4  spot_pos_ls[MAX_SPOT_LIGHTS];
in float z_pos_cs;

out vec4 color;

void main() {
    vec3 _normal_cs = calc_bumped_normal(uv, normal_cs, tangent_ws, bitangent_ws);

    float csm_shadow_factor = 0.0;
    for (int i = 0; i < CSM_MAPS_COUNT; ++i) {
        if (z_pos_cs > csm_end_cs[i]) {
            csm_shadow_factor = calc_shadow_factor(shadow_map[i], position_ls[i]);
            break;
        }
    }
    vec4 total_light = calc_dir_light(_normal_cs, position_ws, csm_shadow_factor);

    for (int i = 0; i < point_lights_count; ++i) {
        total_light += calc_point_light(point_lights[point_lights_idxs[i]], _normal_cs, position_ws, 1.0);
    }

    for (int i = 0; i < spot_lights_count; ++i) {
        int idx = spot_lights_idxs[i];
        float shadow_factor = calc_shadow_factor(spot_shadow_map[idx], spot_pos_ls[idx]);
        total_light += calc_spot_light(spot_lights[idx], _normal_cs, position_ws, shadow_factor);
    }

    color = texture(texture0, uv) * total_light;
}

