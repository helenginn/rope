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

#ifndef __vagabond__ProbeBond__
#define __vagabond__ProbeBond__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/elements/IndexResponder.h>

class BondProbe;
class ProtonNetworkView;

class ProbeBond : public Image, public ButtonResponder,
virtual public IndexResponder
{
public:
	ProbeBond(ProtonNetworkView *view, BondProbe *probe);

	void updateProbe();
	void fixVertices(const glm::vec3 &start, const glm::vec3 &dir);
	
	virtual size_t requestedIndices();
	
	void reindex();
	void interacted(int idx, bool hover, bool left);
	void offerBondMenu();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void declareBond(hnet::Bond::Values value);

	BondProbe *_probe = nullptr;
	ProtonNetworkView *_view = nullptr;

};

#endif
