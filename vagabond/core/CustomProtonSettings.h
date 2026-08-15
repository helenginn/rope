// vagabond
// Copyright (C) 2022 Helen Ginn
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__CustomProtonSettings__
#define __vagabond__CustomProtonSettings__

#include <string>
#include <map>
#include <cmath>
#include <nlohmann/json.hpp>
#include <vagabond/utils/maths.h>
using nlohmann::json;

// per-Model pH and per-residue-type pKa values driving proton network
// calculation (see AdjustPhPkas.h in vagabond/gui) - owned by Model
// (Model::protonSettings()) and saved/loaded as part of its own JSON.
class CustomProtonSettings
{
public:
	CustomProtonSettings();

	friend void to_json(json &j, const CustomProtonSettings &value);
	friend void from_json(const json &j, CustomProtonSettings &value);

	// used to detect whether the proton network is stale relative to
	// the currently edited settings - see EditModel's own comment.
	bool operator==(const CustomProtonSettings &other) const
	{
		return _pH == other._pH && _pKas == other._pKas;
	}

	bool operator!=(const CustomProtonSettings &other) const
	{
		return !(*this == other);
	}

	const float &pH() const
	{
		return _pH;
	}

	void setPH(float pH)
	{
		_pH = pH;
	}

	// residue code (e.g. "ARG") -> side chain pKa. Returns 7 (neutral,
	// no meaningful ionisable side chain) for any code not already
	// present - only the amino acids with an ionisable side chain
	// (see the constructor) are ever populated.
	float pKa(const std::string &code) const
	{
		auto it = _pKas.find(code);
		return (it == _pKas.end()) ? 7.f : it->second;
	}

	void setPKa(const std::string &code, float pKa)
	{
		_pKas[code] = pKa;
	}

	const std::map<std::string, float> &pKas() const
	{
		return _pKas;
	}

	// Henderson-Hasselbalch as a Boltzmann-compatible ΔG (kJ/mol),
	// deprotonated relative to protonated, at the given pH and pKa:
	// pKa = pH + log10([HA]/[A-]), so [A-]/[HA] = 10^(pH-pKa), and a
	// state's Boltzmann weight is proportional to exp(-E/RT) - matching
	// those two together (weight ratio = concentration ratio) gives
	// E(deprotonated) - E(protonated) = RT * ln(10) * (pKa - pH).
	// physiological_rt_kjmol (maths.h) is the same RT CertainStates' own
	// Boltzmann weighting uses (probsForAve()/probsForLocalAve()), so
	// feeding this straight into hnet::Energy::energy_wrapper_for_
	// protonation() and letting CertainStates resolve the resulting
	// per-state energies back into probabilities reproduces this same
	// ratio, not merely something proportional to it. Takes pH
	// explicitly (rather than always this object's own _pH) so a live
	// caller (Network::effectivePH(), for testing a pH nudge without a
	// full rebuild - see its own comment) can override it.
	static float deprotonationEnergyFor(float pKa, float pH)
	{
		return physiological_rt_kjmol * (float)std::log(10.0) * (pKa - pH);
	}

	// this settings object's own pH - only the two-state (proto/deproto)
	// case, not meaningful for histidine's two independent ring pKas at
	// once, so callers there pass "HIS2" or "HIS1" for whichever single
	// boundary is actually being modelled.
	float deprotonationEnergy(const std::string &code) const
	{
		return deprotonationEnergyFor(pKa(code), _pH);
	}

private:
	float _pH = 7.f;
	std::map<std::string, float> _pKas;
};

inline CustomProtonSettings::CustomProtonSettings()
{
	// standard side chain pKa values for the amino acids with an
	// ionisable side chain.
	_pKas["ARG"] = 12.5f;
	_pKas["LYS"] = 10.5f;
	_pKas["CYS"] = 8.3f;
	_pKas["GLU"] = 4.3f;
	_pKas["ASP"] = 3.9f;
	_pKas["SER"] = 13.5f;
	_pKas["THR"] = 13.5f;
	_pKas["TYR"] = 10.5f;

	// histidine's imidazole has two relevant pKas, not one like every
	// other entry here - HIS2 (doubly protonated, cationic imidazolium
	// -> neutral imidazole) and HIS1 (singly protonated neutral
	// imidazole -> imidazolate anion, which essentially never happens
	// physiologically, hence the very high default). See AdjustPhPkas.cpp,
	// which lists both as separate entries.
	_pKas["HIS2"] = 6.0f;
	_pKas["HIS1"] = 15.0f;
}

inline void to_json(json &j, const CustomProtonSettings &value)
{
	j["pH"] = value._pH;
	j["pKas"] = value._pKas;
}

inline void from_json(const json &j, CustomProtonSettings &value)
{
	if (j.count("pH"))
	{
		value._pH = j.at("pH");
	}

	if (j.count("pKas"))
	{
		std::map<std::string, float> pKas = j.at("pKas");
		value._pKas = pKas;
	}
}

#endif
