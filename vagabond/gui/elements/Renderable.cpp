// Copyright (C) 2021 Helen Ginn

#include "Renderable.h"
#include "SnowGL.h"
#include "Window.h"
#include "config/config.h"
#include "Library.h"
#include <vagabond/utils/FileReader.h>
#include <float.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>
#include <vagabond/core/matrix_functions.h>

void Renderable::addToVertexArray(glm::vec3 add, std::vector<Vertex> *vs)
{
	for (size_t i = 0; i < vs->size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			(*vs)[i].pos[j] += add[j];
		}
	}
}

void Renderable::addToVertices(glm::vec3 add)
{
	if (add.x != add.x || add.y != add.y || add.z != add.z)
	{
		return;
	}

	addToVertexArray(add, &_vertices);
	addToVertexArray(add, &_unselectedVertices);

	positionChanged();
}

void Renderable::deleteOnMainThread()
{
	Window::setDelete(this);
}

Renderable::~Renderable()
{
	delete _shaderGets;

	deleteVBOBuffers();
	deletePrograms();
	deleteTextures();

	_vString = "";
	_fString = "";
	_gString = "";
	_unselectedVertices.clear();
	_vertices.clear();
	_indices.clear();
	std::vector<Vertex>().swap(_unselectedVertices);
	std::vector<Vertex>().swap(_vertices);
	std::vector<GLuint>().swap(_indices);
	
	for (size_t i = 0; i < _objects.size(); i++)
	{
		if (_objects[i] == _hover)
		{
			removeObject(_hover);
			delete _hover;
			_hover = nullptr;
		}
	}
}


void Renderable::deletePrograms()
{
	Library::getLibrary()->endProgram(_vString, _fString);
	_program = 0;
}

void Renderable::initialisePrograms()
{
	if (_program != 0)
	{
		return;
	}

	bool old = true;
	_program = Library::getLibrary()->getProgram(_vString, _vFile, 
	                                             _fString, _fFile, old);
	
	if (!old)
	{
		_shaderGets->extraVariables();
		Library::getLibrary()->checkProgram(_program);
	}
}

void Renderable::deleteTextures()
{
	Library::getLibrary()->dropTexture(_texid);
	_texid = 0;
}

void Renderable::unbindVBOBuffers()
{
	glBindVertexArray(0);
	checkErrors("vbo buffer unbinding");
}

void Renderable::deleteVBOBuffers()
{
	if (_program != 0 && _vaoMap.count(_program))
	{
		GLuint vao = _vaoMap[_program];
		glDeleteVertexArrays(1, &vao);
		_vaoMap.erase(_program);
	}
}

void Renderable::rebindVBOBuffers()
{
	int vao = vaoForContext();
	glBindVertexArray(vao);

	bool error = checkErrors("vertex array rebinding");
	
	if (error)
	{
		std::cout << "Vao: " << vao << std::endl;
	}

	checkErrors("vertex array attribute reenabling");
	
	if (!_setupBuffers)
	{
		setupVBOBuffers();
	}
}

int Renderable::vaoForContext()
{
	if (_vaoMap.count(_program) && !_forceVertices && !_forceIndices)
	{
		GLuint vao = _vaoMap[_program];
		return vao;
	}
	else if (_vaoMap.count(_program) && (_forceVertices || _forceIndices))
	{
		if (!tryLockMutex())
		{
			GLuint vao = _vaoMap[_program];
			return vao;
		}

		if (_forceVertices)
		{
			rebufferVertexData();
			_forceVertices = false;
		}
		if (_forceIndices)
		{
			rebufferIndexData();
			_forceIndices = false;
		}

		GLuint vao = _vaoMap[_program];
		unlockMutex();
		return vao;
	}
	
	lockMutex();
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	_vaoMap[_program] = vao;
	setupVBOBuffers();
	unlockMutex();
	
	return vao;
}

void Renderable::rebufferIndexData()
{
	_shaderGets->rebufferIndexData();
	
	if (_gl)
	{
		_gl->viewChanged();
	}
}

void Renderable::rebufferVertexData()
{
	_shaderGets->rebufferVertexData();
	
	if (_gl)
	{
		_gl->viewChanged();
	}
}

