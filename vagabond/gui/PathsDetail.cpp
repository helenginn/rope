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

#include <gemmi/to_pdb.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "PathsDetail.h"
#include <vagabond/core/RouteValidator.h>
#include "RouteExplorer.h"
#include "PdbFile.h"
#include <vagabond/core/Path.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/AskMultipleChoice.h>

PathsDetail::PathsDetail(Scene *prev, Path *p) : 
Scene(prev),
AddObject<Path>(prev, p)
{
	calculateMetrics();
}

void PathsDetail::calculateMetrics()
{
	PlausibleRoute *pr = _obj.toRoute();
	pr->setup();

	RouteValidator rv(*pr);

	_linearity = rv.linearityRatio();
	_valid = rv.validate();
	_rmsd = rv.rmsd();
	_obj.cleanupRoute();
}

void PathsDetail::refresh()
{
	calculateMetrics();
	redraw();
}

void PathsDetail::redraw()
{
	deleteTemps();

	float top = 0.3;
	{
		std::string str = _valid ? "valid" : "not valid";
		Text *t = new Text(str);
		t->setRight(0.8, top);
		addTempObject(t);
		if (!_valid)
		{
			t->setColour(0.5, 0.0, 0.0);

			top += 0.06;
			std::string str = ("Deviation: " + std::to_string(_rmsd) 
			                   + " Angstroms");
			Text *t = new Text(str);
			t->addAltTag("Threshold of validity would be 0.02 Angstroms");
			t->setRight(0.8, top);
			addTempObject(t);

			Image *image = new Image("assets/images/sad_face.png");
			image->resize(0.04);
			image->setCentre(0.83, top);
			addTempObject(image);
		}
	}
	
	top = 0.5;
	
	{
		std::string str = std::to_string(_linearity);
		Text *t = new Text(str);
		t->addAltTag("Well-refined paths usually have linearity above 0.9");
		t->setRight(0.8, top);
		addTempObject(t);
		
		if (_linearity > 0.9 && _valid)
		{
			Image *image = new Image("assets/images/happy_face.png");
			image->resize(0.04);
			image->setCentre(0.83, top);
			addTempObject(image);
		}
	}
}

