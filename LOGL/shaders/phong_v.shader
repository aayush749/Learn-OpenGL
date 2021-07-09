#version 430
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 texCoords;

out vec3 varyingVertPos;
out vec3 varyingLightDir;
out vec3 varyingNormal;
out vec2 tc;

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
layout(binding = 0) uniform sampler2D samp; //not used in vertex shader
uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix; // for transforming normals
void main(void)
{
	varyingVertPos = (mv_matrix * vec4(vertPos, 1.0f)).xyz;
	varyingLightDir = light.position - varyingVertPos;
	varyingNormal = (norm_matrix * vec4(vertNormal, 1.0f)).xyz;

	tc = texCoords;
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0);
}