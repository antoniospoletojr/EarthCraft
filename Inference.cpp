#include "Inference.h"
#include <Python.h>

Inference::Inference()
{
    this->is_running = false;
    Py_Initialize();                               // Initialize the Python interpreter
    PyEval_InitThreads();                          // Initialize the Python threading system
    Py_DECREF(PyImport_ImportModule("threading")); // Import the 'threading' module
    state = PyEval_SaveThread();                   // Save the current thread state
}

Inference::~Inference()
{
    thread.join();               // Wait for the worker thread to finish
    PyEval_RestoreThread(state); // Restore the saved thread state
    Py_FinalizeEx();             // Finalize the Python interpreter
}

void Inference::reset()
{
    thread.join();               // Wait for the worker thread to finish
    PyEval_RestoreThread(state); // Restore the saved thread state
    state = PyEval_SaveThread(); // Save the current thread state
}

void Inference::predict()
{
    if (!is_running)
    {
        // Start the inference thread
        is_running = true;
        thread = std::thread(&Inference::worker, this);
    }
}

void Inference::worker()
{
    PyGILState_STATE gil_state;
    gil_state = PyGILState_Ensure(); // Acquire the Global Interpreter Lock (GIL)

    FILE *file = fopen("predict.py", "r");

    if (file)
    {
        PyRun_SimpleFile(file, "predict.py"); // Execute the Python script
        fclose(file);
    }
    else
    {
        printf("Error: Could not open Python script file\n");
    }

    // Inference is complete, set isRunning to false
    is_running = false;
    PyGILState_Release(gil_state); // Release the Global Interpreter Lock (GIL)
}
