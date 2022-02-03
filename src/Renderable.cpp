// Copyright (C) 2021 Helen Ginn

#include "Renderable.h"
#include "SnowGL.h"
#include "commit.h"
#include "Library.h"
#include "FileReader.h"
#include <float.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>
#include "matrix_functions.h"

double Renderable::_selectionResize = 1.1;

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

Renderable::Renderable()
{
	_forceRender = false;
	_usesProj = false;
	_texid = 0;
	_gl = NULL;
	_textured = true;
	_name = "generic object";
	_remove = false;
	_renderType = GL_TRIANGLES;
	_program = 0;
	_usingProgram = 0;
	_backToFront = false;
	_uModel = 0;
	_model = glm::mat4(1.);
	_proj = glm::mat4(1.);
	_extra = false;
	_usesLighting = false;
	_usesFocalDepth = false;
	_central = 0;
	_disabled = 0;
	_selected = false;
	_selectable = false;
	_draggable = false;
}

Renderable::~Renderable()
{
	deletePrograms();
	deleteVBOBuffers();
	
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
}

GLuint Renderable::addShaderFromString(GLuint program, GLenum type, 
                                       std::string str)
{
	GLint length = str.length();
	
	if (length == 0)
	{
		return 0;
	}

	const char *cstr = str.c_str();
	GLuint shader = glCreateShader(type);
	bool error = checkErrors("create shader");
	
	if (error)
	{
		switch (type)
		{
			case GL_GEOMETRY_SHADER:
			std::cout <<  "geometry" << std::endl;
			break;
			
			case GL_VERTEX_SHADER:
			std::cout <<  "vertex" << std::endl;
			break;
			
			case GL_FRAGMENT_SHADER:
			std::cout <<  "fragment" << std::endl;

			default:
			std::cout << "Other" << std::endl;
			break;
		}
	}

	glShaderSource(shader, 1, &cstr, &length);
	checkErrors("sourcing shader");
	
	glCompileShader(shader);
	checkErrors("compiling shader");

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		char *log;

		/* get the shader info log */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char *)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);

		/* print an error message and the info log */
		std::cout << "Shader: unable to compile: " << std::endl;
		std::cout << str << std::endl;
		std::cout << log << std::endl;
		free(log);

		glDeleteShader(shader);
		return 0;
	}

	glAttachShader(_program, shader);
	return shader;
}

void Renderable::deletePrograms()
{
	if (_program != 0)
	{
		glDeleteProgram(_program);
	}
	_program = 0;
}

void Renderable::rebindToProgram()
{

}

void Renderable::initialisePrograms(std::string *v, std::string *f,
                                    std::string *g)
{
	if (_program != 0)
	{
		return;
	}

	if (v == NULL)
	{
		v = &_vString;
	}

	if (f == NULL)
	{
		f = &_fString;
	}

	if (g == NULL)
	{
		g = &_gString;
	}
	
	if (_vString.length() == 0 || _fString.length() == 0)
	{
		return;
	}

	GLint result;

	/* create program object and attach shaders */
	_program = glCreateProgram();
	checkErrors("create new program");

	addShaderFromString(_program, GL_VERTEX_SHADER, *v);
	checkErrors("adding vshader");
	
	if (g->length() > 0)
	{
		addShaderFromString(_program, GL_GEOMETRY_SHADER, *g);
		checkErrors("adding gshader");
	}

	addShaderFromString(_program, GL_FRAGMENT_SHADER, *f);
	checkErrors("adding fshader");

	glBindAttribLocation(_program, 0, "position");
	glBindAttribLocation(_program, 1, "normal");
	glBindAttribLocation(_program, 2, "color");

	if (!_extra)
	{
		glBindAttribLocation(_program, 3, "projection");
	}
	else
	{
		glBindAttribLocation(_program, 3, "extra");
	}

	if (_textured || _texid > 0)
	{
		glBindAttribLocation(_program, 4, "tex");
	}

	checkErrors("binding attributions");

	/* link the program and make sure that there were no errors */
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &result);
	checkErrors("linking program");

	if (result == GL_FALSE)
	{
		std::cout << "sceneInit(): Program linking failed." << std::endl;

		GLint length = 256;
		char *log = (char *)malloc(length);
		/* get the shader info log */
		glGetProgramInfoLog(_program, GL_INFO_LOG_LENGTH, &length, log);

		/* print an error message and the info log */
		std::cout << log << std::endl;
		/* delete the program */
		glDeleteProgram(_program);
		_program = 0;
	}
}

void Renderable::deleteTextures()
{
	if (_texid != 0)
	{
		Library::getLibrary()->dropTexture(_texid);
	}
}

