#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <vector>
#include <string>
#include <vagabond/core/Atom.h>
#include "module.h"


PyMethodDef methods[] = 
{
    {"greet", greet, METH_VARARGS, "terrible documentation"},
    {"get_atom_positions", getAtomPositions, METH_NOARGS, "Get atom position"},
    {"get_one_atom_position", getOneAtomPosition, METH_NOARGS, "Get one atom position"},
    {NULL, NULL, 0, NULL}
};

struct PyModuleDef module_def = 
{
    PyModuleDef_HEAD_INIT,
    .m_name = "module",
    .m_doc = "Helen Module",
    .m_size = -1,
    .m_methods = methods,
};

PyMODINIT_FUNC PyInit_module(void)
{
	return PyModule_Create(&module_def);

	PyObject *m = PyModule_Create(&module_def);

	if (m)
	{
		m = helen_module_py_init(m);
	}

	return m;
}

PyMODINIT_FUNC helen_module_py_init(PyObject *module)
{
    return module;
}


PyObject* greet(PyObject* self, PyObject * args)
{
    return Py_BuildValue("s", "Hello world");
}

PyObject* getOneAtomPosition(PyObject* self, PyObject* args)
{
    Atom* atom = new Atom();
    std::string kk = "testAtom";
    atom->setAtomName(kk);

    glm::vec3 position;
    if (atom->fishPosition(&position))
    {
        // Return a tuple containing x, y, z
        return Py_BuildValue("(fff)", position.x, position.y, position.z);
    }
    else
    {
        // Return an error or default value
        PyErr_SetString(PyExc_RuntimeError, "Failed to get atom position");
        return NULL;
    }
}


PyObject* getAtomPositions(PyObject* self, PyObject* args)
{
    PyObject* positionsList = PyList_New(0);

    if (positionsList == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create positions list");
        return NULL;
    }
    std::vector<Atom> atoms;
    for (Atom& atom : atoms) {
        AtomWrapper atomWrapper(atom);

        glm::vec3 position;
        if (atomWrapper.atom.fishPosition(&position)) {
            PyObject* positionTuple = Py_BuildValue("(fff)", position.x, position.y, position.z);

            if (positionTuple != NULL) {
                PyList_Append(positionsList, positionTuple);
                Py_DECREF(positionTuple);
            } else {
                PyErr_SetString(PyExc_RuntimeError, "Failed to create position tuple");
                Py_DECREF(positionsList);
                return NULL;
            }
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Failed to get atom position");
            Py_DECREF(positionsList);
            return NULL;
        }
    }

    // Return the positions list
    return positionsList;
}