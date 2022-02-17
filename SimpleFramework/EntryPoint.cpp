#define _CRTDBG_MAP_ALLOC
#include <iostream>
#include <stdlib.h>
#include <crtdbg.h>

#include "PhysicsProgram.h"


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	PhysicsProgram program;

	while (program.IsRunning())
	{
		program.Update();
		program.Render();
	}
	return 0;
}