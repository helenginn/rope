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

#ifndef __vagabond__Positions__
#define __vagabond__Positions__

#include <vagabond/utils/glm_import.h>
#include <mutex>
#include <map>

struct Competition;
struct Fiducial;
struct Antigen;
struct Mab;

class AbWatch;
class Symmetry;
class HasRenderables;

class Positions
{
public:
	typedef std::function<void(const glm::vec3 &, glm::vec3 &, int &)> FromMesh;

	Positions(Antigen &antigen, const Competition &comp, Mab &mab,
	          const FromMesh &fm);
	~Positions();
	
	
	void setPosition(const std::string &name, glm::vec3 pos);
	void loadAntibodiesInto(HasRenderables *bucket);
private:
	Antigen &_antigen;
	Mab &_mab;
	
	struct AntibodyPos
	{
		void setPosition(std::vector<glm::vec3> &raw, glm::vec3 ref, 
		                 const FromMesh &fromMesh);
		
		~AntibodyPos()
		{

		}
		
		AbWatch *icoAbWatch(const std::vector<glm::vec3> &raw, 
		                    int offset) const;

		Fiducial *fid{};
		std::string name{};
		int start_idx = -1; // for accessing _raw
		int num = 1;
		Symmetry *sym{};
		unsigned int version = 0;
		std::mutex *mut{};
		int meshIdx = -1;
	};

	std::vector<AntibodyPos> _positions;
	std::vector<glm::vec3> _raw;
	std::map<std::string, AntibodyPos *> _lookup;
	FromMesh _fromMesh;
};

#endif
