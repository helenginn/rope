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

#ifndef __vagabond__ClusterView__
#define __vagabond__ClusterView__

#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/c4x/Cluster.h>

#define POINT_TYPE_COUNT 8

class Rule;
class HasMetadata;
class FloatingText;
class MetadataGroup;
class ConfSpaceView;

class ClusterView : public IndexResponder
{
public:
	ClusterView();
	~ClusterView();

	void setCluster(Cluster<MetadataGroup> *cx);
	
	void refresh();
	
	void clearRules()
	{
		_members.clear();
	}

	void setConfSpaceView(ConfSpaceView *csv)
	{
		_confSpaceView = csv;
	}
	
	std::vector<HasMetadata *> &membersForRule(Rule *r)
	{
		return _members[r];
	}
	
	Cluster<MetadataGroup> *cluster()
	{
		return _cx;
	}
	
	const size_t pointTypeCount() const
	{
		return POINT_TYPE_COUNT;
	}

	void addPoint(glm::vec3 pos, int pointType);
	void setPointType(int idx, int type);
	virtual void makePoints();
	virtual void click(bool left = true);

	void applyRule(const Rule &r);

	void render(SnowGL *gl);
	void prioritiseMetadata(std::string key);
	void reset();
	
	virtual void interacted(int idx, bool hover, bool left = true);
	virtual size_t requestedIndices()
	{
		return _vertices.size();
	}

	virtual void reindex();

	virtual bool mouseOver();
	virtual void unMouseOver();
	
	void reselect();
protected:
	virtual void extraUniforms();
	void customiseTexture(Snow::Vertex &vert);
	float _size = 40;
private:
	void applyVaryColour(const Rule &r);
	void applyChangeIcon(const Rule &r);

	Cluster<MetadataGroup> *_cx = nullptr;
	FloatingText *_text = nullptr;
	ConfSpaceView *_confSpaceView = nullptr;

	std::map<const Rule *, std::vector<HasMetadata *>> _members;
};

#endif
