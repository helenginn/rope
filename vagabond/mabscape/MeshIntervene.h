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

#ifndef __vagabond__MeshIntervene__
#define __vagabond__MeshIntervene__

#include <vagabond/gui/Display.h>
#include <atomic>
#include <functional>

struct Mab;
class Mesh;
class TextButton;
class Text;

/** Placeholder scene for stepping in on the mesh calculation of each antigen.
 *  "Even out" spaces the vertices evenly, "Shrink wrap" tightens them; "Next" is there only while they are
 *  settled, and goes on to the rest of the preparation. */
class MeshIntervene : public Display
{
public:
	MeshIntervene(Scene *prev, Mab &mab, 
	              const std::function<void()> &proceed);
	virtual ~MeshIntervene();

	virtual void setup();
private:
	void refreshButtons();
	void evenOut();
	void stop();
	void shrinkWrap();
	void work(const std::function<void(Mesh *)> &job, bool stoppable = true);
	void addBallSlider();
	void addCurvatureTick();
	void colourMeshes();
	void updateBallText();
	void ballCycle();
	void reset();
	bool settled();

	Mab &_mab;
	std::function<void()> _proceed;
	std::atomic<bool> _refining{false};
	std::atomic<bool> _stoppable{false};
	std::atomic<bool> _stopRequested{false};
	Mesh *_mesh{};
	Text *_ballText{};
	// the radius of the rolling ball (Angstroms) of the grow-shrink cycle
	std::atomic<double> _ballRadius{0};
	bool _colourCurvature{false};
	std::atomic<bool> _ballBusy{false};
	std::atomic<bool> _ballPending{false};
};

#endif
