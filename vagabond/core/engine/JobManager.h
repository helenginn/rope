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

#ifndef __vagabond__JobManager__
#define __vagabond__JobManager__

#include <vagabond/utils/AcquireCoord.h>
#include <vagabond/utils/glm_import.h>
#include <iostream>

namespace rope
{
	typedef std::function<Coord::Get (const int &)> IntToCoordGet;
	typedef std::function<IntToCoordGet(const std::vector<float> &)> 
	GetListFromParameters;

	typedef std::function<glm::vec3(const Coord::Get &, const int &)> 
	GetVec3FromCoordIdx;

	typedef std::function<float(const Coord::Get &, const int &)> 
	GetFloatFromCoordIdx;
}

class JobManager
{
public:
	void setDefaultCoordTransform(const rope::GetListFromParameters &cgfp)
	{
		_coordGetFromParams = cgfp;
	}
	
	void setAtomFetcher(const rope::GetVec3FromCoordIdx &gvfci)
	{
		_atomFromCoordIdx = gvfci;
	}
	
	void setTorsionFetcher(const rope::GetFloatFromCoordIdx &gffci)
	{
		_torsionFromCoordIdx = gffci;
	}
	
	const rope::GetFloatFromCoordIdx &defaultTorsionFetcher()
	{
		return _torsionFromCoordIdx;
	}
	
	const rope::GetVec3FromCoordIdx &defaultAtomFetcher()
	{
		return _atomFromCoordIdx;
	}
	
	const rope::GetListFromParameters &defaultCoordTransform() const
	{
		return _coordGetFromParams;
	}
	
	static rope::GetFloatFromCoordIdx identityTransformFloat()
	{
		auto get = [](const Coord::Get &all, const int &i) -> float
		{
			if (i >= 0)
			{
				return all(i);
			}
			return 0.f;
		};
		return get;
	}
	
	static rope::GetListFromParameters identityTransform()
	{
		return [](const std::vector<float> &all)
		{
			auto get = [&all](const int &) -> Coord::Get
			{
				return [&all](const int &i) -> float
				{
					if (all.size() > i && i >= 0)
					{
						return all[i];
					}
					return 0;
				};
			};
			return get;
		};
	}

	static rope::GetVec3FromCoordIdx nanPosition()
	{
		return [](const Coord::Get &, const int &) -> glm::vec3
		{
			return glm::vec3(NAN);
		};
	}
private:
	
	// default value: return 1 no matter what
	rope::GetListFromParameters _coordGetFromParams = identityTransform();

	// return non-existent position
	rope::GetVec3FromCoordIdx _atomFromCoordIdx = nanPosition();

	// return non-existent torsion
	rope::GetFloatFromCoordIdx _torsionFromCoordIdx = identityTransformFloat();
};

#endif
