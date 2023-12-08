#ifndef MODULE_H
#define MODULE_H

#include <Python.h>
#include <vector>
#include <vagabond/core/Atom.h>

struct AtomWrapper {
    Atom& atom;
    AtomWrapper(Atom& a) : atom(a) {}
};

// Function declarations
extern "C" {
    PyMODINIT_FUNC PyInit_module(void);
    PyMODINIT_FUNC helen_module_py_init(PyObject *module);
    PyObject* greet(PyObject* self, PyObject * args);
    PyObject* getOneAtomPosition(PyObject* self, PyObject* args);
    PyObject* getAtomPositions(PyObject* self, PyObject* args);
}

#endif  // MODULE_H
