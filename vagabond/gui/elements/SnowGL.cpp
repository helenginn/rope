// Copyright (C) 2021 Helen Ginn

#define SHADOW_DIM 1024

#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL2/SDL.h>
#include "SnowGL.h"
#include "Renderable.h"
#include "Quad.h"
#include "Window.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

#define MOUSE_SENSITIVITY 500

bool SnowGL::_setup = false;

SnowGL::SnowGL()
{
	_shadowing = false;
	_shadowProgram = 0;
	_depthMap = 0;
	_depthFbo = 0;
	_sceneMapCount = 0;
	memset(_sceneMap, '\0', sizeof(GLuint) * 8);
	_sceneFbo = 0;
	_sceneDepth = 0;
	_proj = glm::mat4(1.);
	_model = glm::mat4(1.);
	_shiftPressed = false;
	_controlPressed = false;
	_right = false;

	_zNear = 1;
	_zFar = 400;

	setupCamera();

	_r = 1.; _g = 1.; _b = 0.5; _a = 1.;
}

SnowGL::~SnowGL()
{
	delete _indices;

}

void SnowGL::time()
{
	_time += 0.002;
	
	if (_time > 1) _time -= 1;
}

void SnowGL::rotate(double x, double y, double z)
{
	_camAlpha += x;
	_camBeta += y;
	_camGamma += z;
}

void checkFrameBuffers()
{
	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Incomplete frame buffer" << std::endl;

		switch (result)
		{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "incomplete attachment" << std::endl;
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "incomplete missing attachment" << std::endl;
			break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "unsupported format combo" << std::endl;
			break;
			case GL_FRAMEBUFFER_UNDEFINED:
			std::cout << "buffer undefined" << std::endl;
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "incomplete multisample" << std::endl;
			break;

			default:
			std::cout << "Something else " << result << std::endl;
			break;
		}
		exit(1);
	}
}

