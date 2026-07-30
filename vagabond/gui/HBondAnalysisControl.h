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

#ifndef __vagabond__HBondAnalysisControl__
#define __vagabond__HBondAnalysisControl__

#include <vagabond/gui/elements/Scene.h>

class Clique;
class Network;

class HBondAnalysisControl : public Scene
{
public:
	HBondAnalysisControl(Scene *prev, Clique *clique, Network &network);

	virtual void setup();
	virtual void refresh();
private:
	// true while a SearchAll worker thread may still be iterating/
	// mutating _clique's subdivisions - anything that would delete or
	// replace them (the cross button, re-subdividing, starting a second
	// search) must stay disabled until this is false.
	bool searchIsRunning();

	Clique *_clique{};
	Network &_network;

	std::vector<std::function<void()>> _refreshes;
};

#endif
