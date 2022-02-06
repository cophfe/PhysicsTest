#include "GLFWCallbacks.h"
#include "GameBase.h"

#define ZOOM_FACTOR 1.2f

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	GameBase* programPointer = (GameBase*)glfwGetWindowUserPointer(window);	
	programPointer->OnWindowResize(width, height);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GameBase* programPointer = (GameBase*)glfwGetWindowUserPointer(window);	//This is an unsafe coercive cast, but we just have to be careful and live with it.
	if (action == GLFW_PRESS)
	{
		programPointer->OnMouseClick(button);
	}
	else if (action == GLFW_RELEASE)
	{
		//We don't just do 'else' here because 'action' can also be GLFW_REPEAT. Operating systems send this when a button or key is held down, depending on the settings, but we're going to ignore it.
		programPointer->OnMouseRelease(button);
	}
}

void MouseWheelCallback(GLFWwindow* window, double xDelta, double yDelta)
{
	GameBase* programPointer = (GameBase*)glfwGetWindowUserPointer(window);
	if (yDelta > 0)
	{
		programPointer->Zoom(ZOOM_FACTOR);
	}
	else
	{
		programPointer->Zoom(1.0f / ZOOM_FACTOR);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GameBase* programPointer = (GameBase*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS)
	{
		programPointer->OnKeyPressed(key);
	}
	else if (action == GLFW_RELEASE)
	{
		programPointer->OnKeyReleased(key);
	}
}