void Renderable::unbindVBOBuffers()
{
	glBindVertexArray(0);
	checkErrors("vbo buffer unbinding");
}

void Renderable::deleteVBOBuffers()
{
	if (_usingProgram != 0 && _vaoMap.count(_usingProgram))
	{
		GLuint vao = _vaoMap[_usingProgram];
		glDeleteVertexArrays(1, &vao);
		_vaoMap.erase(_usingProgram);
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
}

int Renderable::vaoForContext()
{
	if (_vaoMap.count(_usingProgram) && !_forceRender)
	{
		GLuint vao = _vaoMap[_usingProgram];
		return vao;
	}
	else if (_vaoMap.count(_usingProgram) && _forceRender)
	{
		_forceRender = false;
		rebufferVertexData();
		GLuint vao = _vaoMap[_usingProgram];
		return vao;
	}
	
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	_vaoMap[_usingProgram] = vao;
	setupVBOBuffers();
	
	return vao;
}

void Renderable::rebufferVertexData()
{
	if (_bVertices.count(_usingProgram) == 0)
	{
		return;
	}

	GLuint bv = _bVertices[_usingProgram];
	glBindBuffer(GL_ARRAY_BUFFER, bv);

	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
}

void Renderable::setupVBOBuffers()
{
	if (_usingProgram == 0)
	{
		return;
	}

	int vao = vaoForContext();
	glBindVertexArray(vao);

	GLuint bv = 0;
	glGenBuffers(1, &bv);
	_bVertices[_usingProgram] = bv;

	glBindBuffer(GL_ARRAY_BUFFER, bv);
	checkErrors("binding array buffer");
	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_DYNAMIC_DRAW);

	checkErrors("rebuffering data buffer");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	if (_textured || _texid > 0)
	{
		glEnableVertexAttribArray(4); 
	}

	/* Vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(0 * sizeof(float)));
	checkErrors("binding vertices");

	/* Normals */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(3 * sizeof(float)));
	checkErrors("binding indices");

	/* Colours */
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(6 * sizeof(float)));

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(10 * sizeof(float)));

	checkErrors("binding attributes");

	if (_textured || _texid > 0)
	{
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(14 * sizeof(float)));

		checkErrors("rebinding texture attributes");
	}

	GLuint be = 0;
	glGenBuffers(1, &be);
	_bElements[_usingProgram] = be;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bElements[_usingProgram]);
	checkErrors("index array binding");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);
	checkErrors("index array buffering");
	glBindBuffer(GL_ARRAY_BUFFER, _bVertices[_usingProgram]);
	checkErrors("vbo binding");
	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
	checkErrors("vbo buffering");

	glBindVertexArray(0);
}

bool Renderable::checkErrors(std::string what)
{
	if (SDL_GL_GetCurrentContext() == NULL)
	{
		return 0;
	}

	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "Error as " << _name << " was doing " << what << ":" 
		<< err << std::endl;
		
		switch (err)
		{
			case GL_INVALID_ENUM:
			std::cout << "Invalid enumeration" << std::endl;
			break;

			case GL_STACK_OVERFLOW:
			std::cout << "Stack overflow" << std::endl;
			break;

			case GL_STACK_UNDERFLOW:
			std::cout << "Stack underflow" << std::endl;
			break;

			case GL_OUT_OF_MEMORY:
			std::cout << "Out of memory" << std::endl;
			break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "Invalid framebuffer op" << std::endl;
			break;

			case GL_INVALID_VALUE:
			std::cout << "Invalid value" << std::endl;
			break;

			case GL_INVALID_OPERATION:
			std::cout << "Invalid operation" << std::endl;
			break;

		}
	}
	
	return (err != 0);
}

