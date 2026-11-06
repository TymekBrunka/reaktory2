#version 330
uniform mat4 projection;
uniform mat4 view;
uniform mat4 pos;
uniform int length;

uniform vec2 psize;
uniform vec2 bpos[6];
uniform vec2 tpos[6];

#define ELS vec2(10 * 3, 16 * 3)

in int charIdx;
out vec2 texPos;

void main() {
  int v = int(gl_VertexID) % 6;
  float offset = floor(int(gl_VertexID) / 6) - ((length - 1) / 2.0);

  //vec4 lpos = view * pos * vec4(vec2((offset + bpos[v].x) * ELS.x, bpos[v].y * ELS.y), 0.0, 1.0);
  vec4 lpos = view * pos * vec4(0.0, 0.0, 0.0, 1.0);
  lpos.xyz /= lpos.w;
  lpos.w = 1.0;

  //float d = sqrt(1 + abs(lpos.z)) - 1;
  float d = sqrt(1 + (1.2 * abs(lpos.z))) - 1;

  //lpos += vec4(vec2((offset + bpos[v].x) * ELS.x, bpos[v].y * ELS.y) * (1.0 / d), 0.0, 0.0);

  gl_Position = projection * lpos;
  gl_Position.xyz /= gl_Position.w;
  gl_Position.w = 1.0;
  gl_Position += vec4(vec2((offset + bpos[v].x) * ELS.x, bpos[v].y * ELS.y) * psize * (1.0 / d), 0.0, 0.0);
  texPos = vec2(((charIdx % 16) * 0.0625) + tpos[v].x, (1.0 - (floor(charIdx / 16) * 0.0625)) + tpos[v].y);
}