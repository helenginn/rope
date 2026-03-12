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

#ifndef __vagabond__ContactPoint__
#define __vagabond__ContactPoint__

#include <vector>

struct Fiducial;
class Antigens;
class Instance;
struct Antigen;

class ContactPoint
{
public:
	ContactPoint(Fiducial &fiducial, Antigens &antigens);

private:
	Fiducial &_fiducial;
	Antigens &_antigens;
	Antigen *_chosen{};

	// instances belonging to fiducial for antibody
	std::vector<Instance *> _iFiducials;

	// instances belonging to fiducial mirroring antigen
	std::vector<Instance *> _iFidAntigens;

	// instances belonging to antigen
	std::vector<Instance *> _iAntigens;
};

#endif
