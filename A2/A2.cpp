// Termm--Fall 2023

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/reciprocal.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

void A2::reset()
{
	// Reset parameters
	m_lookat = vec3(0, 0, 0);
	m_lookfrom = vec3(10, 8, 6);

	m_up = vec3(0, 1, 0);

	m_far = 10.0f;
	m_near = 1.0f;
	m_fov = glm::pi<GLfloat>() / 6.0f;

	m_aspect = 1.0f;

	m_prev_xpos = 0.0f;

	current_mode = ROTATE_MODEL;

	memset(m_model_rotation, 0.0f, 3*sizeof(float));
	memset(m_model_translation, 0.0f, 3*sizeof(float));

	M_translate_rotate = M_scale = glm::mat4x4(1.0f);

	P = V_view = glm::mat4x4(1.0f);

	V_translate_rotate = glm::mat4x4(1.0f);

	vz = (m_lookat - m_lookfrom) / glm::length(m_lookat - m_lookfrom);
	vx = glm::cross(m_up, vz) / glm::length(m_up * vz);
	vy = glm::cross(vz, vx);

}

void A2::clearMotion(int button)
{
	m_model_rotation[button] = 0.0f;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	cubeModel = 
		{
			vec4(-1.0f, -1.0f, -1.0f, 1),
			vec4(1.0f, -1.0f, -1.0f, 1),
			vec4(-1.0f, -1.0f, 1.0f, 1),
			vec4(1.0f, -1.0f, 1.0f, 1),
			vec4(-1.0f, 1.0f, -1.0f, 1),
			vec4(1.0f, 1.0f, -1.0f, 1),
			vec4(-1.0f, 1.0f, 1.0f, 1),
			vec4(1.0f, 1.0f, 1.0f, 1)
		};

	cubeGnomon = worldGnomon =
		{
			vec4(0.0f, 0.0f, 0.0f, 1),
			vec4(0.3f, 0.0f, 0.0f, 1),
			vec4(0.0f, 0.3f, 0.0f, 1),
			vec4(0.0f, 0.0f, 0.3f, 1),
		};

	reset();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Spring 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// For now, everything is moved to part of the init method
	// TODO: after producing a cube scene that makes sense,
	//		 move some of the code back here

	//----------------------------------------------------------------------------------------
	// MCS to WCS
	
	//----------------------------------------------------------------------------------------
	// Translate


	//----------------------------------------------------------------------------------------
	// Init / Reset view

	glm::mat4 T = glm::transpose(mat4x4(
		1,	0,	0,	-m_lookfrom.x,
		0,	1,	0,	-m_lookfrom.y,
		0,	0,	1,	-m_lookfrom.z,
		0,	0,	0,	1
	));

	glm::mat4 R = glm::transpose(mat4x4(
		vx[0],	vx[1],	vx[2],	0,
		vy[0],	vy[1],	vy[2],	0,
		vz[0],	vz[1],	vz[2],	0,
		0,		0,		0,		1
	));

	V_view = R * T;

	//----------------------------------------------------------------------------------------
	// Projection
	P = glm::transpose(mat4x4(
		(cos(m_fov/2)/sin(m_fov/2)) / m_aspect,	0,	0,	0,
		0,	cos(m_fov/2)/sin(m_fov/2),	0,	0,
		0,	0,	(m_far + m_near) / (m_far - m_near),	
		(-2.0f * m_far * m_near) / (m_far - m_near),
		0,	0,	1,	0
	));

	//----------------------------------------------------------------------------------------
	// Apply matrices
	
	for (int i = 0; i < 8; i++) {
		// cubeFinal[i] = P*V*M_translate*M_rotate*M_scale*cubeModel[i];
		cubeFinal[i] = P*V_view*M_translate_rotate*M_scale*cubeModel[i];

		// Normalize
		cubeFinal[i] = vec4(cubeFinal[i][0]/cubeFinal[i][3], 
							cubeFinal[i][1]/cubeFinal[i][3],
							cubeFinal[i][2]/cubeFinal[i][3],
							1.0f);
	}

	for (int i = 0; i < 4; i++) {
		cubeGnomonFinal[i] = P*V_view*M_translate_rotate*cubeGnomon[i];

		// Normalize
		cubeGnomonFinal[i] = vec4(cubeGnomonFinal[i][0]/cubeGnomonFinal[i][3], 
							cubeGnomonFinal[i][1]/cubeGnomonFinal[i][3],
							cubeGnomonFinal[i][2]/cubeGnomonFinal[i][3],
							1.0f);
	}

	for (int i = 0; i < 4; i++) {
		worldGnomonFinal[i] = P*V_view*worldGnomon[i];

		// Normalize
		worldGnomonFinal[i] = vec4(worldGnomonFinal[i][0]/worldGnomonFinal[i][3], 
							worldGnomonFinal[i][1]/worldGnomonFinal[i][3],
							worldGnomonFinal[i][2]/worldGnomonFinal[i][3],
							1.0f);
	}

	//----------------------------------------------------------------------------------------
	// Draw the wireframe cube
	setLineColour(vec3(1.0f, 1.0f, 1.0f));

	for (int i = 0; i <= 4; i += 4) {
		drawLine(vec2(cubeFinal[0+i].x, cubeFinal[0+i].y), vec2(cubeFinal[1+i].x, cubeFinal[1+i].y));
		drawLine(vec2(cubeFinal[1+i].x, cubeFinal[1+i].y), vec2(cubeFinal[3+i].x, cubeFinal[3+i].y));
		drawLine(vec2(cubeFinal[3+i].x, cubeFinal[3+i].y), vec2(cubeFinal[2+i].x, cubeFinal[2+i].y));
		drawLine(vec2(cubeFinal[2+i].x, cubeFinal[2+i].y), vec2(cubeFinal[0+i].x, cubeFinal[0+i].y));
	}

	for (int i = 0; i < 4; i++) {
		drawLine(vec2(cubeFinal[0+i].x, cubeFinal[0+i].y), vec2(cubeFinal[4+i].x, cubeFinal[4+i].y));
	}
	//----------------------------------------------------------------------------------------
	// Draw the cube gnomon

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
		drawLine(vec2(cubeGnomonFinal[0].x, cubeGnomonFinal[0].y), 
				 vec2(cubeGnomonFinal[1].x, cubeGnomonFinal[1].y));

	setLineColour(vec3(0.0f, 1.0f, 0.0f));
		drawLine(vec2(cubeGnomonFinal[0].x, cubeGnomonFinal[0].y), 
				 vec2(cubeGnomonFinal[2].x, cubeGnomonFinal[2].y));

	setLineColour(vec3(0.0f, 0.0f, 1.0f));
		drawLine(vec2(cubeGnomonFinal[0].x, cubeGnomonFinal[0].y), 
				 vec2(cubeGnomonFinal[3].x, cubeGnomonFinal[3].y));

	//----------------------------------------------------------------------------------------
	// Draw world gnomon
	setLineColour(vec3(0.0f, 1.0f, 1.0f));
		drawLine(vec2(worldGnomonFinal[0].x, worldGnomonFinal[0].y), 
				 vec2(worldGnomonFinal[1].x, worldGnomonFinal[1].y));

	setLineColour(vec3(1.0f, 0.0f, 1.0f));
		drawLine(vec2(worldGnomonFinal[0].x, worldGnomonFinal[0].y), 
				 vec2(worldGnomonFinal[2].x, worldGnomonFinal[2].y));

	setLineColour(vec3(1.0f, 1.0f, 0.0f));
		drawLine(vec2(worldGnomonFinal[0].x, worldGnomonFinal[0].y), 
				 vec2(worldGnomonFinal[3].x, worldGnomonFinal[3].y));

	//----------------------------------------------------------------------------------------

	// // Draw outer square:
	// setLineColour(vec3(1.0f, 0.7f, 0.8f));
	// drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	// drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	// drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	// drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));
	
	// // Draw inner square:
	// setLineColour(vec3(0.2f, 1.0f, 1.0f));
	// drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	// drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	// drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	// drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));

	//----------------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...

		ImGui::RadioButton("Rotate Model       (R)", (int*)&current_mode, ROTATE_MODEL);
		ImGui::RadioButton("Translate Model    (T)", (int*)&current_mode, TRANSLATE_MODEL);
		ImGui::RadioButton("Scale Model        (S)", (int*)&current_mode, SCALE_MODEL);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text("Near: %.1f, Far: %.1f", m_near, m_far);

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		switch(current_mode) {
			case ROTATE_VIEW:
				// updateViewRotation(xPos, yPos);
				break;
			case TRANSLATE_VIEW:
				// updateViewTranslation(xPos, yPos);
				break;
			case PERSPECTIVE:
				// updatePerspective(xPos, yPos);
				break;
			case ROTATE_MODEL:
				updateModelRotation(xPos, yPos);
				break;
			case SCALE_MODEL:
				updateModelScale(xPos, yPos);
				break;
			case TRANSLATE_MODEL:
				updateModelTranslation(xPos, yPos);
				break;
			case VIEWPORT:
				break;
		}

		eventHandled = true;
		// if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		// 	switch(current_mode) {
		// 		case ROTATE_MODEL:
		// 			m_model_rotation[GLFW_MOUSE_BUTTON_LEFT] = (xPos - m_prev_xpos) / 400.0f;
		// 			break;
		// 	}
		// }
		// if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		// 	switch(current_mode) {
		// 		case ROTATE_MODEL:
		// 			m_model_rotation[GLFW_MOUSE_BUTTON_MIDDLE] = (xPos - m_prev_xpos) / 400.0f;
		// 			break;
		// 	}
		// }
		// if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		// 	switch(current_mode) {
		// 		case ROTATE_MODEL:
		// 			m_model_rotation[GLFW_MOUSE_BUTTON_RIGHT] = (xPos - m_prev_xpos) / 400.0f;
		// 			break;
		// 	}
		// }
	}

	m_prev_xpos = xPos;
	m_prev_ypos = yPos;
	

	// Fill in with event handling code...

	return eventHandled;
}

