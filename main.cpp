#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <cppflow/cppflow.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
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
    // Load the noise data
    std::vector<unsigned long> shape {};
    bool fortran_order;
    std::vector<double> noise_data;
    const std::string path {"noise.npy"};
    npy::LoadArrayFromNumpy(path, shape, fortran_order, noise_data);
    cout << noise_data.size() << endl;
    
    
    // Load the 4 channels
    std::vector<float> data;
    int width, height, channels;
    
    for (int i = 1; i <= 4; ++i)
    {
        char image_path[100];
        sprintf(image_path, "./custom_inputs/%d.png", i);
        cv::Mat image = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
        cv::imshow("Image", image);
        //cv::waitKey(0);
        for (int row = 0; row < image.rows; ++row)
        {
            for (int col = 0; col < image.cols; ++col)
            {
            // Get the pixel value at the current position
            float value = static_cast<float>(image.at<uchar>(row, col));
            
            // Store the value in the vector
            data.push_back(value);
            }
        }
    }

    
    cout << data.size() << endl;
    
    auto input_tensor = cppflow::tensor(data, {450, 450, 4});
    input_tensor = input_tensor / 255.f;
    input_tensor = cppflow::expand_dims(input_tensor, 0);
    
    // Generate random noise
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::normal_distribution<float> dist(0.0f, 1.0f);
    
    std::vector<float> w_noise(28 * 28 * 1024);
    int i = 0;
    for (auto& value : w_noise) {
        value = static_cast<float>(noise_data[i]);
        i = i+1;
    }
    
    // Reshape the vector to a tensor
    cppflow::tensor noise_tensor(w_noise, {1, 28, 28, 1024});

    
    // Load the model
    cout << "Loading model..." << endl;
    cppflow::model model("./saved_model");
    
    
    cout << "Running the model" << endl;
    auto output = model({{"serving_default_input_1:0", input_tensor}, {"serving_default_input_2:0", noise_tensor}},{"StatefulPartitionedCall:0"});
    cppflow::tensor output_tensor = cppflow::squeeze(output[0], {0});
    output_tensor = output_tensor * 127.5f + 127.5f;
    output_tensor = cppflow::cast(output_tensor, TF_FLOAT, TF_UINT8);
    cout << output_tensor.shape() << endl;
    
    // Write the output tensor to an image with stb_image
    std::vector<uint8_t> output_data = output_tensor.get_data<uint8_t>();
    cv::Mat output_image(450, 450, CV_8UC(1), output_data.data());    
    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);
    cv::imshow("Image", output_image);
    cv::waitKey(0);
    //stbi_write_jpg("output.png", width, height, channels, output_data.data(), 100);
    


    framework.initialize(argc, argv);
    input_handler.initialize(&camera);
    
    glutDisplayFunc(drawScene);
    
    setup();
    //framework.run();
}