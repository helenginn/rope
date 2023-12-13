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

#include "ImplicitBHandler.h"
#include "grids/AnisoMap.h"
#include "grids/AtomMap.h"

ImplicitBHandler::ImplicitBHandler(int resources, const AtomMap *calc_template)
{
	_threads = resources;
	_template = calc_template;

}

void ImplicitBHandler::setup()
{
	for (int i = 0; i < _threads; i++)
	{
		AnisoMap *anisou = new AnisoMap(*_template);
		_anisoPool.pushObject(anisou);
	}
}

AnisoMap *ImplicitBHandler::acquireAnisoMapIfAvailable()
{
	AnisoMap *anisou;
	_anisoPool.acquireObjectIfAvailable(anisou);

	return anisou;
}

void ImplicitBHandler::returnAnisoMap(AnisoMap *anisou)
{
	_anisoPool.pushObject(anisou);
}

