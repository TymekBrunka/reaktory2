#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;

// Output fragment color
out vec4 finalColor;

void main()
{
    finalColor = texture(environmentMap, vec3(fragPosition.x, fragPosition.y * -1, fragPosition.z));
}