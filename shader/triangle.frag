#version 450

layout (location = 0) in vec3 inFragColor;
layout (location = 1) in vec2 texCoord;

layout (location = 0) out vec4 outFragColor;
layout (set = 2, binding = 0) uniform sampler2D tex1;

layout (set = 0, binding = 1) uniform SceneData{
    vec4 fogColor; // w is for exponent
    vec4 fogDistances; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 sunlightDirection; //w for sun power
    vec4 sunlightColor; 
} sceneData;

void main(){
    // outFragColor = vec4(inFragColor + sceneData.ambientColor.xyz, 1.0f);
    // outFragColor = vec4(texCoord.x, texCoord.y, 0.5f, 1.0f);
    outFragColor = vec4(texture(tex1, texCoord), 1.0f);
}
