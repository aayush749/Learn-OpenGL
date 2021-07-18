#version 430
in vec3 varyingVertPos;
in vec3 varyingLightDir;
in vec3 varyingNormal;
in vec3 varyingHalfVector;
in vec2 tc; //for texture coordinates
out vec4 fragColor;

layout(binding = 1) uniform sampler2D samp;

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

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

void main(void)
{
	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-varyingVertPos);

	////compute light reflection vector with respect to varying normal vector
	//vec3 R = reflect(-L, N);

	//get the angle between light and surface normal
	float cosTheta = dot(L, N);

	//get the angle between view vector and reflected light
	float cosPhi = dot(varyingHalfVector, N);

	//Calculate ADS contribution (per pixel) and add the contributions to get the output color
	vec3 ambient = (globalAmbient * material.ambient + light.ambient * material.ambient).xyz;
	vec3 diffuse = (light.diffuse.xyz * material.diffuse.xyz * max(cosTheta, 0.0f));
	vec3 specular = (light.specular.xyz * material.specular.xyz * pow(max(cosPhi, 0.0f), material.shininess));


	//sample the texture to get its color contribution
	vec4 tex_col = texture(samp, tc);	//sample at texCoord tc => gives you the color value as a vec4
	fragColor = tex_col * vec4(ambient + diffuse, 1.0f) + vec4(specular, 1.0f);
}