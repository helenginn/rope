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

#include <vagabond/gui/elements/FloatingText.h>

#include <vagabond/core/Rule.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/MetadataGroup.h>
#include <vagabond/c4x/ClusterSVD.h>

#include "LineSeries.h"

#include <algorithm>

LineSeries::LineSeries(ClusterView *cv, const Rule &r) : SimplePolygon(),
_rule(r)
{
	setName("Line series");
	appendObject(cv);
	_indices.clear();
	_group = cv->data();

	_renderType = GL_LINES;
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");

	setup();
}

LineSeries::~LineSeries()
{
	
}

void LineSeries::addLabel(int idx)
{
	HasMetadata *hm = _group->object(idx);
	const Metadata::KeyValues data = hm->metadata();

	if (data.size())
	{
		std::string header = _rule.header();
		std::string value = data.at(header).text();
		std::string text = header + "/" + value;
		FloatingText *ft = new FloatingText(text);
		ft->setPosition(_vertices[idx].pos);
		addObject(ft);
	}
}

void LineSeries::addLabels()
{
	if (_group->objectCount() < 1)
	{
		return;
	}
	
	if (_indices.size() <= 1)
	{
		return;
	}

	if (_rule.label() & Rule::Start)
	{
		addLabel(_indices[0]);
	}

	if (_rule.label() & Rule::End)
	{
		addLabel(_indices.back());
	}

}

void LineSeries::setup()
{
	std::string header = _rule.header();
	std::vector<OrderedIndex> idxs;

	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		const Metadata::KeyValues data = _group->object(i)->metadata();
		
		if (data.count(header) && data.at(header).hasNumber())
		{
			float val = data.at(header).number();
			OrderedIndex oi{i, val};
			idxs.push_back(oi);
		}
	}
	
	std::sort(idxs.begin(), idxs.end());
	
	for (int i = 0; i < (int)idxs.size() - 1; i++)
	{
		addIndex(idxs[i].index);
		addIndex(idxs[i + 1].index);
	}
	
	addLabels();
	setColour(0., 0., 0.);
	setAlpha(1.);
}
