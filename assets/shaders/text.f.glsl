uniform sampler2D diffuse;

varying vec2 interpolatedTexCoord;
varying vec4 interpolatedColor;

void main() {
  float alpha = texture2D(diffuse, interpolatedTexCoord).a;
  vec4 textColor = clamp(interpolatedColor, 0.0, 1.0);
  gl_FragColor = vec4(textColor.rgb * textColor.a, textColor.a) * alpha; // Premultiplied alpha.
}
