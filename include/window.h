#pragma once
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    glViewport(0, 0, w, h);
}

GLFWwindow* createWindow(int w, int h, const char* title)
{
    // Initialize GLFW for window creation.
    glfwInit();

    // Specify OpenGL version (3.3).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Use core profile for modern OpenGL features.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Disable window resizing to maintain a fixed size.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Enable multisampling for smoother edges.
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create the GLFW window with specified dimensions and title.
    GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);

    // Set the OpenGL context to the newly created window.
    glfwMakeContextCurrent(window);

    // Set a callback function to handle window resizing.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

// center window function
// can be found here: https://github.com/glfw/glfw/issues/310
//
void setWindowCenter(GLFWwindow* window) {
    // Get window position and size
    int window_x, window_y;
    glfwGetWindowPos(window, &window_x, &window_y);

    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    // Halve the window size and use it to adjust the window position to the center of the window
    window_width *= 0.5;
    window_height *= 0.5;

    window_x += window_width;
    window_y += window_height;

    // Get the list of monitors
    int monitors_length;
    GLFWmonitor** monitors = glfwGetMonitors(&monitors_length);

    if (monitors == NULL) {
        return;
    }

    // Figure out which monitor the window is in
    GLFWmonitor* owner = NULL;
    int owner_x, owner_y, owner_width, owner_height;

    for (int i = 0; i < monitors_length; i++) {
        // Get the monitor position
        int monitor_x, monitor_y;
        glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

        // Get the monitor size from its video mode
        int monitor_width, monitor_height;
        GLFWvidmode* monitor_vidmode = (GLFWvidmode*)glfwGetVideoMode(monitors[i]);

        if (monitor_vidmode == NULL) {
            // Video mode is required for width and height, so skip this monitor
            continue;

        }
        else {
            monitor_width = monitor_vidmode->width;
            monitor_height = monitor_vidmode->height;
        }

        // Set the owner to this monitor if the center of the window is within its bounding box
        if ((window_x > monitor_x && window_x < (monitor_x + monitor_width)) && (window_y > monitor_y && window_y < (monitor_y + monitor_height))) {
            owner = monitors[i];

            owner_x = monitor_x;
            owner_y = monitor_y;

            owner_width = monitor_width;
            owner_height = monitor_height;
        }
    }

    if (owner != NULL) {
        // Set the window position to the center of the owner monitor
        glfwSetWindowPos(window, owner_x + (owner_width * 0.5) - window_width, owner_y + (owner_height * 0.5) - window_height);
    }
}