#pragma once

#include <vector>
#include <glm/glm.hpp>

class Sphere
{
public:
	Sphere();
	Sphere(int prec);
	inline int GetNumVertices() const { return numVertices; }
	inline int GetNumIndices() const { return numIndices; }
	const std::vector<glm::vec3>& GetVertices() const;
	const std::vector<int>& GetIndices() const;
	const std::vector<glm::vec2>& GetTexCoords() const;
	const std::vector<glm::vec3>& GetNormals() const;
	
	static float toRadians(float degrees);

private:
	int numVertices;
	int numIndices;
	std::vector<int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

private:
	void Init(int prec);
};

