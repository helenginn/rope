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

#ifndef __vagabond__ProbeCharge__
#define __vagabond__ProbeCharge__

#include <vagabond/gui/elements/FloatingImage.h>
#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/core/protonic/hnet.h>

class Probe;
class CountProbe;
class ProtonNetworkView;

class ProbeCharge : public FloatingImage, virtual public IndexResponder
{
public:
	ProbeCharge(ProtonNetworkView *view, CountProbe *probe);

	CountProbe *probe()
	{
		return _probe;
	}

	virtual size_t requestedIndices()
	{
		return 1;
	}

	virtual bool selectable() const
	{
		return false;
	}

	virtual void reindex();
	void quickUpdate();
private:
	void fullUpdate();

	CountProbe *_probe = nullptr;
	ProtonNetworkView *_view = nullptr;

};

#endif
