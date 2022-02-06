#pragma once
#include "Graphics.h"
#include "Maths.h"
#include <vector>
static const Vector3 defaultTriangleColour = { 1.0f,1.0f,1.0f };

//literally just the line renderer but with triangles
//uses the same shader even
class TriangleRenderer
{
public:
	void Initialize();

	void QueueBox(Vector2 min, Vector2 max, Vector3 colour = defaultTriangleColour);
	void QueueTriangle(Vector2 a, Vector2 b, Vector2 c, Vector3 colour  = defaultTriangleColour);

	void UpdateFrame();

	void Clear();
	void Compile();
	void Draw();

	TriangleRenderer() = default;
	~TriangleRenderer();
	TriangleRenderer(const TriangleRenderer&) = delete;
	TriangleRenderer(const TriangleRenderer&&) = delete;
	const TriangleRenderer& operator=(const TriangleRenderer&) = delete;
	const TriangleRenderer& operator=(TriangleRenderer&&) = delete;
private:
	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;

	bool initialised = false;
	GLuint positionBufferID;
	GLuint colourBufferID;
};

