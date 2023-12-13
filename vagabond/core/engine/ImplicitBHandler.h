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

#ifndef __vagabond__ImplicitBHandler__
#define __vagabond__ImplicitBHandler__

#include <thread>
#include "engine/Handler.h"
//#include "engine/ElementTypes.h"

class AtomMap;
class AnisoMap;

class ImplicitBHandler : public Handler
{
public:
	ImplicitBHandler(int resources, const AtomMap *calc_template);

	void setup();

	AnisoMap *acquireAnisoMapIfAvailable();
	void returnAnisoMap(AnisoMap *aniso);
private:
	const AtomMap *_template = nullptr;
	
	Pool<AnisoMap *> _anisoPool;

	int _threads = 1;

};

#endif
