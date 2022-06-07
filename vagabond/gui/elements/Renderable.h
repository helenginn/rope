// Copyright (C) 2021 Helen Ginn

#ifndef __Slip_Renderable__
#define __Slip_Renderable__

#include <vagabond/utils/glm_import.h>

#include <GLES3/gl32.h>
#include <mutex>
#include <map>
#include <atomic>
#include "HasRenderables.h"

namespace Snow
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec4 color;
		glm::vec4 extra;
		glm::vec2 tex;
	};
}

using namespace Snow;

typedef struct
{
	GLuint index[3];
	GLfloat z;
} IndexTrio;


class SnowGL;

class Renderable : public HasRenderables
{
public:
	Renderable();
	virtual ~Renderable();
	virtual void initialisePrograms(std::string *v = NULL, 
	                                std::string *f = NULL,
	                                std::string *g = NULL);
	virtual void render(SnowGL *sender);
	void deleteOnMainThread();
	
	GLuint renderType()
	{
		return _renderType;
	}
	
	Vertex *vPointer()
	{
		return &_vertices[0];
	}
	
	Vertex vertex(size_t idx)
	{
		return _vertices[idx];
	}
	
	size_t vertexCount()
	{
		return _vertices.size();
	}
	
	void setVertex(size_t idx, Vertex v)
	{
		_vertices[idx] = v;
	}

	size_t vSize()
	{
		return sizeof(Vertex) * _vertices.size();
	}

	void clearVertices()
	{
		_vertices.clear();
		_indices.clear();
	}
	
	void setImage(std::string imagename);
	void copyFrom(Renderable *s);

	GLuint *iPointer()
	{
		return &_indices[0];
	}

	size_t iSize()
	{
		return sizeof(GLuint) * _indices.size();
	}
	
	GLuint texture()
	{
		return _texid;
	}
	
	size_t indexCount()
	{
		return _indices.size();
	}
	
	std::string name()
	{
		return _name;
	}
	
	void setName(std::string name)
	{
		_name = name;
	}
	
	bool isDisabled()
	{
		return _disabled;
	}
	
	void setModel(glm::mat4x4 model)
	{
		_model = model;
	}
	
	void setProj(glm::mat4x4 proj)
	{
		if (!_usesProj)
		{
			return;
		}

		_proj = proj;
	}
	
	glm::mat4x4 projection();
	glm::mat4x4 model();
	
	void setUnproj(glm::mat4x4 unproj)
	{
		_unproj = unproj;
	}
	
	void setUsesProjection(bool usesProj);
	
	void lockMutex()
	{
		_mut.lock();
	}
	
	bool tryLockMutex()
	{
		return _mut.try_lock();
	}
	
	void unlockMutex()
	{
		_mut.unlock();
	}
	
	void setDisabled(bool dis);
	
	void addToVertices(glm::vec3 add);
	
	virtual void rotateRound(glm::mat3x3 &rot, 
	                         glm::vec3 c = glm::vec3(0, 0, 0));
	void rotateRoundCentre(glm::mat3x3 m);

	void recolour(double red, double green, double blue,
	              std::vector<Vertex> *vs = NULL);
	void setAlpha(double alpha);
	glm::vec3 centroid();
	virtual void setPosition(glm::vec3 pos);
	virtual void setExtra(glm::vec4 pos);
	virtual void setCentre(double x, double y);
	void setLeft(double x, double y);
	void setRight(double x, double y);
	
	void realign();
	void realign(double x, double y);
	
	void changeMidPoint(double x, double y);
	void setHighlighted(bool highlighted);
	
	virtual void unMouseOver() {};
	virtual bool mouseOver() { return false; };

	void reorderIndices();
	void boundaries(glm::vec3 *min, glm::vec3 *max);
	bool intersectsPolygon(double x, double y, double *z);
	double envelopeRadius();
	double averageRadius();
	
	void changeToLines();
	void changeToTriangles();

	virtual void triangulate();
	virtual void calculateNormals();
	
	void remove()
	{
		_remove = true;
	}
	
	GLuint getProgram()
	{
		return _program;
	}
	
	bool shouldRemove()
	{
		return _remove;
	}
	
	void setFragmentShader(std::string shader)
	{
		_fString = shader;
	}

	void setShaderFile(std::string file, std::string *location,
	                   std::string *fLoc);
	void setFragmentShaderFile(std::string file);
	void setVertexShaderFile(std::string file);

	void setShadersLike(Renderable *o)
	{
		_vString = o->_vString;
		_fString = o->_fString;
	}

	bool checkErrors(std::string what = "");
	void deleteVBOBuffers();

	void setColour(double red, double green, double blue)
	{
		_red = red;
		_green = green;
		_blue = blue;
		recolour(red, green, blue);
		recolour(red, green, blue, &_unselectedVertices);
	}

	void resize(double scale, bool unselected = false);
	
	void changeVertexShader(std::string v);
	void changeFragmentShader(std::string f);
	void setSelectable(bool selectable);
	
