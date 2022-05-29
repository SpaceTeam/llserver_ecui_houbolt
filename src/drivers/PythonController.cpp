#include "driver/PythonController.h"

#include "StateController.h"
#include "EventManager.h"
#include "utility/utils.h"

#include <stdio.h>
#include <Python.h>

static PyObject * get_state_value(PyObject *self, PyObject *args)
{
    const char *stateName;

    if (!PyArg_ParseTuple(args, "s", &stateName))
        return NULL;

    try
    {
        double stateValue = StateController::Instance() -> GetStateValue((std::string) stateName);
        return PyFloat_FromDouble(stateValue);
    }
    catch (std::exception &e)
    {
        Debug::error("PythonController - get_state_value: StateController - GetStateValue failed, %s", e.what());
    }
    
    return NULL;
}

static PyObject * set_state(PyObject *self, PyObject *args)
{
    const char *stateName;
    double stateValue;

    if (!PyArg_ParseTuple(args, "sd", &stateName, &stateValue))
        return NULL;

    try
    {
        StateController::Instance() -> SetState((std::string) stateName, stateValue, utils::getCurrentTimestamp());
    }
    catch (std::exception &e)
    {
        Debug::error("PythonController - set_state: StateController - SetState failed, %s", e.what());
    }
    
    Py_RETURN_NONE;
}

static PyMethodDef StateControllerMethods[] = {
    {"GetStateValue",  get_state_value, METH_VARARGS,
     "Get the state value."},
    {"SetState", set_state, METH_VARARGS, "Set the state value."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef StateControllerModule = {
    PyModuleDef_HEAD_INIT,
    "state_controller",   /* name of module */
    "Control the state", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    StateControllerMethods
};

PyMODINIT_FUNC PyInit_StateController(void)
{
    return PyModule_Create(&StateControllerModule);
}

static PyObject * execute_command(PyObject *self, PyObject *args)
{
    const char *commandName;
    PyObject *paramsObj;

    std::vector<double> params;

    if (PyArg_ParseTuple(args, "sO", &commandName, &paramsObj))
    {
        if (PyList_Check(paramsObj))
        {
            /* get the number of lines passed to us */
            Py_ssize_t size = PyList_Size(paramsObj);
            

            PyObject *currParamElem;

            for (Py_ssize_t i = 0; i < size; i++)
            {
                /* grab the string object from the next element of the list */
                currParamElem = PyList_GetItem(paramsObj, i); /* Can't fail */

                /* make it a string */
                params.push_back(PyFloat_AsDouble(currParamElem));
            }
        }
        else if (PyNumber_Check(paramsObj))
        {
            PyObject *floatObj = PyNumber_Float(paramsObj);
            if (floatObj != NULL)
            {
                params.push_back(PyFloat_AsDouble(floatObj));
            }
            else
            {
                Debug::error("pynumber to float failed");
                PyErr_SetString(PyExc_TypeError, "pynumber to float failed.");
                return NULL;
            }
            
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "params must be a list or a single number.");
            return NULL;
        }
    }
    else
    {
        return NULL;
    }

    try
    {
        EventManager::Instance() -> ExecuteCommand((std::string) commandName, params, false); //let's assume no testonly for now
    }
    catch (std::exception &e)
    {
        Debug::error("PythonController - execute_command: EventManager - ExecuteCommand failed, %s", e.what());
    }
    
    Py_RETURN_NONE;
}

static PyMethodDef EventManagerMethods[] = {
    {"ExecuteCommand",  execute_command, METH_VARARGS,
     "Execute command directly over CAN Bus."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef EventManagerModule = {
    PyModuleDef_HEAD_INIT,
    "event_manager",   /* name of module */
    "Manage Events", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    EventManagerMethods
};

PyMODINIT_FUNC PyInit_EventManager(void)
{
    return PyModule_Create(&EventManagerModule);
}

PythonController::~PythonController()
{
    if (running)
    {
        StateController::Instance() -> SetState((std::string) "python_running", 0, utils::getCurrentTimestamp());
        if (pyThread != nullptr)
        {
            if (pyThread->joinable())
            {
                pyThread->join();
                delete pyThread;
            }
        }
    }
    
    return;
}

void PythonController::StartPythonScript(std::string scriptPath)
{
    if (!running)
    {
        running = true;
        Debug::print("Executing Python script.");
        pyThread = new std::thread(&PythonController::RunPyScript, this, scriptPath);
    }
    
}

void PythonController::RunPyScript(std::string scriptPath)
{
    if (PyImport_AppendInittab("state_controller", PyInit_StateController) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        return;
    }
    if (PyImport_AppendInittab("event_manager", PyInit_EventManager) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        return;
    }

    Py_Initialize();

    FILE *fp = _Py_fopen(scriptPath.c_str(), "r");

    StateController::Instance() -> SetState((std::string) "python_running", 1, utils::getCurrentTimestamp());
    

	PyRun_SimpleFile(fp, scriptPath.c_str());

    Py_Finalize();
    running = false;
}