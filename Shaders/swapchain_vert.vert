#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragColor = inNormal;

    if(dot(vec3(1.0f, 0.0f, 0.0f), inNormal) > 0.0f) {
        fragColor = vec3(1.0f, 0.0f, 0.0f);
    } else if(dot(vec3(0.0f, 1.0f, 0.0f), inNormal) > 0.0f){
        fragColor = vec3(0.0f, 1.0f, 0.0f);
    } else if(dot(vec3(0.0f, 0.0f, 1.0f), inNormal) > 0.0f){
        fragColor = vec3(0.0f, 0.0f, 1.0f);
    } else if(dot(vec3(0.0f, 0.0f, -1.0f), inNormal) > 0.0f){
        fragColor = vec3(0.0f, 0.0f, 1.0f);
    } else if(dot(vec3(0.0f, -1.0f, 0.0f), inNormal) > 0.0f){
        fragColor = vec3(0.0f, 1.0f, 0.0f);
    }
}