void A2::updateModelRotation(double xPos, double yPos) {
	//----------------------------------------------------------------------------------------
	// Rotate

	float theta = (xPos - m_prev_xpos) / 300.0f;

	glm::mat4 Rz = glm::mat4x4(1.0f);
	glm::mat4 Rx = glm::mat4x4(1.0f);
	glm::mat4 Ry = glm::mat4x4(1.0f);

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		Rx = glm::transpose(mat4x4(
			1,	0,	0,	0,
			0,	cos(theta),	-sin(theta),	0,
			0,	sin(theta),	cos(theta),	0,
			0,	0,	0,	1
		));
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		Ry = glm::transpose(mat4x4(
			cos(theta),	0,	sin(theta),	0,
			0,	1,	0,	0,
			-sin(theta),	0,	cos(theta),	0,
			0,	0,	0,	1
		));
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		Rz = glm::transpose(mat4x4(
			cos(theta),	-sin(theta),	0,	0,
			sin(theta),	cos(theta),	0,	0,
			0,	0,	1,	0,
			0,	0,	0,	1
		));
	}

	M_translate_rotate = M_translate_rotate * Rz * Ry * Rx;
}

void A2::updateModelScale(double xPos, double yPos) {
	glm::mat4 S = glm::mat4x4(1.0f);

	float s_xyz = (xPos - m_prev_xpos) / 300.0f;

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		if (S[0][0] + s_xyz >= 0.0f) S[0][0] += s_xyz;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		if (S[1][1] + s_xyz >= 0.0f) S[1][1] += s_xyz;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		if (S[2][2] + s_xyz >= 0.0f) S[2][2] += s_xyz;
	}

	M_scale = M_scale * S;
}

void A2::updateModelTranslation(double xPos, double yPos) {
	glm::mat4 T = glm::mat4x4(1.0f);

	float delta_xyz = (xPos - m_prev_xpos) / 300.0f;

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		T[3][0] = delta_xyz;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		T[3][1] = delta_xyz;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		T[3][2] = delta_xyz;
	}

	M_translate_rotate = M_translate_rotate * T;
}

void A2::updateViewRotation(double xPos, double yPos) {
	
}

void A2::updateViewTranslation(double xPos, double yPos) {

}

void A2::updatePerspective(double xPos, double yPos) {

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (actions == GLFW_PRESS) {

			eventHandled = true;
		}

		if (actions == GLFW_RELEASE) {

			eventHandled = true;
		}
	}

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
		if (key == GLFW_KEY_A) {
			reset();
			eventHandled = true;
		}
		if (key == GLFW_KEY_R) {
			current_mode = ROTATE_MODEL;
			eventHandled = true;
		}
		if (key == GLFW_KEY_T) {
			current_mode = TRANSLATE_MODEL;
			eventHandled = true;
		}
		if (key == GLFW_KEY_S) {
			current_mode = SCALE_MODEL;
			eventHandled = true;
		}
	}

	return eventHandled;
}