void SnowGL::preparePingPongBuffers(int w_over, int h_over)
{
	glGenFramebuffers(2, _pingPongFbo);
	glGenTextures(2, _pingPongMap);

	int w = width();
	int h = height();
	
	if (w_over > 0 && h_over > 0)
	{
		w = w_over;
		h = h_over;
	}

	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _pingPongFbo[i]);
		glBindTexture(GL_TEXTURE_2D, _pingPongMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h,
		             0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		                       GL_TEXTURE_2D, _pingPongMap[i], 0);
		
		checkFrameBuffers();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SnowGL::resizeTextures(int w_over, int h_over)
{
	int w = width();
	int h = height();
	
	if (w_over > 0 && h_over > 0)
	{
		w = w_over;
		h = h_over;
	}

	glBindTexture(GL_TEXTURE_2D, _sceneDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	for (size_t i = 0; i < _sceneMapCount; i++)
	{
		/* wrong, if you bring this into action */
		glBindTexture(GL_TEXTURE_2D, _sceneMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
		             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	if (_pingPongFbo[0] > 0)
	{
		for (size_t i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, _pingPongMap[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
			             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		}
	}
}

void SnowGL::prepareDepthColourIndex()
{
	/* generate new frame buffer for additional targets */
	glGenFramebuffers(1, &_sceneFbo);
	
	/* bind it! */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _sceneFbo);

	/* generate a texture for each target */
	glGenTextures(2, _sceneMap);
	/* generate a single depth buffer as well */
	glGenTextures(1, &_sceneDepth);

	_sceneMapCount = 2;

	int w = Window::width();
	int h = Window::height();
	_dw = w; _dh = h;
	
	/* texture for the depth buffer */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sceneDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
	                       GL_TEXTURE_2D, _sceneDepth, 0);
	checkFrameBuffers();

	unsigned int attachments[2];
	
	/* additional attachments */
	for (size_t i = 0; i < 2; i++)
	{
		/* bind next texture */
		glActiveTexture(GL_TEXTURE0 + i + 1);
		/* from texture produced earlier */
		glBindTexture(GL_TEXTURE_2D, _sceneMap[i]);
		checkErrors("Binding texture");

		if (i == 0) /* suitable for colours */
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
			             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
			checkErrors("glTexImage2D");
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (i == 1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, w, h, 0, 
			             GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
		                       GL_TEXTURE_2D, _sceneMap[i], 0);
		checkFrameBuffers();
		
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	
	glDrawBuffers(2, attachments); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	_indices = new GLuint[_dw * _dh];
	memset(_indices, '\0', _dw * _dh * sizeof(GLuint));
	
	_quad = new Quad();
	_quad->prepareTextures(this);
	_quad->setTextureID(_sceneMap[0]);
}

int SnowGL::checkIndex(double x, double y) const
{
	if (_indices == nullptr)
	{
		return -1;
	}

	x = 0.5 + x / 2;
	y = 0.5 + y / 2;

	x *= _dw;
	y *= _dh;
	
	int idx = y * _dw + x;
	
	if (idx > _dw * _dh - 1)
	{
		return -1;
	}

	int val = _indices[idx];
	return val - 1;
}

void SnowGL::grabIndexBuffer()
{
	if (_sceneFbo == 0)
	{
		return;
	}

	checkErrors("before read pixels");
//	glBindTexture(GL_TEXTURE_2D, _sceneMap[1]);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, _indices);


	glBindFramebuffer(GL_READ_FRAMEBUFFER, _sceneFbo);
#ifndef __EMSCRIPTEN__
	glReadBuffer(GL_COLOR_ATTACHMENT0 + 1);
#endif
	glReadPixels(0, 0, _dw, _dh, GL_RED_INTEGER, GL_UNSIGNED_INT, _indices);

	checkErrors("read pixels");
}

void SnowGL::prepareShadowBuffer()
{
	glGenFramebuffers(1, &_depthFbo);
	glGenTextures(1, &_depthMap);
	glBindTexture(GL_TEXTURE_2D, _depthMap);

	int w = SHADOW_DIM; int h = SHADOW_DIM;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

	glBindFramebuffer(GL_FRAMEBUFFER, _depthFbo);
	checkErrors("Making framebuffer");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
	                       GL_TEXTURE_2D, _depthMap, 0);
	checkErrors("Binding texture");
	GLenum none = GL_NONE;
	glDrawBuffers(1, &none);
	glReadBuffer(GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	shadowProgram();
}

void SnowGL::renderShadows()
{
	_shadowing = true;
	int w = SHADOW_DIM; int h = SHADOW_DIM;
	glViewport(0, 0, w, h);
	glBindFramebuffer(GL_FRAMEBUFFER, _depthFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
	glm::mat4x4 model = _model;
	glm::mat4x4 proj = _proj;
    changeProjectionForLight(0);
    renderScene();
	_model = model;
	_proj = proj;

	int ratio = 1;
	glViewport(0, 0, width() * ratio, height() * ratio);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
	_shadowing = false;
}

void SnowGL::renderScene()
{
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		if (_objects[i]->shouldRemove())
		{
			delete _objects[i];
			_objects.erase(_objects.begin() + i);
			continue;
		}

		_objects[i]->setModel(_model);
		_objects[i]->setProj(_proj);
		_objects[i]->setUnproj(_unproj);
		_objects[i]->render(this);
	}
}

void SnowGL::render()
{
	checkErrors("before paintGL");
	updateCamera();
	
	if (!_viewChanged && !_alwaysOn)
	{
		return;
	}
	
	std::unique_lock<std::mutex> lock(_renderMutex, std::try_to_lock);
	
	if (!lock.owns_lock())
	{
		return;
	}

	if (_depthMap > 0)
	{
		renderShadows();
	}
	
	if (_sceneFbo > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _sceneFbo);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	checkErrors("clear color");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkErrors("clear buffer bits");

	renderScene();
	checkErrors("after render");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (_sceneFbo > 0)
	{
		grabIndexBuffer();
		_quad->render(this);
	}

	_viewChanged = false;
}

void SnowGL::updateCamera()
{
	glm::vec3 centre = _centre;
	glm::vec3 negCentre = centre;
	negCentre *= -1;
	
	if (fabs(_camAlpha) > 1e-6 || fabs(_camBeta) > 1e-6
	    || fabs(_camGamma) > 1e-6)
	{
		_viewChanged = true;
	}

	if (glm::length(_translation) > 1e-6)
	{
		_viewChanged = true;
	}

	/* move to centre, rotate, move away from centre */
	glm::mat4x4 back = glm::translate(glm::mat4(1.f), negCentre);
	glm::mat4x4 rot = glm::rotate(glm::mat4(1.f), _camAlpha, glm::vec3(1, 0, 0));
	rot = glm::rotate(rot, _camBeta, glm::vec3(0, 1, 0));
	rot = glm::rotate(rot, _camGamma, glm::vec3(0, 0, 1));
	glm::mat4x4 front = glm::translate(glm::mat4(1.f), centre);

	glm::mat4x4 change = front * rot * back;

	_centre += _translation;
	glm::mat4x4 transMat = glm::translate(glm::mat4(1.f), _translation);

	_model = change * transMat * _model;

	_camAlpha = 0; _camBeta = 0; _camGamma = 0;
	_translation = glm::vec3(0, 0, 0);
	
	_centre.x = 0;
	_centre.y = 0;
}


void SnowGL::setupCamera()
{
	_translation = glm::vec3(0, 0, 0);
	_transOnly = glm::vec3(0, 0, 0);
	_totalCentroid = glm::vec3(0, 0, 0);
	_centre = glm::vec3(0, 0, 0);
	_camAlpha = 0;
	_camBeta = 0;
	_camGamma = 0;
	_model = glm::mat4(1.);

	updateProjection();
	updateCamera();
}


void SnowGL::updateProjection(float side)
{
	float aspect = 1 / Window::aspect();
	
	if (aspect != aspect)
	{
		return;
	}
	
	_proj = glm::perspective((float)deg2rad(35), aspect, 
	                         _zNear, _zFar);

	_unproj = glm::inverse(_proj);
}

void SnowGL::resizeGL(int w, int h)
{
	updateProjection();
}

void SnowGL::convertCoords(double *x, double *y)
{
	double w = width();
	double h = height();

	*x = 2 * *x / w - 1.0;
	*y =  - (2 * *y / h - 1.0);
}

bool SnowGL::checkErrors(std::string what)
{
	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "Error as SnowGL was doing " << what << ":" 
		<< err << std::endl;
		
		switch (err)
		{
			case GL_INVALID_ENUM:
			std::cout << "Invalid enumeration" << std::endl;
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

			default:
			break;
		}
	}
	
	return (err != 0);
}

void SnowGL::shadowProgram()
{
	if (_shadowProgram != 0)
	{
		return;
	}
	
//	std::string v = vShadow();
//	std::string f = fShadow();
	std::string v, f;
	
	_shadowObj = new Renderable();
	_shadowObj->initialisePrograms(&v, &f);

	_shadowProgram = _shadowObj->getProgram();
}

GLuint SnowGL::getOverrideProgram()
{
	if (_shadowing)
	{
		return _shadowProgram;
	}
	
	return 0;
}

void SnowGL::resetMouseKeyboard()
{
	_right = false;
	_controlPressed = false;
	_shiftPressed = false;

}

void SnowGL::shiftToCentre(glm::vec3 &update, float distance)
{
	glm::vec3 diff = update - _centre;
	_model = glm::mat4(1.f);
	_centre += diff;
	_translation = -diff;
	_translation.z -= distance;
	updateCamera();
}
