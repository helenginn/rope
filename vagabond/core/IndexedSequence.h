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

#ifndef __vagabond__IndexedSequence__
#define __vagabond__IndexedSequence__

class Residue;

/** \class IndexedSequence
 *  for providing necessary information for SequenceView display */

class IndexedSequence
{
public:
	virtual ~IndexedSequence() {}

	/** for display in SequenceView, return 1 row for a simple sequence */
	virtual size_t rowCount() = 0;

	/** for display in SequenceView, return how many entries (columns) in total */
	virtual size_t entryCount() = 0;
	
	/** get the residue for a given row/entry
	 * @return pointer to residue or nullptr if not applicable */
	virtual Residue *residue(int row, int entry) = 0;

	/** @return is there is a non-null residue associated with this row/entry */
	virtual bool hasResidue(int row, int entry) = 0;

	/** @return display string to insert into SequenceView for this row/entry */
	virtual std::string displayString(int row, int entry) = 0;
private:

};

#endif