void Renderable::setupVBOBuffers()
{
	_shaderGets->setupVBOBuffers();
	_setupBuffers = true;
	
	if (_gl)
	{
		_gl->viewChanged();
	}
}

glm::mat4x4 Renderable::getModel()
{
	return _myModel * _gl->getModel();
}

void Renderable::render(SnowGL *sender)
{
	if (_disabled)
	{
		return;
	}
	
//	if (!tryLockMutex())
	{
//		return;
	}

	_gl = sender;
	
	if (_program == 0)
	{
		initialisePrograms();
	}

	if (_program == 0)
	{
		return;
	}

	glUseProgram(_program);

	checkErrors("use program");
	rebindVBOBuffers();
	checkErrors("rebinding program");

	_model = getModel();
	_uModel = glGetUniformLocation(_program, "model");
	_glModel = glm::transpose(model());
	glUniformMatrix4fv(_uModel, 1, GL_FALSE, &_model[0][0]);
	checkErrors("rebinding model");

	_proj = sender->getProjection();
	_uProj = glGetUniformLocation(_program, "projection");
	_glProj = glm::transpose(projection());
	glUniformMatrix4fv(_uProj, 1, GL_FALSE, &_proj[0][0]);
	checkErrors("rebinding projection");

	extraUniforms();

	checkErrors("rebinding extras");

	if (hasTexture())
	{
		GLuint which = GL_TEXTURE_2D;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(which, _texid);
		GLuint uTex = glGetUniformLocation(_program, "pic_tex");
		glUniform1i(uTex, 0);
	}

	if (_setupBuffers)
	{
		checkErrors("before drawing elements");
		glDrawElements(_renderType, indexCount(), GL_UNSIGNED_INT, 0);
		checkErrors("drawing elements");
	}


	glUseProgram(0);
	unbindVBOBuffers();

	_renderCount++;

	for (size_t i = 0; i < objectCount(); i++)
	{
		_objects[i]->render(_gl);
	}
}

void Renderable::setAlpha(double alpha)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].color[3] = alpha;
	}

	for (size_t i = 0; i < _unselectedVertices.size(); i++)
	{
		_unselectedVertices[i].color[3] = alpha;
	}
}

void Renderable::recolour(double red, double green, double blue,
                          std::vector<Vertex> *vs)
{
	if (vs == NULL)
	{
		vs = &_vertices;
	}
	for (size_t i = 0; i < vs->size(); i++)
	{
		(*vs)[i].color[0] = red;
		(*vs)[i].color[1] = green;
		(*vs)[i].color[2] = blue;
	}
}

void Renderable::resize_around_centre(double scale, glm::vec3 centre, 
                                      bool unselected, bool realign)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &pos = _vertices[i].pos;
		pos -= centre;
		pos *= scale;
		pos += centre;
	}
	
	if (realign)
	{
		_x -= centre.x; _x *= scale; _x += centre.x;
		_y -= centre.y; _y *= scale; _y += centre.y;
	}
	
	rebufferVertexData();
	
	if (!unselected)
	{
		resized(scale);
	}
	else
	{
		for (size_t i = 0; i < _unselectedVertices.size(); i++)
		{
			glm::vec3 &pos = _unselectedVertices[i].pos;
			pos -= centre;
			pos *= scale;
			pos += centre;
		}
	}

	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->resize_around_centre(scale, centre, unselected, realign);
	}
}

void Renderable::resize(double scale, bool unselected, bool realign)
{
	bool dis = _hover ? _hover->isDisabled() : true;
	
	if (!dis)
	{
		_hover->setDisabled(true);
	}

	glm::vec3 centre = centroid();
	resize_around_centre(scale, centre, unselected, realign);
	
	if (!dis)
	{
		_hover->setDisabled(false);
	}
}

double Renderable::averageRadius()
{
	glm::vec3 centre = centroid();
	double all = 0;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &pos = _vertices[i].pos;
		pos -= centre;
		double length = glm::length(pos);
		all += length;
	}
	
	all /= (double)_vertices.size();
	return all;
}


double Renderable::envelopeRadius()
{
	double longest = 0;

	glm::vec3 centre = centroid();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &pos = _vertices[i].pos;
		pos -= centre;
		double sqlength = glm::dot(pos, pos);
		
		if (sqlength > longest)
		{
			longest = sqlength;
		}
	}
	
	return sqrt(longest);
}

