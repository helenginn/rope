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

#ifndef __vagabond__Socket__
#define __vagabond__Socket__

#include <vagabond/core/Responder.h>

class Dictator;

class Socket : public HasResponder<Responder<Socket>>
{
public:
	Socket(Dictator *dictator, unsigned short port = 12345);

	bool operator()();
private:
	unsigned short _port = 12345;
	Dictator *_dictator = nullptr;

};

#endif
