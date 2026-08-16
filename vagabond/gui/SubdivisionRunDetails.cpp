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

#include "SubdivisionRunDetails.h"
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/utils/FileReader.h>
#include <ctime>

SubdivisionRunDetails::SubdivisionRunDetails(Scene *scene, Clique *clique,
                                             SubdivisionRun *run,
                                             const std::function<void()>
                                             &onChange)
: Modal(scene, 0.5, 0.4), _clique(clique), _run(run), _onChange(onChange)
{
	int count = static_cast<int>(run->subdivisions.size());
	int sum = 0;
	for (const Clique &sub : run->subdivisions)
	{
		sum += sub.probes().size();
	}
	float ave = (count > 0 ? sum / (float)count : 0.f);

	std::string details = run->description() + "\n";
	details += std::to_string(count) + " subdivision" +
	           (count == 1 ? "" : "s") + "\n";
	details += "average " + f_to_str(ave, 1) + " nodes";

	if (!run->bruteForce)
	{
		details += "\nguide size: " + std::to_string(run->maxNodes);
		details += "\nsamples per node: " + std::to_string(run->samplesPerNode);
	}

	if (run->timestamp > 0)
	{
		char buf[64] = {};
		struct tm *info = localtime(&run->timestamp);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", info);
		details += std::string("\ncreated: ") + buf;
	}

	Text *text = new Text(details);
	text->setCentre(0.5, 0.35);
	text->resize(0.9);
	addObject(text);
	squeezeToWidth(text);

	TextButton *close = new TextButton("Close", this);
	close->setReturnTag("close");
	close->setCentre(0.4, 0.62);
	addObject(close);

	ImageButton *del = new ImageButton("assets/images/cross.png", this);
	del->resize(0.06);
	del->setCentre(0.6, 0.62);
	del->setReturnTag("delete");
	addObject(del);
}

void SubdivisionRunDetails::buttonPressed(std::string tag, Button *button)
{
	if (tag == "close")
	{
		hide();
	}
	else if (tag == "delete")
	{
		_clique->removeSubdivisionRun(_run);
		hide();

		if (_onChange)
		{
			_onChange();
		}
	}
}
