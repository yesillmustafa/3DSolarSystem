#include "Camera.h"

Camera::Camera(int width, int height)
	: width(width), height(height)
{
}
glm::vec3 Camera::getPosition()
{
	return Position;
}

glm::vec3 Camera::getOrientation()
{
	return Orientation;
}

glm::vec3 Camera::getUp()
{
	return Up;
}

float Camera::getFOV()
{
	return FOV;
}

void Camera::setFOV(float value)
{
	FOV = value;
}



void Camera::processInputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Check if ImGui is capturing the mouse or keyboard inputs
	ImGuiIO& io = ImGui::GetIO();

	float adjustedSpeed = speed * deltaTime;

	if (!cameraFocusMode)
	{
		// Handles key inputs
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			Position += adjustedSpeed * Orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			Position += adjustedSpeed * -glm::normalize(glm::cross(Orientation, Up));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			Position += adjustedSpeed * -Orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			Position += adjustedSpeed * glm::normalize(glm::cross(Orientation, Up));
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			Position += adjustedSpeed * Up;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if (!ImGui::IsAnyItemHovered())
				Position += adjustedSpeed * -Up;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			speed = 30000.f;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		{
			speed = 10000.f;
		}



		if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {

			// Handles mouse inputs
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				// Hides mouse cursor
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

				// Prevents camera from jumping on the first click
				if (firstClick)
				{
					glfwSetCursorPos(window, (width / 2), (height / 2));
					firstClick = false;
				}

				// Stores the coordinates of the cursor
				double mouseX;
				double mouseY;
				// Fetches the coordinates of the cursor
				glfwGetCursorPos(window, &mouseX, &mouseY);

				// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
				// and then "transforms" them into degrees 
				float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
				float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

				// Calculates upcoming vertical change in the Orientation
				glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

				// Decides whether or not the next vertical Orientation is legal or not
				if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
				{
					Orientation = newOrientation;
				}

				// Rotates the Orientation left and right
				Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

				// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
				glfwSetCursorPos(window, (width / 2), (height / 2));
			}
			else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			{
				// Unhides cursor since camera is not looking around anymore
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				// Makes sure the next time the camera looks around it doesn't jump
				firstClick = true;
			}
		}
	}
	else {
		// Arcball camera controlled by keyboard
		float adjustedModelViewSpeed = modelViewSpeed * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pitch += 1.f * adjustedModelViewSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			pitch -= 1.f * adjustedModelViewSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			yaw += 1.f * adjustedModelViewSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			yaw -= 1.f * adjustedModelViewSpeed;
		}


		// Limits the pitch
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		// Calculate new camera position
		float radPitch = glm::radians(pitch);
		float radYaw = glm::radians(yaw);

		Position.x = target.x + modelViewDistance * cos(radPitch) * cos(radYaw);
		Position.y = target.y + modelViewDistance * sin(radPitch);
		Position.z = target.z + modelViewDistance * cos(radPitch) * sin(radYaw);

		// Update orientation
		Orientation = glm::normalize(target - Position);
		// Update up vector
		Right = glm::normalize(glm::cross(Orientation, glm::vec3(0.0f, 1.0f, 0.0f)));
		Up = glm::normalize(glm::cross(Right, Orientation));

	}


	// FOV
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
		increaseFOV(0.5f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
		decreaseFOV(0.5f);

	// TORCH
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && (currentFrame - lastTorchToggleTime) >= torchToggleDelay)
	{
		lastTorchToggleTime = currentFrame;
		torchPressed = !torchPressed;
	}
}

void Camera::setPosition(const glm::vec3& position)
{
	Position = position;
}

void Camera::orientCamera(float pitch, float yaw)
{
	Orientation = glm::rotate(Orientation, glm::radians(pitch), glm::normalize(glm::cross(Orientation, Up)));
	Orientation = glm::rotate(Orientation, glm::radians(yaw), Up);
}

void Camera::moveAndOrientCamera(glm::vec3 target, float modelViewDistance) {
	this->target = target;
	this->modelViewDistance = modelViewDistance;

	// Yeni kamera pozisyonunu hesapla
	Position.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.x;
	Position.y = sin(glm::radians(pitch)) * modelViewDistance + target.y;
	Position.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.z;

	// Yön vektörlerini güncelle
	Orientation = glm::normalize(target - Position);
	Right = glm::normalize(glm::cross(Orientation, Up));
	Up = glm::normalize(glm::cross(Right, Orientation));
}

void Camera::setCameraMode(bool value)
{
	cameraFocusMode = value;
	if (!cameraFocusMode) resetOrientation(target, modelViewDistance);
}

bool Camera::getCameraMode()
{
	return cameraFocusMode;
}

bool Camera::isTorchPressed()
{
	return torchPressed;
}

void Camera::activateTorch()
{
	torchPressed = true;
}
void Camera::deactivateTorch()
{
	torchPressed = false;
}



void Camera::increaseFOV(float value)
{
	float newValue = FOV + (value * deltaTime * 100);
	if (newValue < 70)	FOV = newValue;


}

void Camera::decreaseFOV(float value)
{
	float newValue = FOV - (value * deltaTime * 100);
	if (newValue > 15) FOV = newValue;
}

void Camera::resetOrientation(glm::vec3 target, float modelViewDistance)
{
	yaw = -90.f;
	pitch = 0.0f;
	// Yeni kamera pozisyonunu hesapla
	Position.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.x;
	Position.y = sin(glm::radians(pitch)) * modelViewDistance + target.y;
	Position.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.z;

	Orientation = glm::normalize(target - Position);
	Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Right = glm::normalize(glm::cross(Orientation, Up));
}
