#include "fzx.h"
namespace fzx
{
	Transform::Transform(Vector2 position, float rotation) : position(position), rotation(rotation)
	{
		UpdateData();
	}

	void Transform::UpdateData() { //updates once per frame
		s = sin(rotation);
		c = cos(rotation);
	}

	Vector2 Transform::TransformPoint(Vector2 point)
	{
		return Vector2(point.x * c - point.y * s + position.x, point.y * c + point.x * s + position.y);
	}

	Vector2 Transform::InverseTransformPoint(Vector2 point)
	{
		Vector2 p = point;
		p -= position;

		//just negate sine to invert rotation 
		//this is because -sin(50) == sin(-50) but cos(50) == cos(-50)
		return Vector2(p.x * c + p.y * s, p.y * c - p.x * s);
	}


	Vector2 Transform::TransformDirection(Vector2 direction)
	{
		return Vector2(direction.x * c - direction.y * s, direction.y * c + direction.x * s);
	}

	Vector2 Transform::InverseTransformDirection(Vector2 direction)
	{
		return Vector2(direction.x * c + direction.y * s, direction.y * c - direction.x * s);
	}

	Matrix3x3 Transform::GetTransformationMatrix()
	{
		return Matrix3x3(
			c, -s, position.x,
			s, c, position.y,
			0.0f, 0.0f, 1.0f
		);
	}

	//is useful for rendering maybe
	Matrix4x4 Transform::Get3DTransformationMatrix()
	{
		Matrix4x4 m = glm::rotate(Matrix4x4(), rotation, Vector3(0, 0, 1));
		return  glm::translate(m, Vector3(position.x, position.y, 0));
	}

}