#include "GameBase.h"
#include "GLFWCallbacks.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

glm::mat4 GameBase::GetCameraTransform() const
{
	return glm::ortho(
		-aspectRatio * cameraHeight / 2.0f + cameraCentre.x, 
		aspectRatio * cameraHeight / 2.0f + cameraCentre.x,
		-cameraHeight / 2.0f + cameraCentre.y,
		cameraHeight / 2.0f + cameraCentre.y,
		-1.0f, 1.0f);
}

GameBase::GameBase()
{
	if (!glfwInit())
	{
		return;
	}
	//Can choose resolution here.
	window = glfwCreateWindow(1280, 720, "Physics!", nullptr, nullptr);
	//glfwSetWindowPos(window, 1950, 30);
	//glfwWindowHint(GLFW_REFRESH_RATE, 400);
	
	//This is the somewhat hacky oldschool way of making callbacks work without everything having to be global. Look
	//at the way the function callbacks work to get an idea of what's going on.
	glfwSetWindowUserPointer(window, (void*)this);	

	//These functions will be able to reference the testbed object via the user pointer.
	glfwSetWindowSizeCallback(window, WindowResizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, MouseWheelCallback);
	glfwSetKeyCallback(window, KeyCallback);

	if (!window)
	{
		//If the window failed to create for some reason, abandon ship.
		glfwTerminate();
		return;
	}

	//We have to tell glfw to use the OpenGL context from the window.
	glfwMakeContextCurrent(window);

	//This is where GLAD gets set up. After this point we can use openGL functions.
	if (!gladLoadGL())
	{
		return;
	}
	
	//set to 0 for uncapped fps
	glfwSwapInterval(1);

	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	simpleShader = ShaderProgram("Simple.vsd", "Simple.fsd");
	simpleShader.UseShader();

	lines.Initialise();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	grid.Initialise();
	for (float i = (float)-gridLimits; i <= (float)gridLimits; i++)
	{
		glm::vec3 colour = (i == 0) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.3f, 0.3f, 0.3f);
		grid.DrawLineSegment({ i, -gridLimits }, { i, gridLimits }, colour);
		grid.DrawLineSegment({ -gridLimits, i }, { gridLimits, i }, colour);
	}
	grid.DrawLineSegment({ 0,0 }, { 1, 0 }, { 1, 0, 0 });
	grid.DrawLineSegment({ 0,0 }, { 0, 1 }, { 0, 1, 0 });
	grid.Compile();

	//text now
	textShader = ShaderProgram("Text.vsd", "Text.fsd");
	textRenderer.Initialise("cabin.ttf", textShader, false);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	windowSize = { width, height };

	textProjectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
	textShader.SetUniform("textProjection", textProjectionMatrix);

	triangleRenderer.Initialize();
	linesUI.Initialise();
}

GameBase::~GameBase()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

void GameBase::CapFPS(bool capped)
{
	if (capped)
	{
		glfwSwapInterval(1);
	}
	else 
	{
		glfwSwapInterval(0);
	}
}

void GameBase::Update()
{
	/*int width, height;
	glfwGetWindowSize(window, &width, &height);*/
	
	time += deltaTime;

	

	

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		cameraCentre.x -= cameraSpeed * deltaTime * cameraHeight;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		cameraCentre.x += cameraSpeed * deltaTime * cameraHeight;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		cameraCentre.y += cameraSpeed * deltaTime * cameraHeight;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		cameraCentre.y -= cameraSpeed * deltaTime * cameraHeight;
	}

	leftButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	rightButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	glm::mat4 deprojection = glm::inverse(GetCameraTransform());
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	screenCursorPos = glm::vec2(cursorX, windowSize.y - cursorY);
	cursorX = (cursorX / windowSize.x) * 2.0 - 1.0;
	cursorY = -((cursorY / windowSize.y) * 2.0 - 1.0);
	glm::vec4 mousePosNDC(float(cursorX), float(cursorY), 0, 1);

	glm::vec4 mousePosWorld = deprojection * mousePosNDC;

	cursorPos.x = mousePosWorld.x;
	cursorPos.y = mousePosWorld.y;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

}

void GameBase::Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	simpleShader.UseShader();
	glm::mat4 orthoMat = GetCameraTransform();
	simpleShader.SetUniform("vpMatrix", orthoMat);
	//grid.Draw();	//Grid lines don't change so we just draw them.
	lines.UpdateFrame();	//Other lines potentially change every frame, so we have to compile/draw/clear them.
	
	//change matrix for UI triangles
	simpleShader.SetUniform("vpMatrix", textProjectionMatrix);
	//draw triangles
	triangleRenderer.UpdateFrame();
	linesUI.UpdateFrame();
	//draw text
	textShader.UseShader();
	textRenderer.Draw();
	textRenderer.RefreshTextData();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool GameBase::IsRunning() const
{
	return !glfwWindowShouldClose(window);
}

void GameBase::OnMouseClick(int mouseButton)
{
	//Override this function if you need to respond to mouse clicks.
}

void GameBase::OnMouseRelease(int mouseButton)
{
	//Override this function if you need to respond to mouse button releases.
}

void GameBase::OnWindowResize(int width, int height)
{
	oldWindowSize = windowSize;
	windowSize = { width,height };
	aspectRatio = width / (float)height;

	textProjectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
	textShader.SetUniform("textProjection", textProjectionMatrix);
}

void GameBase::OnKeyPressed(int key)
{
	//eg: GLFW_KEY_W
}

void GameBase::OnKeyReleased(int key)
{
	//eg: GLFW_KEY_W

}

void GameBase::Zoom(float zoomFactor)
{
	cameraHeight /= zoomFactor;
}
