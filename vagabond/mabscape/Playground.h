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

#include <map>
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

struct Mab;
struct Fiducial;
class Box;
struct DisplayUnit;
class AtomGroup;

class Playground : public Display
{
public:
	Playground(Scene *prev, Mab &mab);
	virtual ~Playground();

	virtual void setup();
	void refine();

	virtual void interactedWithNothing(bool left, bool hover);

	// shift+drag a box around mesh vertices to pick them out (alt+drag to
	// take them out of the pick); right click to exclude them from where
	// refinements start, or to clear the exclusions
	virtual void sendSelection(float t, float l, float b, float r, 
	                           bool inverse);
	virtual void sendClickSelection(double x, double y, bool inverse);
	void meshMenu();
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
	// centres on an antibody by name, as if it had been searched for
	void showAntibody(const std::string &name);
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
	void alignmentMenu(AbWatch *clicked);
	// writes every antibody of a group of results to a PDB file (see
	// AntibodyPdb)
	void exportPdb(const std::string &key, const std::string &name,
	               const std::vector<RefinedResult> &results);
	void setupWatch(AbWatch *aw);

	// translucent bands over the row of the pinned antibody in the data and
	// model matrices (none if there is nothing pinned)
	void highlightRows();
	std::vector<Box *> _rowQuads;
	void syncAntibodies();

	// An antibody with a fixed fiducial shown glowing, where its alignment puts
	// it on the mesh. One orientation (the one for the copy clicked) of each
	// fiducial at a time, but any number of antibodies together, until a click
	// on empty space lets go of them all.
	void showAlignment(AbWatch *clicked);
	// the alignment window (see AlignmentView) for the antibody picked, at the
	// orientation nearest to the copy picked
	void checkAlignment();
	// which of a fiducial's orientations puts it nearest to a copy
	size_t nearestOrientation(AbWatch *aw, Fiducial *fid);
	void clearAlignments();
	// the footprints of the antibodies shown, dark grey on the mesh
	void markFootprints();
	struct ShownAlignment
	{
		Fiducial *fid{};
		DisplayUnit *unit{};
		AtomGroup *atoms{};
	};
	std::vector<ShownAlignment> _alignments;
	void showGhost(AbWatch *aw);
	void clearGhost();
	void showToggleFixed();
	void keepFixed();
	void askForRadius();
	void askForCurvatureScale();
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
	std::string _selectedKey; // the bucket of _selectedResults

	// What the dark grey faces are made of, so that they can be picked: the
	// clusters of the pinned antibody over the selected results, which is the
	// one the hovered copy belongs to, the dark vertices (with how many
	// points are on each) and the vertex each point of the clusters is on.
	Positions::Clusters _heatClusters;
	size_t _heatNearest{0};
	std::map<int, int> _heatMain;
	std::vector<int> _heatVertex;

	// Shift+drag over dark grey faces picks results: those which landed on the
	// vertices picked. They are what is shown and what "make group" takes.
	void narrowToSelection();
	void dropNarrowing(bool restoreAverage);
	bool _narrowed{false};
	std::vector<RefinedResult> _narrowedResults;
	TextEntry *_search{};
	AbWatch *_pinned{};
	ColourBy _colourBy{Nothing};
	GhostAb *_ghost{};
	GhostLine *_ghostLine{};
	std::vector<AbWatch *> _searchHits;
};

#endif
