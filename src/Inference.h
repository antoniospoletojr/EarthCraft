#ifndef INFERENCE_H
#define INFERENCE_H

#include <thread>
#include <Python.h>
#include "Colors.h"
#include "Constants.h"
#include "Terrain.h"

class Inference
{
public:
    Inference();
    ~Inference();
    void predict();
    void reset();
    Terrain *terrain;

private:
    PyThreadState *state;
};

#endif
