#version 150


in vec4 pass_Normal;
in vec3 pass_Color;
in vec3 normalInt;
in vec3 vertPos;
in vec3 sunPos;
in vec2 pass_TexCoord;

out vec4 out_Color;


// moved const color into function
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;

uniform sampler2D Texture;

void main(void)
{
	vec3 ambientColor = pass_Color; 
    vec3 diffuseColor = pass_Color;

	vec3 normal = normalize(normalInt);
	vec3 lightDir = normalize(sunPos -vertPos);

	float lambertian = max(dot(lightDir,normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0)
	{
		vec3 viewDirection = normalize(-vertPos);

		vec3 halfDir = normalize(lightDir + viewDirection);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, shininess);	
	}

	vec3 TextureColor = (texture(Texture, pass_TexCoord)).rgb;

	vec3 colorLinear = vec3(0.1f) * TextureColor + lambertian * TextureColor + specular * specColor;

    out_Color = vec4(colorLinear, 1.0);
}