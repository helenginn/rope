// Copyright (C) 2021 Helen Ginn

#ifndef __Slip_Renderable__
#define __Slip_Renderable__

#include <vagabond/utils/gl_import.h>
#include <mutex>
#include <map>
#include <atomic>
#include "HasRenderables.h"
#include "ShaderGets.h"

typedef struct
{
	GLuint index[3];
	GLfloat z;
} IndexTrio;


class SnowGL;

class Renderable : public HasRenderables
{
public:
	Renderable() {}
	virtual ~Renderable();
	virtual void initialisePrograms();
	virtual void render(SnowGL *sender);
	void deleteOnMainThread();
	
	virtual void test() = 0;
	
	GLuint renderType()
	{
		return _renderType;
	}
	
	Vertex vertex(size_t idx)
	{
		return _vertices[idx];
	}
	
	size_t vertexCount()
	{
		return _vertices.size();
	}
	
	virtual const std::vector<Vertex> &vertices() const
	{
		return _vertices;
	}
	
	void setVertex(size_t idx, Vertex v)
	{
		_vertices[idx] = v;
	}

	void clearVertices()
	{
		_vertices.clear();
		_indices.clear();
	}
	
	void setImage(std::string imagename, bool wrap = false);
	void copyFrom(Renderable *s);

	GLuint *iPointer()
	{
		return &_indices[0];
	}

	size_t iSize()
	{
		return sizeof(GLuint) * _indices.size();
	}

	virtual const std::vector<GLuint> &indices() const
	{
		return _indices;
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

	
	void changeMidPoint(double x, double y);
	void setHighlighted(bool highlighted);
	
	virtual void unMouseOver();
	virtual bool mouseOver() { return false; };
	virtual void undrag() {};

	void reorderIndices();
	void boundaries(glm::vec3 *min, glm::vec3 *max);
	bool intersectsRay(double x, double y, double *z);
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

//	bool checkErrors(std::string what = "");
	void deleteVBOBuffers();

	void setColour(double red, double green, double blue)
	{
		_red = red;
		_green = green;
		_blue = blue;
		recolour(red, green, blue);
		recolour(red, green, blue, &_unselectedVertices);
	}

	void resize(double scale, bool unselected = false,
	            bool realign = false, bool recursive = true);
	
	void changeVertexShader(std::string v);
	void changeFragmentShader(std::string f);
	void setSelectable(bool selectable);
	
	virtual void click(bool left = true) {};
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
	
	void changeSelectionResize(double resize)
	{
		_selectionResize = resize;
	}

	bool polygonIncludes(glm::vec3 point, GLuint *trio);
	Vertex &addVertex(glm::vec3 v, std::vector<Vertex> *vec = NULL);
	void addIndex(GLint i);
	void addIndicesToEnd(std::vector<GLuint> &indices);
	static void addIndex(std::vector<GLuint> &indices, 
	                     std::vector<Snow::Vertex> &vertices, GLint i);
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
	
	void forceRender(bool vert = true, bool idx = true);

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

	void setCurrentVertex(int i)
	{
		_currVertex = i;
	}
	
	void setScroll(bool scroll)
	{
		_scroll = true;
	}

	void setResizeScale(float scale)
	{
		_resizeScale = scale;
	}
	
	virtual GLuint program()
	{
		return _program;
	}

	virtual bool hasTexture() const
	{
		return (_texid > 0 || _expectingTexture);
	}

	int vaoForContext();
protected:
	bool intersectsPolygon(double x, double y, double *z);
	int intersectsPoint(double x, double y, double *z);
	double intersects(glm::vec3 pos, glm::vec3 dir);
	bool polygonIncludesY(glm::vec3 point, GLuint *trio);
	Vertex &addVertex(float v1, float v2, float v3,
	                  std::vector<Vertex> *vec = NULL);
	void fixCentroid(glm::vec3 centre);
	virtual void positionChanged() {};
	virtual void resized(double scale) {};
	
	virtual void extraUniforms() {};
	
	std::vector<Vertex> _vertices;
	std::vector<GLuint> _indices;
	std::vector<Vertex> _unselectedVertices;

	bool _scroll = false;
	bool _expectingTexture = false;
	double _red;
	double _green;
	double _blue;
	
	const float &resizeScale() const
	{
		return _resizeScale;
	}

	bool _backToFront = false;
	GLuint _renderType = GL_TRIANGLES;
	std::string _vString;
	std::string _fString;
	std::string _gString;
	GLuint _program = 0;
	GLuint _usingProgram = 0;

	GLfloat _lightPos[3];
	GLfloat _xAxis[3];
	GLfloat _focalPos[3];

	glm::mat4x4 _model = glm::mat4(1.);
	glm::mat4x4 _myModel = glm::mat4(1.);
	glm::mat4x4 _proj = glm::mat4(1.);
	glm::mat4x4 _unproj = glm::mat4(1.);
	GLuint _texid = 0;
	void appendObject(Renderable *object);
	std::atomic<bool> _forceVertices{false};
	std::atomic<bool> _forceIndices{false};
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
	
	void setAlignXY(const Alignment &a, float x, float y)
	{
		if (x == x) _x = x;
		if (y == y) _y = y;
		_align = a;
	}

	const int &currentVertex() const
	{
		return _currVertex;
	}

	Renderable *_hover = nullptr;
	
	enum RaySearch
	{
		Default,
		Point,
		SearchPlane
	};
	
	RaySearch _searchType = Default;
	double _selectionResize = 1.1;
	
	ShaderGets *_shaderGets = nullptr;
	std::mutex _vertLock;
	std::mutex _buffLock;
private:
	glm::mat4x4 getModel();
	void deleteTextures();
	void rebindVBOBuffers();
	void unbindVBOBuffers();
	void runProgram();
	void deletePrograms();
	void resize_around_centre(double scale, glm::vec3 v, bool unselected,
	                          bool realign = false, bool recursive = true);
//	GLuint addShaderFromString(GLuint program, GLenum type, std::string str);

	static bool index_behind_index(IndexTrio one, IndexTrio two);
	static bool index_in_front_of_index(IndexTrio one, IndexTrio two);

	std::map<GLuint, GLuint> _vaoMap;
	GLuint _uModel = 0;
	GLuint _uProj = 0;
	std::vector<IndexTrio> _temp; // stores with model mat
	std::string _name = "generic object";
	
	glm::mat4x4 _glLightMat;
	glm::mat4x4 _glProj;
	glm::mat4x4 _glModel;

	bool _remove = false;
	bool _disabled = false;
	bool _selected = false;
	bool _selectable = false;
	bool _draggable = false;
	bool _usesProj = false;

	std::string _texture;

	std::string _fFile;
	std::string _gFile;
	std::string _vFile;
	
	Alignment _align;
	double _x = 0.0;
	double _y = 0.0;
	float _resizeScale = 1.f;
	int _currVertex = -1;
	int _renderCount = 0;
	
};

#endif
