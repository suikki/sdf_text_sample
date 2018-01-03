uniform sampler2D sdf;

varying vec2 interpolatedTexCoord;
varying vec4 interpolatedColor;

const float glyphEdge = 0.5;

//
// Supersampling improves rendering quality of very small font sizes (less aliasing), but adds a performance hit
// as it does multiple texture lookups.
//
#define SUPERSAMPLE

// fwidth() is not supported by default on OpenGL ES. Enable it.
#if defined(GL_OES_standard_derivatives)
  #extension GL_OES_standard_derivatives : enable
#endif


float contour(float dist, float edge, float width) {
  return clamp(smoothstep(edge - width, edge + width, dist), 0.0, 1.0);
}

float getSample(vec2 texCoords, float edge, float width) {
  return contour(texture2D(sdf, texCoords).a, edge, width);
}

void main() {
  vec4 tex = texture2D(sdf, interpolatedTexCoord);
  float dist  = tex.a;
  float width = fwidth(dist);
  vec4 textColor = clamp(interpolatedColor, 0.0, 1.0);
  float outerEdge = glyphEdge;

  #if defined(SUPERSAMPLE)
    float alpha = contour(dist, outerEdge, width);

    float dscale = 0.354; // half of 1/sqrt2; you can play with this
    vec2 uv = interpolatedTexCoord.xy;
    vec2 duv = dscale * (dFdx(uv) + dFdy(uv));
    vec4 box = vec4(uv - duv, uv + duv);

    float asum = getSample(box.xy, outerEdge, width)
               + getSample(box.zw, outerEdge, width)
               + getSample(box.xw, outerEdge, width)
               + getSample(box.zy, outerEdge, width);

    // weighted average, with 4 extra points having 0.5 weight each,
    // so 1 + 0.5*4 = 3 is the divisor
    alpha = (alpha + 0.5 * asum) / 3.0;

  #else
    // No supersampling.
    float alpha = contour(dist, outerEdge, width);
  #endif

  gl_FragColor = vec4(textColor.rgb, textColor.a * alpha);

  // Premultiplied alpha output.
  gl_FragColor.rgb *= gl_FragColor.a;
}
