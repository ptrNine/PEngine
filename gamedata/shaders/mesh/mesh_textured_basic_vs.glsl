uniform mat4 MVP;

in  vec3 position_ms;
in  vec2 in_uv;
out vec2 uv;

void main() {
    gl_Position = MVP * vec4(position_ms, 1.0);
    uv          = in_uv;
}

