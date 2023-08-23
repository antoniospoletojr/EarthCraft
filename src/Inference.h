/**
@file
@brief Inference header file.
*/

#ifndef INFERENCE_H
#define INFERENCE_H

#include <thread>
#include <Python.h>
#include "Colors.h"
#include "Constants.h"
#include "Terrain.h"

/**
 * @brief Inference engine class which runs the python prediction.
 *
 * This class runs the Python script thread that performs the inference and manages the Python interpreter state.
 * The output of the inference is an heightmap stored as png from the Python script into the assets/sketches folder. 
 */
class Inference
{
public:
    /**
     * @brief Default constructor.
     */
    Inference();

    /**
     * @brief Destructor.
     */
    ~Inference();

    /**
     * @brief Run the inference engine.
     */
    void predict();

    /**
     * @brief Reset the inference engine state.
     */
    void reset();

private:
    PyThreadState *state; ///< Python thread state.
};

#endif
