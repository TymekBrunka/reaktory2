#version 330
uniform mat4 projection;
uniform mat4 view;
uniform mat4 pos;
uniform int length;
uniform int height;

uniform vec2 psize;
uniform vec2 bpos[6];
uniform vec2 tpos[6];

#define ELS vec2(10 * 0.01, 16 * 0.01)

in int charIdx;
in int colorIdx;
in int lineNr;
in int columnNr;

out vec2 texPos;
flat out int coloridx;

void main() {
  int v = int(gl_VertexID) % 6;
  vec2 offset = vec2(columnNr - ((length - 1) / 2.0), -lineNr + ((height - 1) / 2.0));

  vec4 lpos = view * pos * vec4(0.0, 0.0, 0.0, 1.0);
  lpos.xyz /= lpos.w;
  lpos.w = 1.0;

  // float d = sqrt(1 + (1.2 * abs(lpos.z))) - 1;
  float d = 0.65 + (0.35 * (sqrt(1 + (1.2 * abs(lpos.z))) - 1));

  gl_Position = lpos;
  gl_Position.xyz /= gl_Position.w;
  gl_Position.w = 1.0;
  gl_Position += vec4(vec2(
        (offset.x + bpos[v].x) * ELS.x,
        (offset.y + bpos[v].y) * ELS.y)
    // * psize * (1.0 / d), 0.0, 0.0);
    * 1 * d, 0.0, 0.0);
  gl_Position = projection * gl_Position;
  texPos = vec2(((charIdx % 16) * 0.0625) + tpos[v].x, (1.0 - (floor(charIdx / 16) * 0.0625)) + tpos[v].y);
  coloridx = colorIdx;
}
