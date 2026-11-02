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
class TextEntry;
class GhostAb;
class GhostLine;
class Text;
class ResultsView;

class Playground : public Display
{
public:
	Playground(Scene *prev, Mab &mab);

	virtual void setup();
	void refine();

	virtual void interactedWithNothing(bool left, bool hover);
	virtual void keyPressEvent(SDL_Keycode pressed);
private:
	void showMesh(const Competition &competition);
	void showFiducials(const Competition &comp);
	void showMatrices();
	void setMatricesVisible(bool visible);
	void label(MatrixPlot *plot, float x, float y,
	           const std::string &text);
	// "g": type part of an antibody name to highlight matches as you go;
	// Enter centres on the antibody if only one matches
	void startSearch();
	void updateSearch(const std::string &term);
	void finishSearch();
	// clears an antibody's highlight unless it is a search match
	void unhighlight(AbWatch *aw);
	// keeps one antibody's label, highlights and mesh faces until a click
	// on empty space (or another antibody) replaces them
	void pin(AbWatch *aw);
	void clearSelection();
	// With an antibody clicked on, clicking the data or the model matrix
	// colours all the others by that antibody's row of it (grey where the
	// data has no entry); the same again takes it off
	enum ColourBy { Nothing, Data, Model };
	void colourByMatrix(ColourBy which);
	void applyColouring();
	// right-click menu: view how an antibody with fixed fiducial data sits
	// on the antigen for each orientation found, poor or otherwise
	void checkAlignmentMenu(const std::string &antibody);
	void showGhost(AbWatch *aw);
	void clearGhost();
	void showToggleFixed();
	void setOpaque(bool opaque);
	glm::vec3 surfaceLift(const glm::vec3 &p);
	void viewResults();
	void selectResults(const std::string &key,
	                   const std::vector<RefinedResult> &results);
	void refreshFixedStyles();
	// the antibodies go back to showing the live (refining) positions
	void resumeLive();
	void showHeat(AbWatch *aw);
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
	std::string _heatAntibody;
	TextEntry *_search{};
	AbWatch *_pinned{};
	ColourBy _colourBy{Nothing};
	GhostAb *_ghost{};
	GhostLine *_ghostLine{};
	std::vector<AbWatch *> _searchHits;
};

#endif
