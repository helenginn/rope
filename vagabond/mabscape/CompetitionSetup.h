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

#ifndef __vagabond__CompetitionSetup__
#define __vagabond__CompetitionSetup__

#include "MultipleSetup.h"
#include "Mab.h"

class CompetitionSetup : public MultipleSetup<Competition>
{
public:
	CompetitionSetup(Scene *scene, Competitions &comp, 
	                 Antigens &antigens);

	void setup();
	virtual void refresh();
	
	Competition &comp()
	{
		return *_object;
	}
	
private:
	void guessHeaders();
	void showViewTable();
	void prepareChooseCSV();
	void relevantHeaders();
	void howToReadResult();
	void relevantAntigen();

protected:
	virtual bool acceptable_to_add_after(Competition &comp);

	Antigens &_antigens;

};

#endif