void Renderable::fixCentroid(glm::vec3 centre)
{
	glm::vec3 current = centroid();
	glm::vec3 diff = centre - current;
	addToVertices(diff);
}

void Renderable::setDisabled(bool dis)
{
	_disabled = dis;
}

glm::vec3 Renderable::centroid()
{
	glm::vec3 sum = glm::vec3(0, 0, 0);
	float count = 0;
	
	if (_vertices.size() == 0)
	{
		for (size_t i = 0; i < objectCount(); i++)
		{
			sum += object(i)->centroid();
			count++;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].pos[0] != _vertices[i].pos[0])
		{
			continue;
		}

		sum += _vertices[i].pos;
		count++;
	}
	
	sum /= count;
	
	return sum;
}

Vertex &Renderable::addVertex(glm::vec3 v, std::vector<Vertex> *vec)
{
	addVertex(v.x, v.y, v.z, vec);
	return _vertices.back();
}

Vertex &Renderable::addVertex(float v1, float v2, float v3,
                           std::vector<Vertex> *vec)
{
	Vertex v;
	memset(&v, 0, sizeof(Vertex));

	v.color[2] = 0.;
	v.color[3] = 0.;
	v.pos[0] = v1;
	v.pos[1] = v2;
	v.pos[2] = v3;

	if (vec == NULL)
	{
		_vertices.push_back(v);
		return _vertices.back();
	}
	else
	{
		vec->push_back(v);
		return vec->back();
	}
}

void Renderable::addIndicesToEnd(std::vector<GLuint> &indices)
{
	_indices.reserve(_indices.size() + indices.size());
	int offset = _vertices.size();

	for (const GLuint &idx : indices)
	{
		_indices.push_back(idx + offset);
	}
}

void Renderable::addIndices(GLuint i1, GLuint i2, GLuint i3)
{
	addIndex(i1);
	addIndex(i2);
	addIndex(i3);
}

void Renderable::addIndex(std::vector<GLuint> &indices,
                          std::vector<Snow::Vertex> &vertices, GLint i)
{
	if (i < 0)
	{
		indices.push_back(vertices.size() + i);
		return;
	}

	indices.push_back(i);
}

void Renderable::addIndex(GLint i)
{
	addIndex(_indices, _vertices, i);
}

bool Renderable::index_behind_index(IndexTrio one, IndexTrio two)
{
	return (one.z < two.z);
}

bool Renderable::index_in_front_of_index(IndexTrio one, IndexTrio two)
{
	return (one.z > two.z);
}

void Renderable::reorderIndices()
{
	if (_renderType == GL_LINES || _renderType == GL_POINTS)
	{
		return;
	}

	_temp.resize(_indices.size() / 3);
	
	int count = 0;
	for (size_t i = 0; i < _indices.size(); i+=3)
	{
		int n = _indices[i];
		glm::vec3 tmpVec = _vertices[n].pos;
		n = _indices[i + 1];
		glm::vec3 &tmpVec1 = _vertices[n].pos;
		n = _indices[i + 2];
		glm::vec3 &tmpVec2 = _vertices[n].pos;
		tmpVec += tmpVec1;
		tmpVec += tmpVec2;
		tmpVec = _model * glm::vec4(tmpVec, 1.);
		_temp[count].z = tmpVec.z;
		_temp[count].index[0] = _indices[i];
		_temp[count].index[1] = _indices[i + 1];
		_temp[count].index[2] = _indices[i + 2];
		count++;
	}
	
	if (_backToFront)
	{
		std::sort(_temp.begin(), _temp.end(), index_behind_index);
	}
	else
	{
		std::sort(_temp.begin(), _temp.end(), index_in_front_of_index);
	}

	count = 0;

	for (size_t i = 0; i < _temp.size(); i++)
	{
		_indices[count + 0] = _temp[i].index[0];
		_indices[count + 1] = _temp[i].index[1];
		_indices[count + 2] = _temp[i].index[2];
		count += 3;
	}

	rebufferVertexData();
	rebufferIndexData();
}

