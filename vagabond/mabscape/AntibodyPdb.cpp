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

#include "AntibodyPdb.h"
#include <algorithm>
#include <gemmi/model.hpp>
#include <gemmi/to_pdb.hpp>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

std::vector<AntibodyPdb::Naming> 
AntibodyPdb::assignNames(const std::vector<std::string> &names)
{
	// the most a PDB file can number a residue
	const int biggest = 9999;
	std::set<std::pair<std::string, int>> taken;
	std::vector<Naming> all(names.size());

	// those which give up take the next free of these, in this order
	const std::string chains = "ZYXWVUTSRQPONMLKJIHGFEDCBA"
	"zyxwvutsrqponmlkjihgfedcba9876543210";
	size_t giveUpChain = 0;
	int giveUpResidue = 0;

	auto giveUp = [&]()
	{
		while (giveUpChain < chains.size())
		{
			giveUpResidue++;
			if (giveUpResidue > biggest)
			{
				giveUpResidue = 0;
				giveUpChain++;
				continue;
			}

			std::string chain(1, chains[giveUpChain]);
			if (!taken.count({chain, giveUpResidue}))
			{
				taken.insert({chain, giveUpResidue});
				return Naming{chain, giveUpResidue, true};
			}
		}

		throw std::runtime_error("Too many antibodies to name");
	};

	// the ones which can be named as they are first, so that no antibody
	// which gives up takes the name which another would have had
	std::vector<bool> placed(names.size(), false);
	for (size_t i = 0; i < names.size(); i++)
	{
		const std::string &name = names[i];
		std::string chain, digits;
		for (char c : name)
		{
			if (chain.empty() && std::isalpha((unsigned char)c))
			{
				chain = std::string(1, c);
			}
			if (std::isdigit((unsigned char)c))
			{
				digits += c;
			}
		}

		if (chain.empty() || digits.empty() || digits.size() > 4)
		{
			continue;
		}

		int residue = std::stoi(digits);
		if (residue > biggest || taken.count({chain, residue}))
		{
			continue;
		}

		taken.insert({chain, residue});
		all[i] = {chain, residue, false};
		placed[i] = true;
	}

	for (size_t i = 0; i < names.size(); i++)
	{
		if (!placed[i])
		{
			all[i] = giveUp();
		}
	}

	return all;
}

void AntibodyPdb::write(const std::string &path, 
                        const std::vector<Antibody> &antibodies,
                        const std::string &reference, const std::string &title,
                        size_t results)
{
	std::vector<std::string> names;
	size_t states = 0;
	for (const Antibody &ab : antibodies)
	{
		names.push_back(ab.name);
		states = std::max(states, ab.copies.size());
	}

	std::vector<Naming> naming = assignNames(names);

	gemmi::Structure st;
	st.name = "antibodies";

	auto remark = [&st](const std::string &text)
	{
		st.raw_remarks.push_back("REMARK 999 " + text);
	};

	remark("MABSCAPE ANTIBODY POSITIONS: " + title);
	remark("REFERENCE FRAME (ANTIGEN) PDB: " + reference);
	remark("RESULTS AVERAGED: " + std::to_string(results));
	remark("ONE MERCURY ATOM (HG) PER ANTIBODY, AT THE MEDIAN MESH VERTEX OF");
	remark("ITS POSITIONS (OR WHERE IT IS FIXED).");
	remark("B FACTOR: RMSD (ANGSTROMS) OF ITS POSITIONS, AS CALCULATED BY");
	remark("THE CROSS-CHECK. 0 FOR A FIXED ANTIBODY, WHICH DOES NOT MOVE.");
	remark("EACH MODEL IS ONE SYMMETRY COPY (ALTERNATIVE POSITION) OF EVERY");
	remark("ANTIBODY. NUMBER OF MODELS: " + std::to_string(states));
	for (size_t i = 0; i < antibodies.size(); i++)
	{
		const Antibody &ab = antibodies[i];
		std::ostringstream line;
		line << "ATOM HG CHAIN " << naming[i].chain << " RESIDUE " 
		<< naming[i].residue << " IS ANTIBODY " << ab.name;
		remark(line.str());

		std::ostringstream detail;
		detail << "    RMSD " << std::fixed << std::setprecision(2) << ab.rmsd
		<< " A, " << ab.copies.size() << " COPIES" 
		<< (ab.fixed ? ", FIXED" : "")
		<< (naming[i].gaveUp ? ", NAME DID NOT FIT: ASSIGNED" : "");
		remark(detail.str());
	}

	for (size_t state = 0; state < states; state++)
	{
		gemmi::Model model(std::to_string(state + 1));
		std::map<std::string, size_t> chainAt;

		for (size_t i = 0; i < antibodies.size(); i++)
		{
			const Antibody &ab = antibodies[i];
			if (state >= ab.copies.size())
			{
				continue;
			}

			const std::string &chain = naming[i].chain;
			if (!chainAt.count(chain))
			{
				chainAt[chain] = model.chains.size();
				model.chains.push_back(gemmi::Chain(chain));
			}

			gemmi::Residue res;
			res.name = "HG";
			res.seqid = gemmi::SeqId(naming[i].residue, ' ');
			res.het_flag = 'H';

			gemmi::Atom atom;
			atom.name = "HG";
			atom.element = gemmi::Element("Hg");
			atom.occ = 1.0f;
			atom.b_iso = ab.rmsd;
			const glm::vec3 &p = ab.copies[state];
			atom.pos = gemmi::Position(p.x, p.y, p.z);

			res.atoms.push_back(atom);
			model.chains[chainAt[chain]].residues.push_back(res);
		}

		st.models.push_back(model);
	}

	std::ofstream file(path);
	if (!file)
	{
		throw std::runtime_error("Could not write " + path);
	}

	// there is no cell or space group to put in
	gemmi::PdbWriteOptions options;
	options.cryst1_record = false;
	gemmi::write_pdb(st, file, options);

	if (!file)
	{
		throw std::runtime_error("Could not write " + path);
	}
}
