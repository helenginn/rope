#pragma once

//include other force scopes here
#include "Sphere.h"          // IWYU pragma: export
#include "SphereWithShell.h" // IWYU pragma: export

#include <variant>

namespace rope::force_calc::static_scope
{
	struct All {};

	using ForceScope = std::variant<All, Sphere, SphereWithShell>;
}
