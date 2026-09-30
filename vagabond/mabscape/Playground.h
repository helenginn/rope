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

#ifndef __vagabond__Playground__
#define __vagabond__Playground__

#include "RefinedResult.h"
#include <vagabond/gui/Display.h>

struct Mab;
struct Competition;

class Mesh;
class AbWatch;
class PosToComp;
class Positions;
class MatrixPlot;
class Text;
class ResultsView;

class Playground : public Display
{
public:
	Playground(Scene *prev, Mab &mab);

	virtual void setup();
	void refine();

	virtual void interactedWithNothing(bool left, bool hover);
private:
	void showMesh(const Competition &competition);
	void showFiducials(const Competition &comp);
	void showMatrices();
	void setMatricesVisible(bool visible);
	void label(MatrixPlot *plot, float x, float y,
	           const std::string &text);
	void viewResults();
	void selectResults(const std::vector<RefinedResult> &results);
	void showHeat(const std::string &antibody);
	void clearHeat();
	void saveResults();
	void showCompetitionData();

	Mab &_mab;

	Positions *_positions{};
	Mesh *_mesh{};
	std::vector<AbWatch *> _watches;

	PosToComp *_model{};
	MatrixPlot *_mpModel{};
	MatrixPlot *_mpData{};
	std::vector<Text *> _matrixLabels;
	bool _matricesVisible = true;
	ResultsView *_rv{};
	std::vector<RefinedResult> _selectedResults;
	std::vector<size_t> _selectedOps; // symmetry alignment of each
	std::string _heatAntibody;
};

#endif
