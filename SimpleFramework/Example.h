#pragma once

#include "GameBase.h"


class Example : public GameBase
{


public:
	Example();

	void Update();

	void Render();

	void OnMouseClick(int mouseButton);
};