bool nPolygon(glm::vec3 point, glm::vec3 *vs, int n)
{
	bool c = false;
	int a = (n + 1) % 3;
	int b = (n + 2) % 3;
	
	for (int i = 0, j = 2; i < 3; j = i++) 
	{
		if (((vs[i][b] > point[b]) != (vs[j][b] > point[b]))
		    && (point[a] < (vs[j][a] - vs[i][a]) * (point[b] - vs[i][b])
		    / (vs[j][b] - vs[i][b]) + vs[i][a])) 
		{
			c = !c;
		}
	}

	return c;
}

bool Renderable::intersectsRay(double x, double y, double *z)
{
	RaySearch type = _searchType;
	
	if (type == Default && _renderType == GL_TRIANGLES)
	{
		type = SearchPlane;
	}
	else if (type == Default && _renderType == GL_POINTS)
	{
		type = Point;
	}

	if (type == SearchPlane)
	{
		return intersectsPolygon(x, y, z);
	}
	else if (type == Point)
	{
		return (intersectsPoint(x, y, z) >= 0);
	}
	
	return false;
}

int Renderable::intersectsPoint(double x, double y, double *z)
{
	glm::vec3 target = glm::vec3(x, y, 0);
	const float tol = 0.01;
	int idx = -1;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec4 pos = glm::vec4(_vertices[i].pos, 1.);
		
		if (_usesProj)
		{
			pos = _proj * _model * pos;
			pos /= pos[3];
		}
		
		if (pos.x < x - tol || pos.x >= x + tol ||
		    pos.y < y - tol || pos.y >= y + tol)
		{
			continue;
		}

		if (pos.z > *z)
		{
			*z = pos.z;
			idx = i;
		}
	}

	_currVertex = idx;
	return idx;
}

bool Renderable::intersectsPolygon(double x, double y, double *z)
{
	glm::vec3 target = glm::vec3(x, y, 0);
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		glm::vec3 projs[3];

		for (int j = 0; j < 3; j++)
		{
			projs[j] = _myModel * glm::vec4(_vertices[_indices[i+j]].pos, 1.);
		}

		bool passes = nPolygon(target, projs, 2);

		if (passes)
		{
			*z = projs[0].z;
			return true;
		}
	}
	
	return false;
}

bool Renderable::polygonIncludes(glm::vec3 point, GLuint *trio)
{
	glm::vec3 vs[3];
	vs[0] = _vertices[trio[0]].pos;
	vs[1] = _vertices[trio[1]].pos;
	vs[2] = _vertices[trio[2]].pos;

	double xmin = std::min(std::min(vs[0].x, vs[1].x), vs[2].x);
	double ymin = std::min(std::min(vs[0].y, vs[1].y), vs[2].y);
	double zmin = std::min(std::min(vs[0].z, vs[1].z), vs[2].z);
	
	double xmax = std::max(std::max(vs[0].x, vs[1].x), vs[2].x);
	double ymax = std::max(std::max(vs[0].y, vs[1].y), vs[2].y);
	double zmax = std::max(std::max(vs[0].z, vs[1].z), vs[2].z);
	
	double zdiff = zmax - zmin;
	double ydiff = ymax - ymin;
	double xdiff = xmax - xmin;
	
	if (zdiff < ydiff && zdiff < xdiff)
	{
		return nPolygon(point, (glm::vec3 *)vs, 2);
	}
	else if (ydiff < zdiff && ydiff < xdiff)
	{
		return nPolygon(point, (glm::vec3 *)vs, 1);
	}
	else
	{
		return nPolygon(point, (glm::vec3 *)vs, 0);
	}
}

void Renderable::boundaries(glm::vec3 *min, glm::vec3 *max)
{
	*min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	*max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex &v = _vertices[i];
		
		for (size_t j = 0; j < 3; j++)
		{
			if (v.pos[j] < (*min)[j])
			{
				(*min)[j]= v.pos[j];
			}

			if (v.pos[j] > (*max)[j])
			{
				(*max)[j] = v.pos[j];
			}
		}
	}
}

void Renderable::changeMidPoint(double x, double y)
{
	glm::vec4 pos = glm::vec4(centroid(), 1.0);

	glm::vec4 model = _proj * _model * pos;
	
	double newx = model[3] * x / _proj[0][0];
	double newy = model[3] * y / _proj[1][1];
	glm::vec3 move = glm::vec3(newx - model.x, newy - model.y, 0);

	glm::mat3x3 rot = glm::mat3x3(_model);

	glm::vec3 newpos = rot * move;

	addToVertices(newpos);
	
}

