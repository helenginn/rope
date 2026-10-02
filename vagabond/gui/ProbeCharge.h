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
#include <vagabond/gui/elements/ButtonResponder.h>
#include <vagabond/core/protonic/hnet.h>

class Probe;
class CountProbe;
class ProtonNetworkView;

/** One charge marker in ProtonNetworkView. Right-clicking it offers the
 *  charges it can still take (the same way ProbeBond offers bond states and
 *  ProbeAtom offers presence/absence): choosing one declares it on the
 *  underlying CountConnector through the network's undo stack, so a
 *  contradiction can be undone with Ctrl+Z. */
class ProbeCharge : public FloatingImage, virtual public IndexResponder,
public ButtonResponder
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

	virtual void selected(int idx, bool inverse);

	virtual void reindex();
	void quickUpdate();

	void offerChargeMenu();
	virtual void interacted(int idx, bool hover, bool left);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void fullUpdate();
	void declareCharge(hnet::Count::Values value);

	CountProbe *_probe = nullptr;
	ProtonNetworkView *_view = nullptr;
	bool _selected = false;

};

#endif
