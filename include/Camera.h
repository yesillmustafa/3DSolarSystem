#pragma once

#include<GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui/imgui.h>


enum class CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UPWARD,
	DOWNWARD
};

class Camera
{

public:

	Camera(int width, int height);

	glm::vec3 getPosition();
	glm::vec3 getOrientation();
	glm::vec3 getUp();
	float getFOV();
	void setFOV(float value);

	void processInputs(GLFWwindow* window);

	void setPosition(const glm::vec3& position);
	void orientCamera(float pitch, float yaw);
	void moveAndOrientCamera(glm::vec3 target, float modelViewDistance);
	void setCameraMode(bool value);
	bool getCameraMode();

	bool isTorchPressed();
	void activateTorch();
	void deactivateTorch();

private:

	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right;
	glm::vec3 WorldUp = Up;

	bool firstClick = true;

	bool torchPressed = false;
	float lastTorchToggleTime = 0.0f;
	float torchToggleDelay = 0.25f;

	int width;
	int height;

	float speed = 10000.f;
	float sensitivity = 100.0f;
	float modelViewSpeed = 100.f;

	float yaw = -90.f;
	float pitch = 0.f;


	float FOV = 70.f;
	void increaseFOV(float value);
	void decreaseFOV(float value);

	float deltaTime;
	float lastFrame;

	bool cameraFocusMode = false;
	glm::vec3 target;
	float modelViewDistance;
	void resetOrientation(glm::vec3 target, float modelViewDistance);

};