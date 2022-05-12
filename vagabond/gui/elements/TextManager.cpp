// Vagabond
// Copyright (C) 2017-2018 Helen Ginn
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

#include "TextManager.h"
#include <vagabond/utils/FileReader.h>
#include "font.h"
#include <cstdlib>
#include <vector>

void TextManager::text_free(png_byte **pointer)
{
	free(*pointer);
}

void TextManager::text_malloc(png_byte **pointer, std::string text, int *width, int *height)
{
	if (!text.length())
	{
		return;
	}

	int squish = 10;
	int totalWidth = squish;
	int maxHeight = 0;
	int lines = 0;
	
	std::vector<std::string> bits = split(text, '\n');

	for (size_t j = 0; j < bits.size(); j++)
	{
		std::string &text = bits[j];

		int w = 0;
		for (size_t i = 0; i < text.length(); i++)
		{
			int charHeight = asciiDimensions[(int)text[i]][0];
			int charWidth = asciiDimensions[(int)text[i]][1];
			w += charWidth - squish;
			if (charHeight > maxHeight)
			{
				maxHeight = charHeight;
			}
		}
		
		if (w > totalWidth)
		{
			totalWidth = w;
		}
		
		lines++;
	}

	totalWidth += squish * 2;

	*width = totalWidth;
	*height = maxHeight * lines;

	*pointer = (png_byte *)calloc(*height * totalWidth * 4, 
	                              sizeof(png_byte));
	
	for (size_t i = 0; i < maxHeight * totalWidth; i += 4)
	{
		(*pointer)[i+0] = 255;
		(*pointer)[i+1] = 255;
		(*pointer)[i+2] = 255;
		(*pointer)[i+3] = 0;
	}

	int currentX = 0;
	int currentY = 0;

	for (size_t j = 0; j < bits.size(); j++)
	{
		std::string &text = bits[j];

		for (int i = 0; i < text.length(); i++)
		{
			char whichAscii = text[i];
			png_byte *chosenAscii = asciis[(int)whichAscii];
			int chosenHeight = asciiDimensions[(int)whichAscii][0];
			int chosenWidth = asciiDimensions[(int)whichAscii][1];

			for (int k = 0; k < chosenHeight; k++)
			{
				for (int j = 0; j < chosenWidth; j++)
				{
					int xPos = ((totalWidth * currentY) 
					            + currentX + j);
					int extra = totalWidth * k + xPos;
					int asciiPos = chosenWidth * k + j;
					for (size_t n = 0; n < 3; n++)
					{
						int curr = (*pointer)[extra * 4 + n];
						int sub = 255 - chosenAscii[asciiPos * 4 + n];

						if (curr - sub < 0)
						{
							(*pointer)[extra * 4 + n] = 0;
						}
						(*pointer)[extra * 4 + n] -= sub;
					}

					int add = chosenAscii[asciiPos * 4 + 3];
					(*pointer)[extra * 4 + 3] += add;
				}
			}

			currentX += chosenWidth - squish;
		}
	
		currentX = 0;
		currentY += maxHeight - 10;
	}
}
