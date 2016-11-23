#version 150

in  vec3 pass_Normal;
in 	vec2 pass_TexCoord;

out vec4 out_Color;

uniform sampler2D Texture;

void main() {
  out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
  vec3 TextureColor = (texture(Texture, pass_TexCoord)).rgb;

  out_Color = vec4(TextureColor, 1.0);
}