void Renderable::setSelectable(bool selectable)
{
	if (selectable)
	{
		_unselectedVertices = _vertices;
	}
	else
	{
		_unselectedVertices.clear();
	}

	_selectable = selectable;
}

void Renderable::setHighlighted(bool selected)
{
	if (!_selected && selected)
	{
		_unselectedVertices = _vertices;
		recolour(0.2, 0.2, 0.2);
		resize(_selectionResize);
	}
	
	if (_selected && !selected)
	{
		_vertices = _unselectedVertices;
	}

	checkErrors("before setting highlighted");
	rebufferVertexData();
	checkErrors("after setting highlighted");
	_selected = selected;
}

void Renderable::changeToLines()
{
	if (_renderType == GL_LINES)
	{
		return;
	}

	_renderType = GL_LINES;
	std::vector<GLuint> is = _indices;
	_indices.clear();

	for (size_t i = 0; i < is.size(); i += 3)
	{
		addIndices(is[i], is[i + 1], is[i + 2]);
		addIndices(is[i], is[i + 1], is[i + 2]);
	}
}

void Renderable::changeToTriangles()
{
	if (_renderType == GL_TRIANGLES)
	{
		return;
	}

	_renderType = GL_TRIANGLES;
	std::vector<GLuint> is = _indices;
	_indices.clear();

	for (size_t i = 0; i < is.size(); i += 6)
	{
		addIndices(is[i], is[i + 1], is[i + 2]);
	}

}

void Renderable::copyFrom(Renderable *s)
{
	_vertices = s->_vertices;
	_indices = s->_indices;
	_unselectedVertices = s->_unselectedVertices;
}

void Renderable::changeVertexShader(std::string v)
{
	_vString = v;
	deletePrograms();
}

void Renderable::changeFragmentShader(std::string f)
{
	_fString = f;
	deletePrograms();
}

void Renderable::setPosition(glm::vec3 pos)
{
	glm::vec3 p = centroid();

	glm::vec3 diff = pos - p;
	addToVertices(diff);
	
	positionChanged();
}

void Renderable::rotateByMatrix(glm::mat3x3 m)
{
	rotateRound(m, glm::vec3(0, 0, 0));
}

void Renderable::rotateRoundCentre(glm::mat3x3 m)
{
	rotateRound(m, centroid());
}

void Renderable::rotateRound(glm::mat3x3 &rot, glm::vec3 c)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &v = _vertices[i].pos;
		glm::vec3 &n = _vertices[i].normal;
		v -= c;
		v = rot * v;
		n = rot * n;
		v += c;
	}
}

void Renderable::setFragmentShaderFile(std::string file)
{
	setShaderFile(file, &_fString, &_fFile);
}

void Renderable::setVertexShaderFile(std::string file)
{
	setShaderFile(file, &_vString, &_vFile);
}

void Renderable::setShaderFile(std::string file, std::string *location,
                            std::string *fLoc)
{
	if (file.length() == 0)
	{
		return;
	}

	std::string path = file;
	Library::correctFilename(path);

	*fLoc = file;
	try
	{
		file = get_file_contents(path);
		*location = file;
	}
	catch (std::runtime_error err)
	{
		std::cout << "Could not open " << err.what() << std::endl;
		return;
	}
}

void Renderable::setImage(std::string imagename, bool wrap)
{
	GLuint tex = Library::getLibrary()->getTexture(imagename, nullptr,
	                                               nullptr, wrap);
	_texid = tex;
	_texture = imagename;
}

void Renderable::maximalDim(double *min, double *max, int dim)
{
	if (isDisabled())
	{
		return;
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		float &f = _vertices[i].pos[dim];
		
		if (f != f)
		{
			continue;
		}

		if (f > *max)
		{
			*max = f;
		}

		if (f < *min)
		{
			*min = f;
		}
	}
	
	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->maximalDim(min, max, dim);
	}
}

double Renderable::maximalHeight()
{
	double max = -FLT_MAX;
	double min = FLT_MAX;
	
	maximalDim(&min, &max, 1);
	
	return max - min;
}

double Renderable::maximalWidth()
{
	double max = -FLT_MAX;
	double min = FLT_MAX;
	
	maximalDim(&min, &max, 0);
	
	return max - min;
}

