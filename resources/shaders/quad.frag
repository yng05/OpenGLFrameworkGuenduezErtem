#version 150

uniform sampler2D ColorTex;
uniform vec2 Resolution;

uniform bool GreyscaleActive;
uniform bool FlipHorizontalActive;
uniform bool FlipVerticalActive;
uniform bool GaussianSmoothActive;

in vec2 pass_TexCoord;
out vec4 out_Color;



const vec3 LUMINANCE_SCALING_FACTORS = vec3(0.2126f, 0.7152f, 0.0722f);
vec4 luminance_preserving_greyscale (vec4 cl)
{
    return vec4(vec3(dot(LUMINANCE_SCALING_FACTORS, cl.rgb)), cl.a);
}

vec2 flip_horizontal (vec2 coords)
{
    return vec2(1.0f-coords.x, coords.y);
}

vec2 flip_vertical (vec2 coords)
{
    return vec2(coords.x, 1.0f-coords.y);
}

vec4 gaussian_smooth (vec2 texCoords)
{
    vec2 inv_resolution = vec2(1.0) / Resolution;
    vec4 sum = vec4(0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float d = abs(x) + abs(y);
            float factor = 0.25f * pow(2.0f, -d);

            vec2 offset = vec2(float(x), float(y));
            vec2 texCoord = texCoords + offset * inv_resolution;
            vec4 cl = texture(ColorTex, texCoord);

            sum += factor * cl;
        }
    }

    return sum;
}

void main(void)
{
    vec2 texCoords = pass_TexCoord;

    if (FlipVerticalActive)
    {
        texCoords = flip_vertical(texCoords);
    }

    if (FlipHorizontalActive)
    {
        texCoords = flip_horizontal(texCoords);
    }

    vec4 color = vec4(0.0f);
    if (GaussianSmoothActive)
    {
        color = gaussian_smooth(texCoords);
    } 
    else 
    {
        color = texture(ColorTex, texCoords);
    }

    if (GreyscaleActive)
    {
        color = luminance_preserving_greyscale(color);
    }

    out_Color = color;
}