void Renderable::render(SnowGL *sender)
{
	if (!tryLockMutex())
	{
		return;
	}

	if (_disabled)
	{
		unlockMutex();
		return;
	}
	
	_gl = sender;
	
	if (_program == 0)
	{
		initialisePrograms();
	}

	if (_gl->getOverrideProgram() > 0)
	{
		_usingProgram = _gl->getOverrideProgram();
	}
	else
	{
		_usingProgram = _program;
	}

	if (_program > 0)
	{
		glUseProgram(_usingProgram);

		checkErrors("use program");
		rebindVBOBuffers();
		checkErrors("rebinding program");

		_model = sender->getModel();
		const char *uniform_name = "model";
		_uModel = glGetUniformLocation(_usingProgram, uniform_name);
		_glModel = glm::transpose(model());
		glUniformMatrix4fv(_uModel, 1, GL_FALSE, &_model[0][0]);
		checkErrors("rebinding model");

		_proj = sender->getProjection();
		uniform_name = "projection";
		_uProj = glGetUniformLocation(_usingProgram, uniform_name);
		_glProj = glm::transpose(projection());
		glUniformMatrix4fv(_uProj, 1, GL_FALSE, &_proj[0][0]);
		checkErrors("rebinding projection");

		if (_gl->getOverrideProgram() == 0)
		{
			extraUniforms();
		}

		checkErrors("rebinding extras");

		if ((_textured || _texid > 0)
		    && _gl->getOverrideProgram() == 0)
		{
			GLuint which = GL_TEXTURE_2D;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(which, _texid);
			GLuint uTex = glGetUniformLocation(_usingProgram, "pic_tex");
			glUniform1i(uTex, 0);
		}

		if (_gl != NULL && _gl->depthMap() > 0 && 
		    _gl->getOverrideProgram() == 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, _gl->depthMap());
			GLuint uTex = glGetUniformLocation(_usingProgram, "shadow_map");
			glUniform1i(uTex, 1);

			glm::mat4x4 lightMat = sender->lightMat();
			uniform_name = "light_mat";
			GLuint uLight = glGetUniformLocation(_usingProgram, uniform_name);
			_glLightMat = glm::transpose(lightMat);
			glUniformMatrix4fv(uLight, 1, GL_FALSE, &_glLightMat[0][0]);
			checkErrors("rebinding lights");

		}

		glDrawElements(_renderType, indexCount(), GL_UNSIGNED_INT, 0);
		checkErrors("drawing elements");

		glUseProgram(0);
		unbindVBOBuffers();
		unlockMutex();
	}

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

void Renderable::resize(double scale, bool unselected)
{
	glm::vec3 centre = centroid();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &pos = _vertices[i].pos;
		pos -= centre;
		pos *= scale;
		pos += centre;
	}
	
	rebindVBOBuffers();
	
	if (!unselected)
	{
		resized(scale);
		return;
	}
	
	for (size_t i = 0; i < _unselectedVertices.size(); i++)
	{
		glm::vec3 &pos = _unselectedVertices[i].pos;
		pos -= centre;
		pos *= scale;
		pos += centre;
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

void Renderable::addVertex(glm::vec3 v, std::vector<Vertex> *vec)
{
	addVertex(v.x, v.y, v.z, vec);
}

void Renderable::addVertex(float v1, float v2, float v3,
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
	}
	else
	{
		vec->push_back(v);
	}
}

void Renderable::addIndices(GLuint i1, GLuint i2, GLuint i3)
{
	_indices.push_back(i1);
	_indices.push_back(i2);
	_indices.push_back(i3);
}

void Renderable::addIndex(GLint i)
{
	if (i < 0)
	{
		_indices.push_back(_vertices.size() + i);
		return;
	}

	_indices.push_back(i);
}

bool Renderable::index_behind_index(IndexTrio one, IndexTrio two)
{
	return (one.z > two.z);
}

bool Renderable::index_in_front_of_index(IndexTrio one, IndexTrio two)
{
	return (one.z < two.z);
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
		glm::vec3 &tmpVec = _vertices[n].pos;
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

bool Renderable::intersectsPolygon(double x, double y, double *z)
{
	glm::vec3 target = glm::vec3(x, y, 0);
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		glm::vec3 projs[3];

		for (int j = 0; j < 3; j++)
		{
			projs[j] = _vertices[_indices[i+j]].pos;
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

			if (v.pos[j] < (*max)[j])
			{
				(*max)[j]= v.pos[j];
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
	rebindVBOBuffers();
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
	lockMutex();
	addToVertices(diff);
	unlockMutex();
	
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
	#ifndef __EMSCRIPTEN__
	path = std::string(DATA_DIRECTORY) + "/" + file;
	#endif

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

void Renderable::setImage(std::string imagename)
{
	GLuint tex = Library::getLibrary()->getTexture(imagename);
	_texid = tex;
	_texture = imagename;
}

double Renderable::maximalWidth()
{
	double max = -FLT_MAX;
	double min = FLT_MAX;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].pos[0] > max)
		{
			max = _vertices[i].pos[0];
		}

		if (_vertices[i].pos[0] < min)
		{
			min = _vertices[i].pos[0];
		}
	}
	
	return max - min;
}

void Renderable::appendObject(Renderable *object)
{
	int add = _vertices.size();
	_vertices.reserve(_vertices.size() + object->vertexCount());
	_indices.reserve(_indices.size() + object->indexCount());
	
	for (size_t i = 0; i < object->vertexCount(); i++)
	{
		Vertex &v = object->_vertices[i];
		_vertices.push_back(v);
	}

	for (size_t i = 0; i < object->_indices.size(); i++)
	{
		long idx = object->_indices[i] + add;
		_indices.push_back(idx);
	}
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
	
	calculateNormals();
	
	unlockMutex();
}