void Renderable::appendObject(Renderable *object)
{
	lockMutex();
	int add = _vertices.size();
	_vertices.reserve(_vertices.size() + object->vertexCount());
	_indices.reserve(_indices.size() + object->indexCount());
	
	_vertices.insert(_vertices.end(), object->_vertices.begin(),
	                 object->_vertices.end());
	
	for (size_t i = 0; i < object->_indices.size(); i++)
	{
		long idx = object->_indices[i] + add;
		_indices.push_back(idx);
	}
	unlockMutex();
}

glm::vec3 Renderable::rayTraceToPlane(glm::vec3 point, GLuint *trio, 
                                      glm::vec3 dir, bool *backwards)
{
	glm::vec3 vs[3];
	vs[0] = _vertices[trio[0]].pos; 
	vs[1] = _vertices[trio[1]].pos;
	vs[2] = _vertices[trio[2]].pos;

	glm::vec3 diff1 = vs[1] - vs[0];
	glm::vec3 diff2 = vs[2] - vs[0];
	glm::vec3 cross = glm::cross(diff1, diff2);
	cross = glm::normalize(cross); 
	
	double denom = glm::dot(dir, cross);
	glm::vec3 subtract = vs[0] - point;
	double nom = glm::dot(subtract, cross);
	double d = nom / denom;
	
	dir *= d;
	point += dir;
	
	*backwards = (d < 0);
	return point;
}

double Renderable::intersects(glm::vec3 pos, glm::vec3 dir)
{
	double closest = FLT_MAX;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		bool back = false;
		glm::vec3 intersect = rayTraceToPlane(pos, &_indices[i], dir, &back);
		
		if (back)
		{
			continue;
		}

		bool passes = polygonIncludes(intersect, &_indices[i]);

		if (passes)
		{
			glm::vec3 a = _vertices[_indices[i+2]].pos;
			glm::vec3 b = _vertices[_indices[i+1]].pos;
			glm::vec3 c = _vertices[_indices[i]].pos;
			c += a + b;
			c /= 3.;
			c -= pos;

			double l = glm::length(c);
			
			if (l < closest)
			{
				closest = l;
			}
		}
	}
	
	if (closest == FLT_MAX)
	{
		return -1;
	}
	
	return closest;
}

glm::mat4x4 Renderable::model()
{
	if (!_usesProj)
	{
		return glm::mat4(1.);
	}
	
	return _model;
}

glm::mat4x4 Renderable::projection()
{
	if (!_usesProj)
	{
		return glm::mat4(1.);
	}
	
	return _proj;
}

void Renderable::setUsesProjection(bool usesProj)
{
	_usesProj = usesProj;
	if (!_usesProj)
	{
		_proj = glm::mat4(1.);
		_model = glm::mat4(1.);
	}
}

void Renderable::calculateNormals()
{
	lockMutex();
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].normal = glm::vec3(0.);
	}
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		glm::vec3 &pos1 = _vertices[_indices[i+0]].pos;
		glm::vec3 &pos2 = _vertices[_indices[i+1]].pos;
		glm::vec3 &pos3 = _vertices[_indices[i+2]].pos;

		glm::vec3 diff31 = pos3 - pos1;
		glm::vec3 diff21 = pos2 - pos1;

		glm::vec3 cross = glm::cross(diff31, diff21);
		cross = glm::normalize(cross);
		
		if (!is_glm_vec_sane(cross))
		{
			continue;
		}
		
		/* Normals */					
		for (int j = 0; j < 3; j++)
		{
			_vertices[_indices[i + j]].normal += cross;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &norm = _vertices[i].normal;
		norm = glm::normalize(norm);
	}
	unlockMutex();
}

