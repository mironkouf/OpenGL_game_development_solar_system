// Include standard headers

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <windows.h>
using namespace std;

#include <stdlib.h>
#include <string.h>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "objloader.hpp"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) { // einai gia tis koryfes
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) { //syntetagmenes apo to U,V
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) { //to trigwno kai pou 8a antistoixoun oi koryfes tou(gia to texture)
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, u8"Ηλιακό Σύστημα", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	GLuint textureID[5];
	glGenTextures(5, textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	// Load the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load("sun.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	// Load the texture
	data = stbi_load("planet.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	// Load the texture
	data = stbi_load("meteor.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//glGenerateMipmap(GL_TEXTURE_2D);
	// BONUS 1 planhths
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID[3]);
	// Load the texture
	data = stbi_load("neptune.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// BONUS 2 planhths
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID[4]);
	// Load the texture
	data = stbi_load("earth.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool res = loadOBJ("sun.obj", vertices, uvs, normals);
	std::vector<glm::vec3> vertices2;
	std::vector<glm::vec3> normals2;
	std::vector<glm::vec2> uvs2;
	bool res2 = loadOBJ("planet.obj", vertices2, uvs2, normals2);
	std::vector<glm::vec3> vertices3;
	std::vector<glm::vec3> normals3;
	std::vector<glm::vec2> uvs3;
	bool res3 = loadOBJ("meteor.obj", vertices3, uvs3, normals3);
	std::vector<glm::vec3> vertices4;
	std::vector<glm::vec3> normals4;
	std::vector<glm::vec2> uvs4;
	bool res4 = loadOBJ("planet.obj", vertices4, uvs4, normals4); //bonus1
	std::vector<glm::vec3> vertices5;
	std::vector<glm::vec3> normals5;
	std::vector<glm::vec2> uvs5;
	bool res5 = loadOBJ("planet.obj", vertices5, uvs5, normals5); //bonus2

	// Load it into a VBO
	// buffers gia ton hlio
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// buffers gia ton planhth
	GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);

	GLuint uvbuffer2;
	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2), &uvs2[0], GL_STATIC_DRAW);

	// buffers gia ton komhth
	GLuint vertexbuffer3;
	glGenBuffers(1, &vertexbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
	glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(glm::vec3), &vertices3[0], GL_STATIC_DRAW);

	GLuint uvbuffer3;
	glGenBuffers(1, &uvbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
	glBufferData(GL_ARRAY_BUFFER, uvs3.size() * sizeof(glm::vec2), &uvs3[0], GL_STATIC_DRAW);

	// buffers gia ton BONUS 1 planhth
	GLuint vertexbuffer4;
	glGenBuffers(1, &vertexbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
	glBufferData(GL_ARRAY_BUFFER, vertices4.size() * sizeof(glm::vec3), &vertices4[0], GL_STATIC_DRAW);

	GLuint uvbuffer4;
	glGenBuffers(1, &uvbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
	glBufferData(GL_ARRAY_BUFFER, uvs3.size() * sizeof(glm::vec2), &uvs4[0], GL_STATIC_DRAW);

	// buffers gia ton BONUS 2 planhth
	GLuint vertexbuffer5;
	glGenBuffers(1, &vertexbuffer5);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer5);
	glBufferData(GL_ARRAY_BUFFER, vertices5.size() * sizeof(glm::vec3), &vertices5[0], GL_STATIC_DRAW);

	GLuint uvbuffer5;
	glGenBuffers(1, &uvbuffer5);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer5);
	glBufferData(GL_ARRAY_BUFFER, uvs5.size() * sizeof(glm::vec2), &uvs5[0], GL_STATIC_DRAW);

	float m_scale = 0.0005f; //arxikh timh gia th peristrofh tou planhth
	int count = 0; //flag gia elegxo oti yparxei hdh enas kommhths gia na mh dhmiourgh8ei allos
	int planet = 0; //elegxos an exei ginei sygkroush me ton kommhth gia na e3afanistei o planhths
	int choose = 0; //flag gia na paroume thn arxikh 8esh ths kameras (dhmiourgia kommhth) kai epeita na exoume ane3arthth kinhsh kommhth-kameras
	float m_scale_planet = 0.0005f; //xrhsimopoieitai sthn peristrofh tou planhth gyrw apo ton eauto tou
	float delim = 0.0005f;
	int planet4 = 0; //BONUS 1 elegxos an exei ginei sygkroush me ton kommhth gia na e3afanistei o planhths
	float m_scale2 = 0.0005f; //BONUS 1 arxikh timh gia th peristrofh tou planhth
	float m_scale_planet2 = 0.0005f; // BONUS 1 xrhsimopoieitai sthn peristrofh tou planhth gyrw apo ton eauto tou
	int planet1 = 0; //BONUS 1 elegxos an exei ginei sygkroush me ton kommhth gia na e3afanistei o planhths
	int planet2 = 0; //BONUS 2 elegxos an exei ginei sygkroush me ton kommhth gia na e3afanistei o planhths
	float m_scale3 = 0.0005f; //BONUS 2 arxikh timh gia th peristrofh tou planhth

	do {
		//Hlios
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;


		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID[0]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Dhmiourgia Planhth
		// Use our shader
		glUseProgram(programID);
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, 0.0f, 0.0f));	// gurw apo pou thelw na kanw orbit
		ModelMatrix1 = glm::rotate(ModelMatrix1, m_scale, glm::vec3(0.0f, 1.0f, 0.0f)); //a3onas panw ston opoio 8elw na kanei orbit
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(25.0f, 0.0f, 0.0f));	// h aktina pou thelw na kanei orbit
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, 0.0f, 0.0f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, m_scale_planet, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate gyrw apo ton eauto tou
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID[1]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		if (planet == 0)
			glDrawArrays(GL_TRIANGLES, 0, vertices2.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		//BONUS 1 PLANHTHS
		// Dhmiourgia Planhth
		// Use our shader
		glUseProgram(programID);
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix3 = glm::mat4(1.0);
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(0.0f, 0.0f, 0.0f));	// gurw apo pou thelw na kanw orbit
		ModelMatrix3 = glm::rotate(ModelMatrix3, m_scale2, glm::vec3(0.0f, 1.0f, 0.0f)); //a3onas panw ston opoio 8elw na kanei orbit
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(36.0f, 0.0f, 0.0f));	// h aktina pou thelw na kanei orbit
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(0.0f, 0.0f, 0.0f));
		ModelMatrix3 = glm::rotate(ModelMatrix3, m_scale_planet2, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate gyrw apo ton eauto tou
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix3;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID[3]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		if (planet1 == 0)
			glDrawArrays(GL_TRIANGLES, 0, vertices4.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		//BONUS 2 PLANHTHS
		// Dhmiourgia Planhth
		// Use our shader
		glUseProgram(programID);
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix4 = glm::mat4(1.0);
		ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(0.0f, 0.0f, 0.0f));	// gurw apo pou thelw na kanw orbit
		ModelMatrix4 = glm::rotate(ModelMatrix4, m_scale3, glm::vec3(0.0f, 1.0f, 0.0f)); //a3onas panw ston opoio 8elw na kanei orbit
		ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(47.0f, 0.0f, 0.0f));	// h aktina pou thelw na kanei orbit
		ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(0.0f, 0.0f, 0.0f));
		ModelMatrix4 = glm::rotate(ModelMatrix4, m_scale_planet2, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate gyrw apo ton eauto tou
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix4;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID[4]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer5);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer5);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		if (planet2 == 0)
			glDrawArrays(GL_TRIANGLES, 0, vertices5.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// meteoriths
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || count != 0) { //elegxos an exei path8ei to space h an yparxei hdh meteoriths gia na ton kanei render
			count = 1;
			float speed = 0.01f;
			glm::mat4 ModelMatrix2 = glm::mat4(1.0);
			//dhmiourgia meteorhth
			glUseProgram(programID);
			glm::vec3 positNew;
			if (choose == 0)
			{
				choose = 1;
				glm::vec3 posit = getPosition(); //pairnoume thn 8esh ths kameras gia thn topo8esia dhmiourgias tou kommhth
				positNew = posit; //thn krataw gia 8esh anaforas
				computeMatricesFromInputs();
				ProjectionMatrix = getProjectionMatrix();
				ViewMatrix = getViewMatrix();
				glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f); //orizw thn kateu8ynsh pros to kentro
				direction = glm::normalize(posit - direction); //kanonikopoish ths kateu8ynshs
				posit -= direction * speed; //ypologizw thn epomenh 8esh
				ModelMatrix2 = glm::translate(glm::mat4(1.0), posit); //thn pernaw ston ModelMatrix
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix2;
			}
			else
			{	// gia na mporoume na doume ton meteorith kai na mhn kanei thn kinhsh ths cameras
				computeMatricesFromInputs();
				ProjectionMatrix = getProjectionMatrix();
				ViewMatrix = getViewMatrix();
				glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
				direction = glm::normalize(positNew - direction);
				positNew -= direction * speed;
				//elelgxos an exei xa8ei mesa ston hlio gia na e3afanistei
				if (abs(positNew.x) < 1.0 && abs(positNew.y) < 1.0 && abs(positNew.z) < 1.0) {
					count = 0;
					choose = 0;
				}
				ModelMatrix2 = glm::translate(glm::mat4(1.0), positNew);
				//elegxos an exei xtyphsei ton planhth gia na katastrafoun kai oi 2
				if ((abs(ModelMatrix2[3][0] - ModelMatrix1[3][0]) < 7.0) && (abs(ModelMatrix2[3][1] - ModelMatrix1[3][1]) < 7.0) && (abs(ModelMatrix2[3][2] - ModelMatrix1[3][2]) < 7.0) && planet == 0) {
					bool play = PlaySound("sound.wav", NULL, SND_ASYNC);
					count = 0;
					choose = 0;
					planet = 1;
					glDeleteBuffers(1, &vertexbuffer2);
					glDeleteBuffers(1, &uvbuffer2);
				}
				if ((abs(ModelMatrix2[3][0] - ModelMatrix3[3][0]) < 7.0) && (abs(ModelMatrix2[3][1] - ModelMatrix3[3][1]) < 7.0) && (abs(ModelMatrix2[3][2] - ModelMatrix3[3][2]) < 7.0) && planet1 == 0) {
					bool play = PlaySound("sound.wav", NULL, SND_ASYNC);
					count = 0;
					choose = 0;
					planet1 = 1;
					glDeleteBuffers(1, &vertexbuffer4);
					glDeleteBuffers(1, &uvbuffer4);
				}if ((abs(ModelMatrix2[3][0] - ModelMatrix4[3][0]) < 7.0) && (abs(ModelMatrix2[3][1] - ModelMatrix4[3][1]) < 7.0) && (abs(ModelMatrix2[3][2] - ModelMatrix4[3][2]) < 7.0) && planet2 == 0) {
					bool play = PlaySound("sound.wav", NULL, SND_ASYNC);
					count = 0;
					choose = 0;
					planet2 = 1;
					glDeleteBuffers(1, &vertexbuffer5);
					glDeleteBuffers(1, &uvbuffer5);
				}
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix2;
			}

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID[2]);
			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(TextureID, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);


			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, vertices3.size());

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		m_scale += 0.00005f;
		m_scale2 += 0.00005f;
		m_scale3 += 0.00005f;
		m_scale_planet += delim;
		m_scale_planet2 += 0.0005;

		if ((glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) && //gia thn au3hsh ths taxythtas peristrofhs tou planhth
			(GetKeyState(0x14) == 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
			delim += 0.00005;
		}
		else if ((glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) &&
			(GetKeyState(0x14) != 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
			delim += 0.00005;
		}
		else if ((glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) &&
			(GetKeyState(0x14) != 0) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
			delim += 0.00005;
		}
		if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) && //gia thn meiwsh ths taxythtas peristrofhs tou planhth
			(GetKeyState(0x14) == 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
			if (delim > 0)
				delim -= 0.00005;
		}
		else if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) &&
			(GetKeyState(0x14) != 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
			if (delim > 0)
				delim -= 0.00005;
		}
		else if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) &&
			(GetKeyState(0x14) != 0) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
			if (delim > 0)
				delim -= 0.00005;
		}

		if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && //e3odos apo to programma me to q
			(GetKeyState(0x14) != 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS)) {
			break;
		}
		else if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) &&
			(GetKeyState(0x14) == 0) &&
			(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
			break;
		}
		else if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) &&
			(GetKeyState(0x14) == 0) &&
			(glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
			break;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, textureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}