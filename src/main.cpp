/*  by Alun Evans 2016 LaSalle (aevanss@salleurl.edu)

Model View Projection matrices, and camera movement using keys and mouse.

WASD moves camera, click-drag left mouse button to rotate

*/

//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "glfunctions.h" //include all OpenGL stuff
#include "Shader.h" // class to compile shaders
#include "imageloader.h"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;
using namespace glm;

// Default window size, in pixels
int g_ViewportWidth = 512; int g_ViewportHeight = 512;

// background colour - a GLM 3-component vector
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f);

GLuint g_ShaderProgram = 0; //shader identifier
GLuint g_bg_ShaderProgram = 0; //shader identifier
GLuint g_Vao = 0; //vao
GLuint g_NumTriangles = 0; //  Numbre of triangles we are painting.

GLuint texture_id;
GLuint bg_texture_id;
GLuint sun_texture_id;

						   //global variables used for camera movement
int key_flags[] = { 0, 0, 0, 0 }; //w, a, s, d
float mouse_coords[] = { 0.0, 0.0 }; // x, y
vec3 cam_pos(0, 0, 10); //camera always starts at center of world
vec3 cam_target(0, 0, -1); //camera always starts looking down z-axis
float cam_pitch = 0; // up/down
float cam_yaw = 0; //left/right

				   //you may need to change these variables, depending on your system
float MOVE_SPEED = 0.02f;
float LOOK_SPEED = 0.02f;

vec3 earth_pos(5, 0, -10);
float earth_angle = 0;
float sun_angle = 0;

//LIGHTS
vec3 g_light_dir(1,1,1);


// ------------------------------------------------------------------------------------------
// This function loads the teapot object from file, 
// associates it with a shader and stores it in a VAO.
// ------------------------------------------------------------------------------------------
void createGeometry()
{
	//load obj file
	string inputfile = "sphere.obj";
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string err;
	bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str());

	//check for errors
	cout << "# of shapes    : " << shapes.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;

	//get data
	GLfloat* uvs = &(shapes[0].mesh.texcoords[0]);
	GLuint uvs_size = shapes[0].mesh.texcoords.size() * sizeof(GLfloat);
	GLfloat* normals = &(shapes[0].mesh.normals[0]);
	GLuint normals_size = shapes[0].mesh.normals.size() * sizeof(GLfloat);
	GLfloat* vertices = &(shapes[0].mesh.positions[0]);
	GLuint vertices_size = shapes[0].mesh.positions.size() * sizeof(GLfloat);
	GLuint* indices = &(shapes[0].mesh.indices[0]);
	GLuint indices_size = shapes[0].mesh.indices.size() * sizeof(GLuint);
	g_NumTriangles = shapes[0].mesh.indices.size() / 3;

	// Create the VAO where we store all geometry (stored in g_Vao)
	g_Vao = gl_createAndBindVAO();

	//create vertex buffer for positions, colors, and indices
	gl_createAndBindAttribute(vertices, vertices_size, g_ShaderProgram, "a_vertex", 3);
	gl_createAndBindAttribute(uvs, uvs_size, g_ShaderProgram, "a_uv", 2);
	gl_createAndBindAttribute(normals, normals_size, g_ShaderProgram, "a_normals", 3);
	gl_createIndexBuffer(indices, indices_size);

	//unbind everything
	gl_unbindVAO();

	Image* image = loadBMP("earthmap1k.bmp");
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
	earth_angle = 0.0;

	Image* sun_image = loadBMP("sunmap.bmp");
	glGenTextures(1, &sun_texture_id);
	glBindTexture(GL_TEXTURE_2D, sun_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sun_image->width, sun_image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, sun_image->pixels);

	Image* bg_image = loadBMP("milkyway.bmp");
	glGenTextures(1, &bg_texture_id);
	glBindTexture(GL_TEXTURE_2D, bg_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg_image->width, bg_image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, bg_image->pixels);
}

