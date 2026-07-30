// Copyright (C) 2021 Helen Ginn

#ifndef __practical__InfoModal__
#define __practical__InfoModal__

#include "Modal.h"

class Scene;

/** \class InfoModal
 * Simple, dismissable, read-only text popup. Unlike BadChoice, this is
 * marked stacked (see Modal::setStacked()), so it is meant to be shown on
 * top of another already-open modal (e.g. a help popup over a form) -
 * dismissing it removes only itself, leaving whatever is underneath in
 * place. */

class InfoModal : public Modal
{
public:
/** @param scene which should display this modal
 *  @param details message to display, pre-formatted with line breaks. */
	InfoModal(Scene *scene, std::string details);

	virtual void buttonPressed(std::string tag, Button *button = NULL);
};

#endif
