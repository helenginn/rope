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

#ifndef __vagabond__CyclicView__
#define __vagabond__CyclicView__

#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/programs/Cyclic.h>
#include <vagabond/core/Responder.h>

class Icosahedron;
class Cyclic;

class CyclicView : public Renderable, public Responder<Cyclic>,
public HasResponder<Responder<CyclicView> >
{
public:
	CyclicView(Cyclic *cyclic = nullptr);
	~CyclicView();

	float score();
	void refine();
	void populate();
	void increment();
	void update()
	{
		updateBalls();
		updateCurve();
	}
	virtual void respond();
	
	bool isRotating()
	{
		return _rotate;
	}
	
	void toggleRotate()
	{
		_rotate = !_rotate;
	}
	
	Cyclic *cyclic()
	{
		return _cyclic;
	}
	
	std::string angles();
	std::string lengths();
private:
	void addCurve();
	void addBalls();
	void updateCurve();
	void updateBalls();

	bool _rotate = true;
	std::vector<Icosahedron *> _balls;
	Cyclic *_cyclic = nullptr;
};

#endif