void PathsDetail::setup()
{
	addTitle("Path details");
	AddObject::setup();

	Text *t = new Text(_obj.desc());
	t->setCentre(0.5, 0.15);
	t->squishToWidth(0.8);
	addObject(t);
	
	float top = 0.3;
	
	{
		Text *t = new Text("Route validity:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	
	top = 0.5;
	
	{
		Text *t = new Text("Measure of linearity:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	
	top = 0.6;

	{
		Text *t = new Text("Note: path validation does not currently include any "\
		                   "estimation of\natomic clashes or remedy against them.");
		t->squishToWidth(0.6);
		t->setLeft(0.2, top);
		addObject(t);
	}

	{
		TextButton *t = new TextButton("View", this);
		t->setLeft(0.2, 0.8);
		t->setReturnTag("view");
		addObject(t);
	}

	{
		TextButton *t = new TextButton("Export as PDB", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("export");
		addObject(t);
	}
	
	redraw();
}

void PathsDetail::buttonPressed(std::string tag, Button *button)
{
	if (tag == "delete" && _deleteAllowed)
	{
		Environment::purgePath(_obj);
		back();
	}
	
	if (tag == "view")
	{
		PlausibleRoute *pr = _obj.toRoute();
		RouteExplorer *re = new RouteExplorer(this, pr);
		re->show();
	}
	
	if (tag == "export")
	{
#ifndef __EMSCRIPTEN__
		AskMultipleChoice *amc = nullptr;
		amc = new AskMultipleChoice(this, "What kind of ensemble format would you"
		                            " like to export?", "format", this);
		amc->addChoice("One PDB file per structure", "per_structure");
		amc->addChoice("Ensemble PDB file with all structures", "ensemble");
		setModal(amc);
#else
	buttonPressed("format_ensemble", nullptr);
	return;
#endif
	}
	
	std::string end = Button::tagEnd(tag, "format_");
	if (end.length())
	{
		_ensemble = (end == "ensemble");
		AskForText *aft = new AskForText(this, "How many samples along the path?", 
		                                 "samples", this, TextEntry::Numeric);
		setModal(aft);
	}
	
	if (tag == "samples")
	{
		std::string str = static_cast<TextEntry *>(button)->scratch();
		int num = atoi(str.c_str());
		_samples = num;

		AskForText *aft = new AskForText(this, "PDB filename?", 
		                                 "filename", this);
		setModal(aft);
	}
	
	if (tag == "filename")
	{
		std::string filename = static_cast<TextEntry *>(button)->scratch();
		exportPDB(filename, _samples, _ensemble);
	}

	AddObject::buttonPressed(tag, button);
}

int count_chars(const std::string &s, const char &ch)
{
	int count = 0;

	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == ch)
		{
			count++;
		}
	}

	return count;
}

void check_path_and_make(std::string &path)
{
	if (path.back() == '/')
	{
		path.pop_back();
	}
	
	if (count_chars(path, '/') > 1)
	{
		std::string err = ("Please no more than one subdirectory down."\
		                   "\nI cannot cope with such complexities.");
		throw std::runtime_error(err);
	}

	if (path.find('.') != std::string::npos)
	{
		std::string err = ("If you are going to add a path, please don't "\
		                   "\nuse full stops (periods), it is dangerous "\
		                   "for your filesystem.");
		throw std::runtime_error(err);
	}
	else if (path.find('/') == 0)
	{
		std::string err = ("If you are going to add a path, please don't "\
		                   "\nstart with a forwardslash. I don't want to be "\
		                   "\nresponsible for ruining your filesystem.");
		throw std::runtime_error(err);
	}
	else if (file_exists(path) && !is_directory(path))
	{
		std::string err = ("This path already exists as a file and I refuse"\
		                   "\nto overwrite it. Please choose another name.");
		throw std::runtime_error(err);
	}

	FileReader::makeDirectoryIfNeeded(path);

}

void PathsDetail::attemptEnsemble(const std::string &filename, const int &num)
{
	std::string path = getPath(filename);
	std::string file = getFilename(filename);

	if (path.length())
	{
		check_path_and_make(path);
	}

	PlausibleRoute *pr = _obj.toRoute();
	AtomGroup *grp = pr->instance()->currentAtoms();
	pr->setup();

	gemmi::Structure st;
	float step = 1 / (float)num;
	for (size_t i = 0; i < num; i++)
	{
		float frac = i * step;
		pr->submitJobAndRetrieve(frac, true);

		std::string model_name = std::to_string(i + 1);
		PdbFile::writeAtomsToStructure(grp, st, model_name);
	}
	
	PdbFile::writeStructure(st, filename);

	_obj.cleanupRoute();
}

void PathsDetail::attemptPerStruct(const std::string &filename, const int &num)
{
	std::string path = getPath(filename);
	std::string base = getBaseFilename(filename);
	std::string ext = getExtension(filename);
	
	if (path.length())
	{
		check_path_and_make(path);
	}
	
	PlausibleRoute *pr = _obj.toRoute();
	AtomGroup *grp = pr->instance()->currentAtoms();
	pr->setup();

	float step = 1 / (float)num;
	for (size_t i = 0; i < num; i++)
	{
		float frac = i * step;
		pr->submitJobAndRetrieve(frac, true);

		std::string fullpath = path + "/" + base + "_" + std::to_string(i)
		+ "." + ext;
		
		PdbFile::writeAtoms(grp, fullpath);
	}

	_obj.cleanupRoute();
}

void PathsDetail::exportPDB(const std::string &filename, const int &num,
                            const bool &ensemble)
{
	std::cout << "exporting " << filename << " with " << num << " samples "
	<< "as " << (ensemble ? "an ensemble file" : "a file per structure")
	<< std::endl;

	try
	{
		if (ensemble)
		{
			attemptEnsemble(filename, num);
		}
		else
		{
			attemptPerStruct(filename, num);
		}
	}
	catch (const std::runtime_error &err)
	{
		BadChoice *b = nullptr;
		b = new BadChoice(this, err.what());
		setModal(b);
		return;
	}

#ifndef __EMSCRIPTEN__
	BadChoice *b = nullptr;
	b = new BadChoice(this, "Exported the path in " + std::to_string(num) +
	                  " subdivisions.");
	setModal(b);
#else
	try
	{
		std::string contents = get_file_contents(filename);
		EM_ASM_({ window.download = download; window.download($0, $1, $2) }, 
		        filename.c_str(), contents.c_str(), contents.length());
	}
	catch (const std::runtime_error &err)
	{
		BadChoice *b = nullptr;
		b = new BadChoice(this, err.what());
		setModal(b);
		return;
	}

#endif
}
