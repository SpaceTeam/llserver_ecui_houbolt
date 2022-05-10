#include "StateController.h"


static PyObject *
get_state_value(PyObject *self, PyObject *args)
{
    const char *stateName;

    if (!PyArg_ParseTuple(args, "s", &stateName))
        return NULL;
    int state = StateController::Instance() -> GetStateValue((std::string) stateName);
    printf("Gettin state through python method!\n");
    return PyLong_FromLong(state);
}

static PyMethodDef StateControllerMethods[] = {
    {"get_state_value",  get_state_value, METH_VARARGS,
     "Get the state value."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef StateControllerModule = {
    PyModuleDef_HEAD_INIT,
    "state_controller",   /* name of module */
    "Controll the state", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    StateControllerMethods
};

PyMODINIT_FUNC PyInit_statecontroller(void)
{
    return PyModule_Create(&StateControllerModule);
}


int runPyScript(std::string script){
    if (PyImport_AppendInittab("state_controller", PyInit_statecontroller) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        return -1;
    }
    Py_Initialize();
    PyObject *pmodule = PyImport_ImportModule("state_controller")
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'state_controller'\n");
    }
    PyRun_SimpleString(script);
    Py_Finalize();
    return 0;
}