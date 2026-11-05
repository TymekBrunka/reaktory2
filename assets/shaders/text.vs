#version 330
uniform mat4 projection;
uniform mat4 view;
uniform mat4 pos;
uniform int length;

uniform vec2 bpos[6];
uniform vec2 tpos[6];

#define ELS vec2(10, 16)

in int charIdx;
out vec2 texPos;

void main() {
  vec4 lpos = view * pos * vec4(1.0);
  lpos.xyz /= lpos.w;
  lpos.w = 1.0;

  float d = sqrt(lpos.z);
  int v = int(gl_VertexID) % 6;
  float offset = (int(gl_VertexID) / 6) - (length / 2);

  gl_Position = projection * lpos;
  gl_Position = gl_Position + vec4(vec2((offset + bpos[v].x) * ELS.x, bpos[v].y * ELS.y), 0.0, 0.0);
  //gl_Position = vec4(vec2((offset + bpos[v].x) * ELS.x, bpos[v].y * ELS.y), 0.0, 0.0);
  texPos = vec2(mod(charIdx, 16) * tpos[v].x, floor(charIdx / 16) * tpos[v].y);
}