void Renderable::triangulate()
{
	if (_renderType == GL_LINES)
	{
		return;
	}
	
	lockMutex();

	std::map<std::pair<GLuint, GLuint>, GLuint> lines;
	std::map<std::pair<GLuint, GLuint>, GLuint>::iterator linesit;

	std::map<GLuint, std::map<GLuint, GLuint> > newVs;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			int ij = i + (j % 3);
			int ij1 = i + ((j + 1) % 3);
			int min = std::min(_indices[ij], _indices[ij1]);
			int max = std::max(_indices[ij], _indices[ij1]);
			
			std::pair<GLuint, GLuint> pair = std::make_pair(min, max);

			lines[pair] = 1;
		}
	}

	for (linesit = lines.begin(); linesit != lines.end(); linesit++)
	{
		std::pair<GLuint, GLuint> pair = linesit->first;
		GLuint front = pair.first;
		GLuint back = pair.second;
		
		Vertex v = _vertices[front]; 
		glm::vec3 &v1 = v.pos;
		glm::vec3 v2 = _vertices[back].pos;
		double x = (v.tex[0] + _vertices[back].tex[0]) / 2;
		double y = (v.tex[1] + _vertices[back].tex[1]) / 2;
		
		v1 += v2;
		v1 *= 0.5;
		
		v.tex[0] = x;
		v.tex[1] = y;
		
		_vertices.push_back(v);
		newVs[front][back] = _vertices.size() - 1;
		newVs[back][front] = _vertices.size() - 1;
	}
	
	size_t idxSize = _indices.size();
	for (size_t i = 0; i < idxSize; i += 3)
	{
		GLuint i1 = _indices[i];
		GLuint i2 = _indices[i + 1];
		GLuint i3 = _indices[i + 2];
		
		GLuint i12 = newVs[i1][i2];
		GLuint i13 = newVs[i1][i3];
		GLuint i23 = newVs[i2][i3];
		
		_indices[i+1] = i12;
		_indices[i+2] = i13;
		
		addIndices(i13, i12, i23);
		addIndices(i13, i23, i3);
		addIndices(i23, i12, i2);
	}
	
	unlockMutex();
	
	calculateNormals();
}

void Renderable::setArbitrary(double x, double y, Alignment a)
{
	double dx = x - _x;
	double dy = y - _y;

	_x = x; _y = y;
	double xf = 2 * x - 1;
	double yf = 2 * y - 1;

	double widthmult = (a & Left ? 1 : (a & Right ? -1 : 0));
	xf += widthmult * maximalWidth() / 2;

	double heightmult = (a & Top ? 1 : (a & Bottom ? -1 : 0));
	yf += heightmult * maximalHeight() / 2;
	
	if (xf != xf) xf = 0;
	if (yf != yf) yf = 0;

	setPosition(glm::vec3(xf, -yf, 0));
	
	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->addAlign(dx, dy);
	}

	_align = a;
	setHover(_hover);

}

void Renderable::setCentre(double x, double y)
{
	setArbitrary(x, y, Centre);
}

void Renderable::setLeft(double x, double y)
{
	setArbitrary(x, y, Left);
}

void Renderable::setRight(double x, double y)
{
	setArbitrary(x, y, Right);
}

void Renderable::realign(double x, double y)
{
	_x = x;
	_y = y;
	realign();
}

void Renderable::realign()
{
	switch (_align)
	{
		case Left:
		setLeft(_x, _y);
		break;

		case Right:
		setRight(_x, _y);
		break;

		case Centre:
		setCentre(_x, _y);
		break;
		
		default:
		break;
	}

	if (_hover)
	{
		_hover->realign();
	}
}

void Renderable::setExtra(glm::vec4 pos)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].extra = pos;
	}
}

void Renderable::setHover(Renderable *hover)
{
	if (hover == nullptr && _hover != nullptr)
	{
		removeObject(_hover);
		_hover = nullptr;
		setSelectable(false);
		return;
	}
	
	if (hover == nullptr)
	{
		return;
	}

	if (_hover != hover)
	{
		_hover = hover;
		addObject(_hover);
		setSelectable(true);
	}

	float hx = _x;
	float hy = _y;

	glm::vec3 min, max;
	_hover->boundaries(&min, &max);
	
	hy = _y - 0.02 - (max.y - min.y) / 2;

	_hover->setAlignXY(_align, hx, hy);
	_hover->realign();
	_hover->setDisabled(true);
}

void Renderable::forceRender(bool vert, bool idx)
{
	for (size_t i = 0; i < objectCount(); i++)
	{
		_objects[i]->forceRender(vert, idx);
	}

	if (vert)
	{
		_forceVertices = true;
	}
	if (idx)
	{
		_forceIndices = true;
	}
	
}

void Renderable::unMouseOver()
{
	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->unMouseOver();
	}
}
