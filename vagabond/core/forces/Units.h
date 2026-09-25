#pragma once

#include "au/au.hh" // IWYU pragma: export
#include "au/units/meters.hh"

namespace rope::force_calc::units
{
	using namespace au;

	struct Angstroms : decltype(au::Meters{} / au::pow<10>(au::mag<10>()))
	{
		static constexpr inline const char label[] = "A";
	};

	inline constexpr auto angstroms = au::QuantityMaker<Angstroms>{};

	inline constexpr auto angstrom = au::SingularNameFor<Angstroms>{};

	namespace symbols
	{
		inline constexpr auto A = au::SymbolFor<Angstroms>{};
	}

} // namespace rope::force_calc::units
