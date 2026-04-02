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

#ifndef __vagabond__AntibodyOrderingView__
#define __vagabond__AntibodyOrderingView__

#include "MultipleSetup.h"
#include <vagabond/utils/svd/PCA.h>
#include <mutex>

struct Mab;
struct Competition;

class AntibodyOrderingView : public MultipleSetup<Competition>
{
public:
	AntibodyOrderingView(Scene *prev, Mab &mab);

	virtual void setup();
	virtual void refresh();

	Competition &comp()
	{
		return *_object;
	}
protected:
	virtual bool acceptable_to_add_after(Competition &comp);
private:
	void makePlot();
	PCA::Matrix _forDisplay{};
	Mab &_mab;
	std::mutex _mutex;

};

#endif
