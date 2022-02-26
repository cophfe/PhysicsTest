#pragma once
#include "Maths.h"

namespace fzx
{
	class Transform
	{
	public:
		Vector2 position;
		float rotation;

		void UpdateData();
		Vector2 TransformPoint(Vector2 point);
		Vector2 InverseTransformPoint(Vector2 point);
		Vector2 TransformDirection(Vector2 direction);
		Vector2 InverseTransformDirection(Vector2 direction);

		Matrix3x3 GetTransformationMatrix();
		Matrix4x4 Get3DTransformationMatrix();
		Transform() = default;
		Transform(Vector2 position, float rotation);

	private:
		//so that sine and cosine calculations only have to happen once per frame
		float s;
		float c;
	};
}