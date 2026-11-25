#pragma once

#include <cstdint>
#include <string_view>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

namespace rope::force_calc::static_scope
{
	struct ScopeError
	{
		enum class Code : std::uint8_t
		{
			NonFiniteCenter,
			NonFiniteRadius,
			NegativeRadius,
			InnerRadiusExceedsOuterRadius,
		};

		Code code;
		std::string_view toString() const;
	};

	class SphereWithShell
	{
	public:
		static rope::utils::Result<SphereWithShell, ScopeError>
		create(glm::vec3 center_angstroms, double inner_radius_angstroms,
		       double outer_radius_angstroms);

		glm::vec3 centerAngstroms() const
		{
			return _center_angstroms;
		}
		double innerRadiusAngstroms() const
		{
			return _inner_radius_angstroms;
		}
		double outerRadiusAngstroms() const
		{
			return _outer_radius_angstroms;
		}

	private:
		SphereWithShell(glm::vec3 center_angstroms, double inner_radius_angstroms,
		                double outer_radius_angstroms)
		: _center_angstroms(center_angstroms),
		  _inner_radius_angstroms(inner_radius_angstroms),
		  _outer_radius_angstroms(outer_radius_angstroms)
		{
		}

		glm::vec3 _center_angstroms;
		double _inner_radius_angstroms;
		double _outer_radius_angstroms;
	};
}
