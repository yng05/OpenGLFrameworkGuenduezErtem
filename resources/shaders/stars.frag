#version 150


in  vec3 pass_Color;
out vec4 out_Color;

void main(void)
{   // (x,y,z,alpha)
    out_Color = vec4(abs(pass_Color.xyz), 1.0f);
}
