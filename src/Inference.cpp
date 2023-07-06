#include "Inference.h"


Inference::Inference()
{
    Py_Initialize();                               // Initialize the Python interpreter
    PyEval_InitThreads();                          // Initialize the Python threading system
    Py_DECREF(PyImport_ImportModule("threading")); // Import the 'threading' module
    state = PyEval_SaveThread();                   // Save the current thread state
}

Inference::~Inference()
{
    PyEval_RestoreThread(state); // Restore the saved thread state
    Py_FinalizeEx();             // Finalize the Python interpreter
}

void Inference::reset()
{
    PyEval_RestoreThread(state); // Restore the saved thread state
    state = PyEval_SaveThread(); // Save the current thread state
}

void Inference::predict()
{
    PyGILState_STATE gil_state;
    gil_state = PyGILState_Ensure(); // Acquire the Global Interpreter Lock (GIL)
    FILE *file = fopen("./src/predict.py", "r");

    if (file)
    {
        PyRun_SimpleFile(file, "./src/predict.py"); // Execute the Python script
        fclose(file);
    }
    else
    {
        printf("Error: Could not open Python script file\n");
    }
    
    PyGILState_Release(gil_state); // Release the Global Interpreter Lock (GIL)
    printf(COLOR_GREEN "Inference complete\n" COLOR_RESET);
    fflush(stdout);
}