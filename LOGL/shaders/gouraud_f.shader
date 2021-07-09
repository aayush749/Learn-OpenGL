#version 430

in vec4 varyingColor;
out vec4 fragColor;

in vec2 tc;

struct PositionalLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};

struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

layout(binding = 0) uniform sampler2D samp;

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

void main(void)
{
	vec4 tex_color = texture(samp, tc);
	fragColor = tex_color * varyingColor;
}