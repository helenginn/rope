#pragma once

#include "Atom.h"
#include "Units.h" // IWYU pragma: keep

namespace rope::force_calc::scope
{
	struct All
	{
	};

	struct Sphere
	{
		const Atom &center;
		units::Length radius;
	};
} // namespace rope::force_calc::scope
