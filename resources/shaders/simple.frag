#version 150


in vec4 pass_Normal;
in vec3 pass_Color;
in vec3 normalInt;
in vec3 vertPos;

out vec4 out_Color;

const vec3 lightPos = vec3(0.0,0.0,0.0);
// moved const color into function
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;
const float screenGamma = 2.2;

void main(void)
{
	vec3 ambientColor = pass_Color; //vec3(0.1, 0.0, 0.0);
    vec3 diffuseColor = pass_Color; //vec3(0.5, 0.0, 0.0);

	vec3 normal = normalize(normalInt);
	vec3 lightDir = normalize(lightPos -vertPos);

	float lambertian = max(dot(lightDir,normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0)
	{
		vec3 viewDirection = normalize(-vertPos);

		vec3 halfDir = normalize(lightDir + viewDirection);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, shininess);	
	}

	vec3 colorLinear = ambientColor* vec3(0.1f) + lambertian * diffuseColor + specular * specColor;

    out_Color = vec4(colorLinear, 1.0);
}