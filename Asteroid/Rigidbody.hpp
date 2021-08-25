#ifndef RIGIDBODY
#define RIGIDBODY
#include "MathUtil.hpp"

struct Rigidbody
{
	Vec2f velocity;
	Vec2f accumulated_force;
};

#endif // !RIGIDBODY
