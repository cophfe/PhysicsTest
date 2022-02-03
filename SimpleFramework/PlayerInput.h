#pragma once
class PhysicsProgram;

class PlayerInput
{
public:
	void Update(PhysicsProgram& program);
	void Render(PhysicsProgram& program);

	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);
};

