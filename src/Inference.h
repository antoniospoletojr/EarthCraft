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
    void predict(bool *can_continue);
    void reset();
    Terrain *getTerrain();

private:
    void worker(bool *can_continue);
    
    Terrain *terrain;
    bool is_running;
    std::thread thread;
    PyThreadState *state;
};

#endif
