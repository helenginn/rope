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

#include <vagabond/gui/elements/TextEntry.h>

namespace FocusResidue
{

	inline void prepareEnter(Scene *scene, const 
	                         std::function<void(std::string, int)> &doNext)
	{
		TextEntry *te = new TextEntry("enter residue", scene);
		te->setValidationType(TextEntry::None);
		
		auto process_request = [scene, te, doNext]()
		{
			// expecting a request like "16" or "B16"
			std::string str = te->scratch();
			char *ch = &str[0];
			// navigate to beginning of numbers, leave chain as it is.
			while (!(*ch >= '0' && *ch <= '9') && *ch != '\0') 
			{
				ch++;
			}

			// grab the residue number
			int res = atoi(ch); 
			std::string chain = "";
			
			// in the event of chain, grab separate chain
			if (ch != &str[0])
			{
				*ch = '\0';
				chain = std::string(&str[0]);
				std::transform(chain.begin(), chain.end(), 
				               chain.begin(), ::toupper);
			}

			doNext(chain, res);
			scene->removeObject(te);
		};

		te->setReturnJob(process_request);
		te->setCentre(0.5, 0.3);
		scene->addObject(te);
		
		scene->addMainThreadJob([te]() { te->click(); });

	}

};
