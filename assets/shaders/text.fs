#version 330

uniform sampler2D text;
uniform vec3 ID;
uniform vec4 colors[8];

flat in int coloridx;
in vec2 texPos;
layout (location = 0) out vec4 fragment;
layout (location = 1) out vec4 colorBuffer;

vec4 Blend(in vec4 fragColor, in vec4 textureColor)
{
    vec4 blend;
    //blend = fragColor*(1.0-alpha) + fragColor*textureColor*alpha;
    blend = fragColor*(1.0-textureColor.w) + textureColor;
    return blend;
}

vec4 v4m(in vec4 x, in vec4 y) {
  return vec4(x.x * y.x, x.y * y.y, x.z * y.z, x.w * y.w);
}

void main() {
  fragment = Blend(vec4(0.1, 0.1, 0.1, 1.0), v4m(texture(text, texPos), colors[coloridx]));
  //fragment = vec4(0.0, 0.0, 1.0, 1.0);
  colorBuffer = vec4(ID, 1.0);
}
