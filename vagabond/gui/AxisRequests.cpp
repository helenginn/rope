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

#include <vagabond/core/RAMovement.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/ConfSpaceView.h>
#include <vagabond/gui/RouteExplorer.h>
#include <vagabond/core/PositionData.h>
#include <vagabond/core/TorsionData.h>
#include <vagabond/core/BFactorData.h>
#include <vagabond/core/paths/PlausibleRoute.h>
#include <vagabond/core/paths/NewPath.h>
#include <vagabond/c4x/ClusterSVD.h>

#include "AxisExplorer.h"
#include "Atom2AtomExplorer.h"
#include "Axes.h"

#include "AxisRequests.h"

DoEditMenu editMenu(PositionData *group)
{
	return [](Axes *axes, Menu *menu)
	{
		menu->addOption("explore axis", "explore_axis");
	};
};

DoEditMenu editMenu(TorsionData *group)
{
	return [](Axes *axes, Menu *menu)
	{
		menu->addOption("explore axis", "explore_axis");
		if (axes->status() == Axes::PointingToOther)
		{
			menu->addOption("find route", "route");
		}
	};
};

template <typename Type, class ClusterType, class DataType>
std::vector<Type> vectorFrom(glm::vec3 dir, ClusterType *cluster, 
                             DataType *data)
{
	std::vector<Type> sums;
	for (size_t i = 0; i < 3; i++)
	{
		if (i >= cluster->rows())
		{
			continue;
		}

		int axis = cluster->axis(i);
		std::vector<Type> vals = data->rawVector(cluster, axis);

		float &weight = dir[i];
		
		if (sums.size() == 0)
		{
			sums.resize(vals.size());
		}
		
		for (size_t j = 0; j < vals.size(); j++)
		{
			sums[j] += vals[j] * weight;
		}
	}
	
	return sums;
}


DoRequest doRequest(PositionData *group)
{
	return [group](Axes *axes, const std::string &request)
	{
		glm::vec3 axis = axes->axisInQuestion();

		if (request == "explore_axis")
		{
			std::vector<Atom3DPosition> list;
			list = group->headers();
			std::vector<Posular> vec;
			vec = vectorFrom<Posular>(axis, axes->cluster(), group);

			if (vec.size() == 0)
			{
				return;
			}

			RAMovement movement = RAMovement::movements_from(list, vec);

			std::string str = axes->titleForAxis();
			Instance *instance = static_cast<Instance *>(axes->focus());
			ConfSpaceView *scene = axes->scene();

			Atom2AtomExplorer *a2a = new Atom2AtomExplorer(scene, instance, movement);
			a2a->setFutureTitle(str);
			a2a->show();
		}
	};
};

DoRequest doRequest(TorsionData *group)
{
	return [group](Axes *axes, const std::string &request)
	{
		if (request == "explore_axis")
		{
			glm::vec3 axis = axes->axisInQuestion();
			std::vector<Angular> vals;
			vals = vectorFrom<Angular>(axis, axes->cluster(), group);

			const RTAngles &empty = group->emptyAngles();
			RTAngles angles = RTAngles::angles_from(empty.headers_only(), vals);

			std::string str = axes->titleForAxis();
			Instance *instance = static_cast<Instance *>(axes->focus());
			ConfSpaceView *scene = axes->scene();

			try
			{
				AxisExplorer *ae = new AxisExplorer(scene, instance, angles);
				ae->setCluster(axes->cluster(), group);
				ae->setFutureTitle(str);
				ae->show();
			}
			catch (const std::runtime_error &err)
			{
				BadChoice *bc = new BadChoice(Window::currentScene(), err.what());
				Window::currentScene()->setModal(bc);
			}
		}
		else if (request == "route")
		{
			int start_idx = -1; int end_idx = -1;
			axes->indicesOfObjectsPointedAt(start_idx, end_idx);

			Instance *start = static_cast<Instance *>(group->object(start_idx));
			Instance *end = static_cast<Instance *>(group->object(end_idx));
			
			NewPath new_path(start, end);
			PlausibleRoute *pr = new_path();

			ConfSpaceView *scene = axes->scene();
			scene->setMadePaths();

			RouteExplorer *re = new RouteExplorer(scene, pr);
			re->show();

		}
	};
};

DoEditMenu editMenu(BFactorData *group)
{
	return {};
};

DoRequest doRequest(BFactorData *group)
{
	return {};
};

DoEditMenu editMenu(PathData *group)
{
	return {};
};

DoRequest doRequest(PathData *group)
{
	return {};
};

