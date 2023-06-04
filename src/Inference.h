#ifndef INFERENCE_H
#define INFERENCE_H

#include <thread>
#include <Python.h>

class Inference
{
public:
    Inference();
    ~Inference();
    void predict(bool *can_continue);
    void reset();

private:
    void worker(bool *can_continue);

    bool is_running;
    std::thread thread;
    PyThreadState *state;
};

#endif
