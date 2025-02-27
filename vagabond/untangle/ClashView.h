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

#ifndef __vagabond__ClashView__
#define __vagabond__ClashView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/svd/PCA.h>
#include <mutex>

class AtomGroup;
class NonBonds;

class ClashView : public Scene
{
public:
	ClashView(Scene *prev, AtomGroup *group, NonBonds *nonBonds);

	virtual void setup();
private:
	AtomGroup *_group{};
	NonBonds *_nonBonds{};

	PCA::Matrix _mat{};
	std::mutex _mutex;
};

#endif
