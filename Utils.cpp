#include "Utils.h"
#include <Python.h>

// Description: This file contains the functions to execute the Python scripts
// Function to execute the Python script
namespace Utils
{
    // Initialize the Python interpreter (call this once at the beginning of your program)
    void initializePythonInterpreter()
    {
        Py_Initialize();
    }

    // Finalize the Python interpreter (call this once at the end of your program)
    void finalizePythonInterpreter()
    {
        Py_Finalize();
    }

    // Function to execute the Python script
    void predict()
    {
        // Open the Python script file
        FILE *file = fopen("predict.py", "r");

        // Check if the file was opened successfully
        if (file)
        {
            // Execute the Python script
            PyRun_SimpleFile(file, "predict.py");

            // Close the file
            fclose(file);
        }
        else
        {
            printf("Error: Could not open Python script file\n");
        }
    }
}
