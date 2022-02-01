#include <iostream>


//Include the necessary header file here for your game type
#include "PhysicsProgram.h"


int main()
{
	PhysicsProgram program;	//Make the type of this variable your game application and you'll be good to go.

	while (program.IsRunning())
	{
		program.Update();
		program.Render();
	}
	return 0;
}