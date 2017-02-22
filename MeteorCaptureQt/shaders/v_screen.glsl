#version 330

layout(location = 0) in vec3 position;  // Vertex position
layout(location = 1) in vec2 texCoords; // Texture coordinates

// Pass out the texture coordinates of the vertex
out vec2 texCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    texCoord   = texCoords;
}
