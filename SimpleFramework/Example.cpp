#include "Example.h"


Example::Example() : GameBase()
{
	//Your initialisation code goes here!
}

void Example::Update()
{
	//This call ensures that your mouse position and aspect ratio are maintained as correct.
	GameBase::Update();

	//Your physics (or whatever) code goes here!

}

void Example::Render()
{

	//Example code that draws a coloured circle at the mouse position, whose colour depends on which buttons are down.
	if (leftButtonDown)
	{
		lines.DrawCircle(cursorPos, 0.2f, { 1, 0, 0 });
	}
	else if (rightButtonDown)
	{
		lines.DrawCircle(cursorPos, 0.2f, { 0, 1, 0 });
	}
	else
	{
		lines.DrawCircle(cursorPos, 0.2f, { 0, 0, 1 });
	}

	//Your drawing code goes here!

	lines.DrawLineSegment({ 0, 0 }, { 5, 10 }, { 0, 1, 0 });	//Draw a line from the origin to the point (5.0, 10.0) in green.


	//This call puts all the lines you've set up on screen - don't delete it or things won't work.
	GameBase::Render();
}

void Example::OnMouseClick(int mouseButton)
{

}
