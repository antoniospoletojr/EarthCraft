#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "SOIL/SOIL.h"
#include <fdeep/fdeep.hpp>
#include <chrono>

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
                float value = static_cast<float>(image.at<uchar>(row, col));
                
                // Store the value in the vector
                sketches_vector.emplace_back(value / 255.0f);
            }
        }
    }
    const auto model = fdeep::load_model("fdeep_model.json", false);
    
    const fdeep::tensor sketches_tensor(fdeep::tensor_shape(450, 450, 4), sketches_vector);
    const fdeep::tensor noise_tensor(fdeep::tensor_shape(28, 28, 1024), noise_vector);
    
    
    const auto output_tensor = model.predict({sketches_tensor, noise_tensor});
    
    std::cout << fdeep::show_tensor_shape(output_tensor.front().shape());
    
    
    const std::vector<float> output_vector = output_tensor.front().to_vector();
    cout << output_vector.size() << endl;
    std::vector<uint8_t> output_vector_uint8;
    for (auto& value : output_vector)
        output_vector_uint8.emplace_back(static_cast<uint8_t>(value * 127.5f + 127.5f));
    
    // Show image with opencv
    cv::Mat output_image(450, 450, CV_8U);
    for (int row = 0; row < output_image.rows; ++row)
    {
        for (int col = 0; col < output_image.cols; ++col)
        {
            // Get the pixel value at the current position
            uint8_t value = output_vector_uint8[row * output_image.cols + col];
            
            // Store the value in the vector
            output_image.at<uchar>(row, col) = value;
        }
    }
    cv::imshow("Image", output_image);
    cv::waitKey(0);
    
    framework.initialize(argc, argv);
    input_handler.initialize(&camera);
    
    glutDisplayFunc(drawScene);
    
    setup();
    //framework.run();
}