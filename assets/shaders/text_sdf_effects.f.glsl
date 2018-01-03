
uniform sampler2D sdf;
uniform float time;

varying vec2 interpolatedTexCoord;
varying vec4 interpolatedColor;

const float glyphEdge = 0.5;

//
// Some effects. Enable/disable by commenting out the defines.
//
#define SUPERSAMPLE

#define OUTLINE
const float outlineEdgeWidth = 0.04;
const vec4 outlineColor = vec4(0.0, 0.0, 0.0, 1.0);

#define SHADOW
const float shadowBlur = 0.3;
const vec4 shadowColor = vec4(0.0, 0.0, 0.0, 0.5);

#define GROW_ANIMATION
//#define GLOW_ANIMATION


// fwidth() is not supported by default on OpenGL ES
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

  #if defined(GROW_ANIMATION)
    outerEdge -= (sin(time * 3.3) + 0.8) * 0.1;
  #endif


  #if defined(SUPERSAMPLE)
    vec2 uv = interpolatedTexCoord.xy;
    float alpha = contour(dist, outerEdge, width);

    float dscale = 0.354; // half of 1/sqrt2; you can play with this
    vec2 duv = dscale * (dFdx(uv) + dFdy(uv));
    vec4 box = vec4(uv-duv, uv+duv);

    float asum = getSample(box.xy, outerEdge, width)
               + getSample(box.zw, outerEdge, width)
               + getSample(box.xw, outerEdge, width)
               + getSample(box.zy, outerEdge, width);

    // weighted average, with 4 extra points having 0.5 weight each,
    // so 1 + 0.5*4 = 3 is the divisor
    alpha = (alpha + 0.5 * asum) / 3.0;

  #else
    //float alpha = clamp(smoothstep(outerEdge - width, outerEdge + width, dist), 0.0, 1.0);
    float alpha = contour(dist, outerEdge, width);
  #endif


  // Basic simple SDF text without effects. Normal blending.
  gl_FragColor = vec4(textColor.rgb, textColor.a * alpha);

  #if defined(OUTLINE)
    outerEdge = outerEdge - outlineEdgeWidth;
    #if defined(GROW_ANIMATION)
      outerEdge -= (sin(time * 10.3) + 0.8) * 0.05;
    #endif

    float outlineOuterAlpha = clamp(smoothstep(outerEdge - width, outerEdge + width, dist), 0.0, 1.0);
    float outlineAlpha = outlineOuterAlpha - alpha;
    gl_FragColor.rgb = mix(outlineColor.rgb, gl_FragColor.rgb, alpha);
    gl_FragColor.a = max(gl_FragColor.a, outlineColor.a * outlineOuterAlpha);
  #endif

  #if defined(SHADOW)
    float shadowAlpha = clamp(smoothstep(max(outerEdge - shadowBlur, 0.05), outerEdge + shadowBlur, dist), 0.0, 1.0);
    vec4 shadow = shadowColor * shadowAlpha;
    gl_FragColor = shadow * (1.0 - gl_FragColor.a) + gl_FragColor * gl_FragColor.a;

  #else
    // Premultiplied alpha output.
    gl_FragColor.rgb *= gl_FragColor.a;
  #endif

  #if defined(GLOW_ANIMATION)
    float glowIntensityAnim = (sin(time * 4.3) + 2.0) * 0.25;
    float glowArea = clamp(smoothstep(glyphEdge - 0.25, glyphEdge + 0.0, dist), 0.0, 1.0);
    vec4 glow = vec4(vec3(glowIntensityAnim), 0.0) * glowArea;
    gl_FragColor = glow * (1.0 - gl_FragColor.a) + gl_FragColor * gl_FragColor.a;
  #endif
}
