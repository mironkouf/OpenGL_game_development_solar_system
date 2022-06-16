// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include <windows.h>

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(10, 10, 30);

glm::vec3 getPosition() {
	//glm::vec3 positionN = position + glm::vec3(0,0,0);		// gia na ksekinaei 10 monades mprosta apo thn kamera mas 
	return position;
}

// Initial horizontal angle : toward -Z
float horizontalAngle = 0.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 30.0f; // 30 units / second

void computeMatricesFromInputs() {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Up vector
	glm::vec3 up = glm::vec3(0, 0, 1);


	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction = glm::vec3(0.0, 0.0, 0.0);
	direction = glm::normalize(position - direction);


	// Left-Right vector
	glm::vec3 right = glm::normalize(glm::cross(up, direction));

	// Up-Down vector
	glm::vec3 down = glm::normalize(glm::cross(right, direction));

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {		// kinhsh ston z PROS TA PISW
		position += direction * deltaTime * speed;
	}
	// Move backward
	else if ((glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {		// kinhsh ston z PROS TO KENTRO THS PYRAMIDAS
		position -= direction * deltaTime * speed;
	}
	// Strafe left
	if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) &&	// kinhsh gyrw apo ton y ARISTERA
		(GetKeyState(0x14) == 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
		position -= right * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
		position -= right * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		position -= right * deltaTime * speed;
	}
	// Strafe right
	if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) &&	// kinhsh gyrw apo ton y DEKSIA
		(GetKeyState(0x14) == 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
		position += right * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
		position += right * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		position += right * deltaTime * speed;
	}
	// Move up
	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) &&	// kinhsh gyrw apo ton x PANW
		(GetKeyState(0x14) == 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
		position -= down * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
		position -= down * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		position -= down * deltaTime * speed;
	}
	// Move down
	if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) &&	// kinhsh gyrw apo ton x KATW
		(GetKeyState(0x14) == 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
		position += down * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
		position += down * deltaTime * speed;
	}
	else if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) &&
		(GetKeyState(0x14) != 0) &&
		(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		position += down * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 1:1 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 1.0f / 1.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		glm::vec3(0, 0, 0), // and looks here
		up // Head is up (0,0,1)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
