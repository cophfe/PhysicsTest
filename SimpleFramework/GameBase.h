#pragma once
#include "Graphics.h"
#include "Maths.h"
#include "LineRenderer.h"
#include "TriangleRenderer.h"
#include "TextRenderer.h"
#include "ShaderProgram.h"



class GameBase
{
	float cameraHeight = 10.0f;
	glm::vec2 cameraCentre = { 0.0f, 0.0f };

	float aspectRatio = 16.0f / 9.0f;

	LineRenderer grid;

	const float cameraSpeed = 0.8f;


	glm::mat4 GetCameraTransform() const;


protected:
	const int gridLimits = 10;
	ShaderProgram simpleShader;
	ShaderProgram textShader;
	TextRenderer textRenderer; //updated every time it is modified, does not reset text ever
	TriangleRenderer triangleRenderer;
	glm::vec2 cursorPos = { 0.0f,0.0f };
	glm::vec2 screenCursorPos = { 0.0f,0.0f };
	bool leftButtonDown = false;
	bool rightButtonDown = false;
	float time = 0.0f;
	LineRenderer lines;
	const float deltaTime = 0.0166667f;	//Delta time should be constant for physics simulations.
	GLFWwindow* window = nullptr;

	Matrix4x4 textProjectionMatrix;

	Vector2Int oldWindowSize;
	Vector2Int windowSize;
public:
	GameBase();
	~GameBase();

	GameBase(const GameBase& other) = delete;
	GameBase& operator=(const GameBase& other) = delete;

	void CapFPS(bool capped);
	void Update();
	void Render();

	bool IsRunning() const;

	//1 for left, 2 for right, 3 for middle. Other button codes will get sent for other buttons but I'm not sure how they'd map to a particular mouse.
	virtual void OnMouseClick(int mouseButton);
	virtual void OnMouseRelease(int mouseButton);
	virtual void OnWindowResize(int width, int height);
	virtual void OnKeyPressed(int key);
	virtual void OnKeyReleased(int key);
	
	void Zoom(float zoomFactor);
};