	virtual void click() {};
	virtual void drag(double x, double y) {};
	
	void setDraggable(bool draggable)
	{
		_draggable = draggable;
	}

	bool isDraggable()
	{
		return _draggable;
	}
	
	bool isSelectable()
	{
		return _selectable;
	}

	void addToVertexArray(glm::vec3 add, std::vector<Vertex> *vs);
	
	static void changeSelectionResize(double resize)
	{
		_selectionResize = resize;
	}

	bool polygonIncludes(glm::vec3 point, GLuint *trio);
	Vertex &addVertex(glm::vec3 v, std::vector<Vertex> *vec = NULL);
	void addIndex(GLint i);
	void addIndices(GLuint i1, GLuint i2, GLuint i3);
	void maximalDim(double *min, double *max, int dim);
	double maximalWidth();
	double maximalHeight();


	glm::vec3 rayTraceToPlane(glm::vec3 point, GLuint *trio, 
	                          glm::vec3 dir, bool *backwards);
	
	bool isSelected()
	{
		return _selected;
	}

	void setupVBOBuffers();
	void rebufferVertexData();
	void rebufferIndexData();
	
	void forceRender()
	{
		_forceRender = true;
	}

	void setHover(Renderable *hover);

	enum Alignment
	{
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Centre = 1 << 2,
		Top = 1 << 3,
		Middle = 1 << 4,
		Bottom = 1 << 5,
	};

	void setArbitrary(double x, double y, Alignment a);
	
	void setAlignment(Alignment a)
	{
		_align = a;
	}
	
	const Alignment &alignment() const
	{
		return _align;
	}
	
	const glm::vec2 xy() const
	{
		return glm::vec2(_x, _y);
	}
protected:
	void rebindToProgram();
	double intersects(glm::vec3 pos, glm::vec3 dir);
	bool polygonIncludesY(glm::vec3 point, GLuint *trio);
	Vertex &addVertex(float v1, float v2, float v3,
	                  std::vector<Vertex> *vec = NULL);
	void fixCentroid(glm::vec3 centre);
	virtual void positionChanged() {};
	virtual void resized(double scale) {};
	
	virtual void extraUniforms() {};
	
	void setNeedsExtra(bool extra)
	{
		_extra = extra;
	}
	
	std::vector<Vertex> _vertices;
	std::vector<GLuint> _indices;
	std::vector<Vertex> _unselectedVertices;

	double _red;
	double _green;
	double _blue;

	bool _central;
	bool _usesFocalDepth;
	bool _usesLighting;
	bool _textured;
	bool _backToFront;
	GLuint _renderType;
	std::string _vString;
	std::string _fString;
	std::string _gString;
	GLuint _program;
	GLuint _usingProgram;
	GLint _uLight;

	GLfloat _lightPos[3];
	GLfloat _xAxis[3];
	GLfloat _focalPos[3];

	glm::mat4x4 _model;
	glm::mat4x4 _proj;
	glm::mat4x4 _unproj;
	GLuint _texid = 0;
	void appendObject(Renderable *object);
	std::atomic<bool> _forceRender{false};
	void rotateByMatrix(glm::mat3x3 m);

	SnowGL *_gl = nullptr;
	
	float cx() const
	{
		return _x;
	}
	
	float cy() const
	{
		return _y;
	}
	
	void setAlignXY(Alignment &a, float x, float y)
	{
		_x = x;
		_y = y;
		_align = a;
	}

	void addAlign(float x, float y)
	{
		_x += x;
		_y += y;

		for (size_t i = 0; i < objectCount(); i++)
		{
			object(i)->addAlign(x, y);
		}

		realign();
	}

	Renderable *_hover = nullptr;
private:
	void deleteTextures();
	void rebindVBOBuffers();
	void unbindVBOBuffers();
	int vaoForContext();
	void deletePrograms();
	GLuint addShaderFromString(GLuint program, GLenum type, std::string str);

	static bool index_behind_index(IndexTrio one, IndexTrio two);
	static bool index_in_front_of_index(IndexTrio one, IndexTrio two);

	std::map<GLuint, GLuint> _bVertices;
	std::map<GLuint, GLuint> _bElements;
	std::map<GLuint, GLuint> _vaoMap;
	GLuint _uModel;
	GLuint _uProj;
	std::vector<IndexTrio> _temp; // stores with model mat
	std::string _name;
	std::mutex _mut;
	
	glm::mat4x4 _glLightMat;
	glm::mat4x4 _glProj;
	glm::mat4x4 _glModel;

	bool _extra;
	bool _remove;
	bool _disabled;
	bool _selected;
	bool _selectable;
	bool _draggable;
	bool _usesProj = false;
	std::string _texture;

	std::string _fFile;
	std::string _gFile;
	std::string _vFile;
	
	Alignment _align;
	double _x = 0.0;
	double _y = 0.0;
	static double _selectionResize;
};

#endif
