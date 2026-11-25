#pragma once

#include "SphereError.h"

#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

namespace rope::force_calc::static_scope
{
	class Sphere
	{
	public:
		static rope::utils::Result<Sphere, SphereError>
		create(glm::vec3 center_angstroms, double radius_angstroms);

		glm::vec3 centerAngstroms() const
		{
			return _center_angstroms;
		}
		double radiusAngstroms() const
		{
			return _radius_angstroms;
		}

		// The boundary is included.
		bool contains(const glm::vec3 &position_angstroms) const;

	private:
		Sphere(glm::vec3 center_angstroms, double radius_angstroms)
		: _center_angstroms(center_angstroms),
		  _radius_angstroms(radius_angstroms)
		{
		}

		glm::vec3 _center_angstroms;
		double _radius_angstroms;
	};
}
