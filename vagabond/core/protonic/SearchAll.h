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

#ifndef __vagabond__SearchAll__
#define __vagabond__SearchAll__

#include <vagabond/core/Progressor.h>
#include <atomic>
#include <memory>

class Clique;
class Network;

class SearchAll : public Progressor
{
public:
	SearchAll(Clique *parent, Network &network);

	void run();

	/** shared, not owned: this object is deleted by the worker thread as
	 * soon as run() returns, which could race with a cancel button click
	 * on the main thread if the flag lived here instead. */
	void setCancelFlag(std::shared_ptr<std::atomic<bool>> flag)
	{
		_cancel = flag;
	}
private:
	Clique *_clique{};
	Network &_network;
	std::shared_ptr<std::atomic<bool>> _cancel;

};

#endif
