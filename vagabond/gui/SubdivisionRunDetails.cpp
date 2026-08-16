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
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/FileReader.h>
#include <ctime>

namespace
{
	// wide/tall enough that most run summaries fit on their own line at
	// full size - Modal::squeezeToWidth() (called below) still shrinks
	// the text further if a particular line ends up too wide regardless.
	const double kWidth = 0.55;
	const double kHeight = 0.55;
}

SubdivisionRunDetails::SubdivisionRunDetails(Scene *scene, Clique *clique,
                                             SubdivisionRun *run,
                                             const std::function<void()>
                                             &onChange)
: Modal(scene, kWidth, kHeight), _clique(clique), _run(run),
_onChange(onChange)
{
	int count = static_cast<int>(run->subdivisions.size());
	int sum = 0;
	for (const Clique &sub : run->subdivisions)
	{
		sum += sub.probes().size();
	}
	float ave = (count > 0 ? sum / (float)count : 0.f);

	std::string details = "Name: " + run->displayName() + "\n";
	details += "Summary: " + std::to_string(count) + " subdivision" +
	           (count == 1 ? "" : "s") + ", average " + f_to_str(ave, 1) +
	           " nodes per subdivision\n";

	if (run->bruteForce)
	{
		details += "Brute force";
	}
	else
	{
		details += "Guide size: " + std::to_string(run->maxNodes) +
		           "; Samples per node: " + std::to_string(run->samplesPerNode);
	}

	if (run->timestamp > 0)
	{
		char buf[64] = {};
		struct tm *info = localtime(&run->timestamp);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", info);
		details += std::string("\nCreated: ") + buf;
	}

	Text *text = new Text(details);
	text->setCentre(0.5, 0.42);
	text->resize(0.85);
	addObject(text);
	squeezeToWidth(text);

	TextButton *rename = new TextButton("Rename", this);
	rename->resize(0.4);
	rename->setReturnTag("rename");
	rename->setRight(0.5 + kWidth / 2 - 0.03, 0.5 - kHeight / 2 + 0.05);
	addObject(rename);

	TextButton *close = new TextButton("Close", this);
	close->setReturnTag("close");
	close->setCentre(0.42, 0.68);
	addObject(close);

	ImageButton *del = new ImageButton("assets/images/cross.png", this);
	del->resize(0.06);
	del->setCentre(0.58, 0.68);
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
	else if (tag == "rename")
	{
		AskForText *aft = new AskForText(_scene, "Rename subdivision run:",
		                                 "", this);
		aft->setDefaultText(_run->displayName());
		aft->allowCapitals(true);
		aft->setStacked(true);
		aft->setReturnJob([this](std::string name)
		{
			if (name.length())
			{
				_run->name = name;

				// content is only ever laid out once (in the constructor),
				// so rather than trying to rebuild this modal's Text/
				// TextButton in place, close back to the run list - it
				// rebuilds fresh from the Clique on every
				// HBondAnalysisControl::refresh() and so already shows
				// the new name.
				hide();

				if (_onChange)
				{
					_onChange();
				}
			}
		});
		_scene->setModal(aft, false);
	}
}
