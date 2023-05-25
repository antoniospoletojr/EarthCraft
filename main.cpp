#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "onnxruntime_cxx_api.h"
#include "SOIL/SOIL.h"
#include <chrono>
#include "npy.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include <random>

#include "Terrain.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "GlutFramework.h"

using namespace std;

// Global objects variables
Terrain terrain;
Camera camera;
InputHandler input_handler;
GlutFramework framework;
Renderer renderer;


// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    camera.update();
    renderer.draw();
    
    glutSwapBuffers();
}

// This function sets up the initial state for OpenGL rendering.
void setup()
{
    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Set polygon mode to be not filled
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Enable the depth test to ensure that polygons that are behind others are not drawn
    glEnable(GL_DEPTH_TEST);
    
    // // Enable lighting calculations for polygons
    // glEnable(GL_LIGHTING);
    
    // // Enable light source 0
    // glEnable(GL_LIGHT0);
    
    // // Enable automatic normalization of surface normals to unit length
    // glEnable(GL_NORMALIZE);
    
    // Allocate a terrain object
    terrain.initialize("heightmap.png", 16.0);
    
    // Initialize the mesh
    renderer.initialize(&terrain);
}

// Main routine.
int main(int argc, char **argv)
{   
    // Load the model and create InferenceSession
    Ort::Env env;
    Ort::Session session(env, "model.onnx", Ort::SessionOptions{ nullptr });
    
    // Load noise
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> noise_vector(28 * 28 * 1024);
    for (auto& value : noise_vector)
        value = dist(gen);

    // Load sketches
    std::vector<float> sketches_vector;
    int width, height, channels;
    for (int i = 1; i <= 4; ++i)
    {
        char image_path[100];
        sprintf(image_path, "./custom_inputs/%d.png", i);
        cv::Mat image = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
        // cv::imshow("Image", image);
        //cv::waitKey(0);
        for (int row = 0; row < image.rows; ++row)
        {
            for (int col = 0; col < image.cols; ++col)
            {
                // Get the pixel value at the current position
                float value = static_cast<float>(image.at<uchar>(row, col)) / 255.0f;
                
                // Store the value in the vector
                sketches_vector.emplace_back(value);
            }
        }
    }

    // define shape
    const std::array<int64_t, 4> sketch_shape = { 1, 450, 450, 4};
    const std::array<int64_t, 4> noise_shape = { 1, 28, 28, 1024};
    const std::array<int64_t, 4> output_shape = { 1, 450, 450, 1};

    // define array
    std::array<float, 450*450*4> sketches;
    std::array<float, 28*28*1024> noise;
    std::array<float, 450*450> output;
    
    // define Tensor
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    auto sketch_tensor = Ort::Value::CreateTensor<float>(memory_info, sketches.data(), sketches.size(), sketch_shape.data(), sketch_shape.size());
    auto noise_tensor = Ort::Value::CreateTensor<float>(memory_info, noise.data(), noise.size(), noise_shape.data(), noise_shape.size());
    Ort::Value* input_tensor[2];
    input_tensor[0] = &sketch_tensor;
    input_tensor[1] = &noise_tensor;

    std::vector<Ort::Value> ort_inputs;
    ort_inputs.push_back(std::move(sketch_tensor));
    ort_inputs.push_back(std::move(noise_tensor));
    
    auto output_tensor = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape.data(), output_shape.size());

    // copy image data to input array
    std::copy(sketches_vector.begin(), sketches_vector.end(), sketches.begin());
    std::copy(noise_vector.begin(), noise_vector.end(), noise.begin());

     // define names
    Ort::AllocatorWithDefaultOptions ort_alloc;
    // Ort::AllocatedStringPtr input_sketch_name = session.GetInputNameAllocated(0, ort_alloc);
    // Ort::AllocatedStringPtr input_noise_name = session.GetInputNameAllocated(1, ort_alloc);
    Ort::AllocatedStringPtr output_name = session.GetOutputNameAllocated(0, ort_alloc);
    
    const std::vector<const char*> input_names = {"input_1", "input_2"};
    const std::array<const char*, 1> output_names = {output_name.get()};
    
    output_name.release();
    
    
    std::vector<Ort::Value> ort_outputs = session.Run(Ort::RunOptions{nullptr}, input_names.data(), ort_inputs.data(), ort_inputs.size(), output_names.data(), output_names.size());
    
    const float* data = ort_outputs[0].GetTensorData<float>();
    std::vector<float> output_vector(data, data + 450*450);

    // convert output_vector to uint8_t
    std::vector<uint8_t> output_vector_uint8;
    for (auto& value : output_vector)
        output_vector_uint8.emplace_back(static_cast<uint8_t>(value * 127.5f + 127.5f));
    
    // save output_vector_uint8 to png
    stbi_write_png("output.png", 450, 450, 1, output_vector_uint8.data(), 0);
    

    
    
    framework.initialize(argc, argv);
    input_handler.initialize(&camera);
    
    glutDisplayFunc(drawScene);
    
    setup();
    //framework.run();
}