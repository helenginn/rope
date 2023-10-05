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

#include <algorithm>
#include "ButtonResponder.h"
#include "KeyResponder.h"
#include <algorithm>

void ButtonResponder::setKeyResponder(KeyResponder *kr)
{
	_keyResponders.push_back(kr);
	kr->setButtonResponder(this);
}

void ButtonResponder::unsetKeyResponder(KeyResponder *kr)
{
	auto it = find(_keyResponders.begin(), _keyResponders.end(), kr);
	if (it != _keyResponders.end())
	{
		_keyResponders.erase(it);
		kr->setButtonResponder(nullptr);
	}
}
