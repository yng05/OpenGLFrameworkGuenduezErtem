#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_Texcoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 ColorVec;


out vec4 pass_Normal;
out vec3 vertPos;
out vec3 normalInt;
out vec3 pass_Color;
out vec3 sunPos;
out vec2 pass_TexCoord;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);

	vec4 vertPos4 = ModelMatrix * vec4(in_Position, 1.0);
    vertPos = vec3((ViewMatrix * ModelMatrix) * vec4(in_Position,1.0));

    sunPos = vec3((ViewMatrix) * vec4(vec3(0.0,0.0,0.0), 1.0f));

	normalInt = vec3(NormalMatrix * vec4(in_Normal, 0.0));
	pass_Color = ColorVec;
	pass_TexCoord = in_Texcoord;
}