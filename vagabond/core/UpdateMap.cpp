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

#include <vagabond/utils/maths.h>
#include "UpdateMap.h"
#include "Scaler.h"
#include "RFactor.h"
#include "Recombine.h"
#include "files/MtzFile.h"

UpdateMap::UpdateMap(Diffraction *const &data, Diffraction *const &model)
: _data(data), _model(model)
{
	if (_data->nn() != _model->nn())
	{
		std::cout << "Warning: mismatched n for data/model!" << std::endl;
	}
}

std::vector<WriteColumn> UpdateMap::writeAutoOpeningMtz(Diffraction *const &toWrite,
                                                        Diffraction *const &diff)
{
	/* writing an MTZ which should be auto-openable in Coot */

	std::vector<WriteColumn> columns;

	columns.push_back(WriteColumn("FP", "F", 
	                              [this](int i, int j, int k)
	                              {
		                             return _data->element(i, j, k).amplitude();
	                              }));

	columns.push_back(WriteColumn("FC_ALL", "F", 
	                              [this](int i, int j, int k)
	                              {
		                             return _model->element(i, j, k).amplitude();
	                              }));

	columns.push_back(WriteColumn("FWT", "F", 
	                              [toWrite](int i, int j, int k)
	                              {
		                             return toWrite->element(i, j, k).amplitude();
	                              }));

	columns.push_back(WriteColumn("PHWT", "P", 
	                              [this](int i, int j, int k)
	                              {
		                             return _model->element(i, j, k).phase();
	                              }));

	if (diff)
	{
		columns.push_back
		(WriteColumn("DELFWT", "F", 
		             [this, diff](int i, int j, int k)
		             {
			            return (diff->element(i, j, k).amplitude() -
			            		_model->element(i, j, k).amplitude());
		             }));

		columns.push_back(WriteColumn("PHDELWT", "P", 
		                              [this](int i, int j, int k)
		                              {
			                             return _model->element(i, j, k).phase();
		                              }));
	}
	
	return columns;
}

Diffraction *UpdateMap::operator()()
{
	float maxRes = _data->maxResolution();

	std::vector<float> bins;
	generateResolutionBins(0, maxRes, 20, bins);

	Scaler scale(_data, _model, bins);
	scale();
	
	RFactor rfac(_data, _model, 20);
	float all = rfac();
	std::cout << "Overall R factor: " << all * 100 << "%" << std::endl;
	
	Recombine weighted_map(_data, _model, Recombine::Weighted);
	Diffraction *weighted = weighted_map();

	Recombine difference_map(_data, _model, Recombine::Difference);
	Diffraction *differences = difference_map();
	
	std::vector<WriteColumn> writes = writeAutoOpeningMtz(weighted, differences);
	
	MtzFile file(weighted);
	file.write_to_file("recombined.mtz", maxRes, writes);

	return weighted;
}
