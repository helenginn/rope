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

#ifndef __vagabond__canvas__
#define __vagabond__canvas__

#include <emscripten.h>

// These are JavaScript code in C/C++ file
EM_JS(int, canvas_get_width, (), {
  return document.getElementById("canvas").style.width;
});

EM_JS(int, canvas_get_height, (), {
  return document.getElementById("canvas").style.height;
});

#endif

