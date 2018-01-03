uniform mat4 modelView;
uniform mat4 projection;

attribute vec4 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

varying vec2 interpolatedTexCoord;
varying vec4 interpolatedColor;

void main() {
  interpolatedColor = vertexColor;
  interpolatedTexCoord = vertexTexCoord;
  gl_Position = projection * modelView * vertexPosition;
}
