#ifndef INFERENCE_H
#define INFERENCE_H

#include <thread>
#include <Python.h>

class Inference
{
public:
    Inference();
    ~Inference();
    void predict();
    void reset();

private:
    void worker();

    bool is_running;
    std::thread thread;
    PyThreadState *state;
};

#endif
