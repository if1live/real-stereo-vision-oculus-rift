// Ŭnicode please 

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <memory>
#include "renderer.h"

#include "texture_mapping.h"

const float kWidth = 1280;
const float kHeight = 800;

GLFWwindow *g_window = nullptr;

bool InitWindow(int width, int height) 
{
	if(!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	std::ostringstream oss;
	oss << "If last character is f, run fullscreen mode." << std::endl;
	oss << "elif last character is w, run window mode." << std::endl;
	oss << "else, run window mode" << std::endl;
	std::cout << oss.str();

	std::string input;
	std::cin >> input;
	
	bool fullscreen = false;
	if(input[0] == 'f') {
		fullscreen = true;
	}
    
    // Create a windowed mode window and its OpenGL context 
	if(fullscreen) {
		g_window = glfwCreateWindow(width, height, "Haruna", glfwGetPrimaryMonitor(), NULL);
	} else {
		g_window = glfwCreateWindow(width, height, "GLSL Example", NULL, NULL);
	}

    if (!g_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Make the window's context current 
    glfwMakeContextCurrent(g_window);

	GLenum err = glewInit();
	if(GLEW_OK != err) {
		// Problem: glewInit failed, something is seriously wrong.
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    
	return true;
}


int main()
{
	std::unique_ptr<AbstractLogic> logic(new Texture2DMapping(kWidth, kHeight));

	InitWindow(static_cast<int>(kWidth), static_cast<int>(kHeight));

	bool running = true;
	bool init_result = logic->Init();
	if(init_result == false) {
		getchar();
		running = false;
	}

	double old_time = glfwGetTime();
	while(running) {
		double current_time = glfwGetTime();

		float dt = static_cast<float>(current_time - old_time);
		running = logic->Update(dt);
		logic->Draw();

		glfwSwapBuffers(g_window);
		old_time = current_time;

        // Poll for and process events
        glfwPollEvents();
	}

	logic.reset(NULL);
	glfwTerminate();
	return 0;	
}	