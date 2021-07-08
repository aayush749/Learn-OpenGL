#version 430
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;

out vec4 varyingColor;

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
	//vec3 P = (mv_matrix * mat4(vertPos, 1.0f)).xyz;
	//
	//vec3 L = normalize(light.position - P);
	//vec3 N = normalize((norm_matrix * vec4(vertNormal, 1.0f)).xyz);
	//vec3 V = normalize(-P);

	//vec3 R = reflect(-L, N);
	////Calculate cosTheta and cosPhi
	//float cosTheta = dot(L, N);
	//float cosPhi = dot(V, R);

	////Calculate ADS lighting
	//vec3 ambient = (globalAmbient * material.ambient + globalAmbient * light.ambient).xyz;
	//vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(cosTheta, 0.0f);
	//vec3 specular = light.specular.xyz * material.specular.xyz * pow(max(cosPhi, 0.0f), material.shininess);

	//varyingColor = vec4((ambient + diffuse + specular), 1.0f);
	//gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0f);

	// convert vertex position to view space,
// convert normal to view space, and
// calculate view space light vector (from vertex to light)
	vec4 P = mv_matrix * vec4(vertPos, 1.0);
	vec3 N = normalize((norm_matrix * vec4(vertNormal, 1.0)).xyz);
	vec3 L = normalize(light.position - P.xyz);
	// view vector is equivalent to the negative of view space vertex position
	vec3 V = normalize(-P.xyz);
	// R is reflection of -L with respect to surface normal N
	vec3 R = reflect(-L, N);
	// ambient, diffuse, and specular contributions
	vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;
	vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N, L), 0.0);
	vec3 specular =
		material.specular.xyz * light.specular.xyz * pow(max(dot(R, V), 0.0f), material.shininess);
	// send the color output to the fragment shader
	varyingColor = vec4((ambient + diffuse + specular), 1.0);
	// send the position to the fragment shader, as before
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0);
}