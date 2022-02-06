#include "TriangleRenderer.h"

TriangleRenderer::~TriangleRenderer()
{
	if (initialised)
	{
		glDeleteBuffers(1, &positionBufferID);
		glDeleteBuffers(1, &colourBufferID);
	}
}

void TriangleRenderer::Initialize()
{
	glGenBuffers(1, &positionBufferID);
	glGenBuffers(1, &colourBufferID);
	initialised = true;
}

void TriangleRenderer::QueueBox(Vector2 min, Vector2 max, Vector3 colour)
{
	QueueTriangle(min, Vector2(min.x, max.y), max, colour);
	QueueTriangle(min, max, Vector2(max.x, min.y), colour);
}

void TriangleRenderer::QueueTriangle(Vector2 a, Vector2 b, Vector2 c, Vector3 colour)
{
	positions.push_back(a);
	positions.push_back(b);
	positions.push_back(c);
	colours.push_back(colour);
	colours.push_back(colour);
	colours.push_back(colour);
}

void TriangleRenderer::UpdateFrame()
{
	if (positions.size() != colours.size()) return;

	if (positions.size() > 0)
	{
		Compile();
		Draw();
		Clear();
	}
}

void TriangleRenderer::Clear()
{
	positions.clear();
	colours.clear();
}

void TriangleRenderer::Compile()
{
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, colourBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colours.size(), colours.data(), GL_DYNAMIC_DRAW);
}

void TriangleRenderer::Draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferID);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

	glBindBuffer(GL_ARRAY_BUFFER, colourBufferID);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_TRIANGLES, 0, positions.size());
}
