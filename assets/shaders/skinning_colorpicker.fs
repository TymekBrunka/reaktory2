#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec4 ID;

void main()
{
    finalColor = ID;
}