// ------------------------------------------------------------------------------------------
// Draw the teapot
// ------------------------------------------------------------------------------------------
void drawEarth() {

	// activate shader and VAO
	glUseProgram(g_ShaderProgram);

	GLuint u_texture = glGetUniformLocation(g_ShaderProgram, "u_texture");
	glUniform1i(u_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	//find uniform locations (note this could be done once only, 
	//using globals variables or a struct, and after compiling the shader
	GLuint u_model = glGetUniformLocation(g_ShaderProgram, "u_model");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram, "u_view");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram, "u_projection");
	GLuint u_color = glGetUniformLocation(g_ShaderProgram, "u_color");
	GLuint u_light_dir = glGetUniformLocation(g_ShaderProgram, "u_light_dir");
	GLuint u_light_color = glGetUniformLocation(g_ShaderProgram, "u_light_color");
	GLuint u_cam_pos = glGetUniformLocation(g_ShaderProgram, "u_cam_pos");
	GLuint u_shininess = glGetUniformLocation(g_ShaderProgram, "u_shininess");
	GLuint u_ambient = glGetUniformLocation(g_ShaderProgram, "u_ambient");


	//set MVP
	mat4 translate_matrix = translate(mat4(1.0f),earth_pos);
	translate_matrix = scale(translate_matrix, vec3(0.5, 0.5, 0.5));
	translate_matrix = rotate(translate_matrix, earth_angle, vec3(0, 1, 0));
	mat4 view_matrix = lookAt(cam_pos, cam_target, vec3(0, 1, 0)); //cam_pos and cam_target set in update!
	mat4 projection_matrix = glm::perspective(60.0f, ((float)g_ViewportWidth / (float)g_ViewportHeight), 0.1f, 500.0f);

	//send all values to shader
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(translate_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniform3f(u_color, 0.0, 1.0, 0.0);
	glUniform3f(u_light_dir, g_light_dir.x, g_light_dir.y, g_light_dir.z);
	glUniform3f(u_light_color, 1.0, 1.0, 1.0);
	glUniform3f(u_cam_pos, cam_pos.x, cam_pos.y, cam_pos.z);
	glUniform1f(u_shininess, 30.0);
	glUniform1f(u_ambient, 0.1);


	//bind VAO, draw, unbind
	gl_bindVAO(g_Vao);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();

	//unbind shader
	glUseProgram(0);
}

void drawSun() {

	// activate shader and VAO
	glUseProgram(g_bg_ShaderProgram);

	GLuint u_texture = glGetUniformLocation(g_ShaderProgram, "u_texture");
	glUniform1i(u_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sun_texture_id);

	//find uniform locations (note this could be done once only, 
	//using globals variables or a struct, and after compiling the shader
	GLuint u_model = glGetUniformLocation(g_bg_ShaderProgram, "u_model");
	GLuint u_view = glGetUniformLocation(g_bg_ShaderProgram, "u_view");
	GLuint u_projection = glGetUniformLocation(g_bg_ShaderProgram, "u_projection");
	GLuint u_color = glGetUniformLocation(g_bg_ShaderProgram, "u_color");
	//GLuint u_light_dir = glGetUniformLocation(g_bg_ShaderProgram, "u_light_dir");

	//set MVP
	mat4 translate_matrix = translate(mat4(1.0f), vec3(0.0, 0.0, 0.0));
	translate_matrix = rotate(translate_matrix, -sun_angle, vec3(0, 1, 0));
	mat4 view_matrix = lookAt(cam_pos, cam_target, vec3(0, 1, 0)); //cam_pos and cam_target set in update!
	mat4 projection_matrix = glm::perspective(60.0f, ((float)g_ViewportWidth / (float)g_ViewportHeight), 0.1f, 500.0f);


	//send all values to shader
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(translate_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniform3f(u_color, 0.0, 1.0, 0.0);
	//glUniform3f(u_light_dir, g_light_dir.x, g_light_dir.y, g_light_dir.z);


	//bind VAO, draw, unbind
	gl_bindVAO(g_Vao);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();

	//unbind shader
	glUseProgram(0);
}

void drawMilkyWay() {

	// activate shader and VAO
	glUseProgram(g_bg_ShaderProgram);

	GLuint u_texture = glGetUniformLocation(g_bg_ShaderProgram, "u_texture");
	glUniform1i(u_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bg_texture_id);

	//find uniform locations (note this could be done once only, 
	//using globals variables or a struct, and after compiling the shader
	GLuint u_model = glGetUniformLocation(g_bg_ShaderProgram, "u_model");
	GLuint u_view = glGetUniformLocation(g_bg_ShaderProgram, "u_view");
	GLuint u_projection = glGetUniformLocation(g_bg_ShaderProgram, "u_projection");
	GLuint u_color = glGetUniformLocation(g_bg_ShaderProgram, "u_color");
	//GLuint u_light_dir = glGetUniformLocation(g_bg_ShaderProgram, "u_light_dir");

	//set MVP
	mat4 translate_matrix = translate(mat4(1.0f), cam_pos);
	//translate_matrix = scale(translate_matrix, vec3(1.0, 1.0, 1.0));
	mat4 view_matrix = lookAt(cam_pos, cam_target, vec3(0, 1, 0)); //cam_pos and cam_target set in update!
	mat4 projection_matrix = glm::perspective(60.0f, ((float)g_ViewportWidth / (float)g_ViewportHeight), 0.1f, 50.0f);


	//send all values to shader
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(translate_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniform3f(u_color, 0.0, 1.0, 0.0);
	//glUniform3f(u_light_dir, g_light_dir.x, g_light_dir.y, g_light_dir.z);
	

	//bind VAO, draw, unbind
	gl_bindVAO(g_Vao);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();

	//unbind shader
	glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// This function actually draws to screen
// ------------------------------------------------------------------------------------------
void onDisplay()
{
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	drawMilkyWay();


	//SUN
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	drawSun();

	//EARTH
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	drawEarth();

	// Swap the buffers so back buffer is on screen 
	glutSwapBuffers();
}


// ------------------------------------------------------------------------------------------
// Load all resources (shaders and geometry in this case)
// ------------------------------------------------------------------------------------------
void loadResources()
{
	// Load shaders. Mac uses older version of OpenGL and needs
	// slighly different shaders
#ifdef __APPLE__
	Shader simpleShader("shader.vsh", "shader.fsh");
#else
	// In windows the path is relative to main.cpp location
	Shader simpleShader("shader.vert", "shader.frag");
	Shader bg_simpleShader("shader.vert", "bg_shader.frag");
#endif
	g_ShaderProgram = simpleShader.program;
	g_bg_ShaderProgram = bg_simpleShader.program;

	// create geometry for teapot
	createGeometry();
}

// --------------------------------------------------------------
// sets boolean flag to know if we are pressing keys or not
// --------------------------------------------------------------
void onKeyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		// exit the programme
		exit(EXIT_SUCCESS);
		break;
	case 'w':
		key_flags[0] = 1;
		break;
	case 'a':
		key_flags[1] = 1;
		break;
	case 's':
		key_flags[2] = 1;
		break;
	case 'd':
		key_flags[3] = 1;
		break;
	}
}


// --------------------------------------------------------------
// de-sets boolean flags for keys
// --------------------------------------------------------------
void onKeyUp(unsigned char key, int x, int y)
{
	switch (key)
	{

	case 'w':
		key_flags[0] = 0;
		break;
	case 'a':
		key_flags[1] = 0;
		break;
	case 's':
		key_flags[2] = 0;
		break;
	case 'd':
		key_flags[3] = 0;
		break;
	}
}

// --------------------------------------------------------------
// always update mouse coords, regardless of whether we are clicking or not
// --------------------------------------------------------------
void onMouseMove(int x, int y) {

	//save coords
	mouse_coords[0] = x;
	mouse_coords[1] = y;
}

// --------------------------------------------------------------
// only update camera pitch and yaw if we click-drag
// --------------------------------------------------------------
void onMouseMoveClick(int x, int y) {

	cam_yaw += (mouse_coords[0] - x) * LOOK_SPEED;
	cam_pitch += (mouse_coords[1] - y) * LOOK_SPEED;

	//save coords
	mouse_coords[0] = x;
	mouse_coords[1] = y;
}

// --------------------------------------------------------------
// rotate camera according to pitch and yaw,
// move camera according to key flags
// then render (glutPostRedisplay)
// --------------------------------------------------------------
void update() {

	//reset camera to look down z-axis
	vec3 initial_look_vector = vec3(0, 0, -1);
	//rotate look vector around y_axis (yaw)
	vec3 first_rotation = rotate(initial_look_vector, cam_yaw*57.19f, vec3(0, 1, 0));
	//rotate x-axis by same amount - this avoids gimbal lock
	vec3 intermediate_axis = rotate(vec3(1, 0, 0), cam_yaw*57.19f, vec3(0, 1, 0));
	//now rotate pitch around intermediate axis
	vec3 final_look_vector = rotate(first_rotation, cam_pitch*57.19f, intermediate_axis);

	//set camera target to be position + our new look vector
	cam_target = cam_pos + final_look_vector;

	//get forward and side vectors for movement
	vec3 forward = cam_target - cam_pos;
	vec3 side = cross(vec3(0, 1, 0), forward);

	// WASD = forward / back / strafe left / strafe right
	if (key_flags[0]) {
		cam_pos = cam_pos + (forward*MOVE_SPEED);
		cam_target = cam_target + (forward*MOVE_SPEED);
	}
	if (key_flags[1]) {
		cam_pos = cam_pos + (side*MOVE_SPEED);
		cam_target = cam_target + (side*MOVE_SPEED);
	}
	if (key_flags[2]) {
		cam_pos = cam_pos - (forward*MOVE_SPEED);
		cam_target = cam_target - (forward*MOVE_SPEED);
	}
	if (key_flags[3]) {
		cam_pos = cam_pos - (side*MOVE_SPEED);
		cam_target = cam_target - (side*MOVE_SPEED);
	}

	//earth_pos = earth_pos - vec3(-0.002, 0, 0.001);
	earth_angle = earth_angle + 0.1;
	sun_angle = sun_angle + 0.05;

	// tell window to render
	onDisplay();
	glutPostRedisplay();
}

// --------------------------------------------------------------
// GLUT callback detects window resize
// --------------------------------------------------------------
void onReshape(int w, int h)
{
	g_ViewportWidth = w;
	g_ViewportHeight = h;
	glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);
}



int main(int argc, char* argv[]) {

	//initalise GLUT - GLUT is a windowing toolkit designed to help us make 
	//OpenGL apps on MS Windows
	glutInit(&argc, argv);
	//screen format and precision
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	//window size
	glutInitWindowSize(g_ViewportWidth, g_ViewportHeight);
	//create the window
	glutCreateWindow("Hello graphics!");

	//Init GLEW - glew allows us to use loads of useful opengl functions, especially on Windows
	//it's not 100% necessary though
#if !defined(__APPLE__)
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return EXIT_FAILURE;
	}
#endif

	// Clear the background colour to our global variable
	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);

	// Load resources
	loadResources();

	//this line is very important, as it tell GLUT what function to execute
	//every frame (which should be our draw function)
	glutDisplayFunc(onDisplay);

	//tell GLUT about all our callback functions
	glutKeyboardUpFunc(onKeyUp);
	glutKeyboardFunc(onKeyDown);
	glutReshapeFunc(onReshape);
	glutPassiveMotionFunc(onMouseMove);
	glutMotionFunc(onMouseMoveClick);
	glutIdleFunc(update);

	//start everything up
	glutMainLoop();

	return EXIT_SUCCESS;
}
