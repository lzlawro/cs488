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
	m_fov = glm::radians(30.0f);

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

	m_window_lb = vec2(-1.0f, -1.0f);
	m_window_rt = vec2(1.0f, 1.0f);

	m_viewport_lb = 0.90f * vec2(-1.0f, -1.0f);
	m_viewport_rt = 0.90f * vec2(1.0f, 1.0f);

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
		// cubeProj[i] = P*V*M_translate*M_rotate*M_scale*cubeModel[i];
		cubeProj[i] = P*V_translate_rotate*V_view*M_translate_rotate*M_scale*cubeModel[i];

		// Normalize
		cubeProj[i] = vec4(cubeProj[i][0]/cubeProj[i][3], 
							cubeProj[i][1]/cubeProj[i][3],
							cubeProj[i][2]/cubeProj[i][3],
							1.0f);
	}

	for (int i = 0; i < 4; i++) {
		cubeGnomonProj[i] = P*V_translate_rotate*V_view*M_translate_rotate*cubeGnomon[i];

		// Normalize
		cubeGnomonProj[i] = vec4(cubeGnomonProj[i][0]/cubeGnomonProj[i][3], 
							cubeGnomonProj[i][1]/cubeGnomonProj[i][3],
							cubeGnomonProj[i][2]/cubeGnomonProj[i][3],
							1.0f);

	}

	for (int i = 0; i < 4; i++) {
		worldGnomonProj[i] = P*V_translate_rotate*V_view*worldGnomon[i];

		// Normalize
		worldGnomonProj[i] = vec4(worldGnomonProj[i][0]/worldGnomonProj[i][3], 
							worldGnomonProj[i][1]/worldGnomonProj[i][3],
							worldGnomonProj[i][2]/worldGnomonProj[i][3],
							1.0f);

	}

	//----------------------------------------------------------------------------------------
	// Proj to window
	for (int i = 0; i < 8; i++) {
		cubeFinal[i].x = cubeProj[i].x;
		cubeFinal[i].y = cubeProj[i].y;
	}

	for (int i = 0; i < 4; i++) {
		cubeGnomonFinal[i].x = cubeGnomonProj[i].x;
		cubeGnomonFinal[i].y = cubeGnomonProj[i].y;
		worldGnomonFinal[i].x = worldGnomonProj[i].x;
		worldGnomonFinal[i].y = worldGnomonProj[i].y;
	}

	//----------------------------------------------------------------------------------------
	// Window to viewport

	float Lw = m_window_rt.x - m_window_lb.x, Hw = m_window_rt.y - m_window_lb.y,
		  Lv = m_viewport_rt.x - m_viewport_lb.x, Hv = m_viewport_rt.y - m_viewport_lb.y;  

	for (int i = 0; i < 8; i++) {
		cubeFinal[i].x = (Lv / Lw) * (cubeFinal[i].x - m_window_lb.x) + m_viewport_lb.x;
		cubeFinal[i].y = (Hv / Hw) * (cubeFinal[i].y - m_window_lb.y) + m_viewport_lb.y;
	}

	for (int i = 0; i < 4; i++) {
		cubeGnomonFinal[i].x = (Lv / Lw) * (cubeGnomonFinal[i].x - m_window_lb.x) + m_viewport_lb.x;
		cubeGnomonFinal[i].y = (Hv / Hw) * (cubeGnomonFinal[i].y - m_window_lb.y) + m_viewport_lb.y;

		worldGnomonFinal[i].x = (Lv / Lw) * (worldGnomonFinal[i].x - m_window_lb.x) + m_viewport_lb.x;
		worldGnomonFinal[i].y = (Hv / Hw) * (worldGnomonFinal[i].y - m_window_lb.y) + m_viewport_lb.y;
	}

	//----------------------------------------------------------------------------------------
	// Draw viewport
	setLineColour(vec3(0.0f, 0.0f, 0.0f));
	// drawLine(m_viewport_lb, m_viewport_rt);

	drawLine(m_viewport_lb, vec2(m_viewport_lb.x, m_viewport_rt.y));
	drawLine(m_viewport_lb, vec2(m_viewport_rt.x, m_viewport_lb.y));
	drawLine(m_viewport_rt, vec2(m_viewport_lb.x, m_viewport_rt.y));
	drawLine(m_viewport_rt, vec2(m_viewport_rt.x, m_viewport_lb.y));

	//----------------------------------------------------------------------------------------
	// Draw the wireframe cube
	setLineColour(vec3(1.0f, 1.0f, 1.0f));

	for (int i = 0; i <= 4; i += 4) {
		drawLine(cubeFinal[0+i], cubeFinal[1+i]);
		drawLine(cubeFinal[1+i], cubeFinal[3+i]);
		drawLine(cubeFinal[3+i], cubeFinal[2+i]);
		drawLine(cubeFinal[2+i], cubeFinal[0+i]);
	}

	for (int i = 0; i < 4; i++) {
		drawLine(cubeFinal[0+i], cubeFinal[4+i]);
	}

	//----------------------------------------------------------------------------------------
	// Draw the cube gnomon

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
		drawLine(cubeGnomonFinal[0], cubeGnomonFinal[1]);

	setLineColour(vec3(0.0f, 1.0f, 0.0f));
		drawLine(cubeGnomonFinal[0], cubeGnomonFinal[2]);

	setLineColour(vec3(0.0f, 0.0f, 1.0f));
		drawLine(cubeGnomonFinal[0], cubeGnomonFinal[3]);

	//----------------------------------------------------------------------------------------
	// Draw world gnomon
	setLineColour(vec3(0.0f, 1.0f, 1.0f));
		drawLine(worldGnomonFinal[0], worldGnomonFinal[1]);

	setLineColour(vec3(1.0f, 0.0f, 1.0f));
		drawLine(worldGnomonFinal[0], worldGnomonFinal[2]);

	setLineColour(vec3(1.0f, 1.0f, 0.0f));
		drawLine(worldGnomonFinal[0], worldGnomonFinal[3]);

	//----------------------------------------------------------------------------------------

	// // Draw outer square:
	// setLineColour(vec3(1.0f, 0.7f, 0.8f));
	// drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	// drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	// drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	// drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));
	// drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, 0.5f));
	
	// // Draw inner square:
	// setLineColour(vec3(0.2f, 1.0f, 1.0f));
	// drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	// drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	// drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	// drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));

	// //----------------------------------------------------------------------------------------
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

		ImGui::RadioButton("Rotate View        (O)", (int*)&current_mode, ROTATE_VIEW);
		ImGui::RadioButton("Translate View     (E)", (int*)&current_mode, TRANSLATE_VIEW);
		ImGui::RadioButton("Perspective        (P)", (int*)&current_mode, PERSPECTIVE);

		ImGui::RadioButton("Rotate Model       (R)", (int*)&current_mode, ROTATE_MODEL);
		ImGui::RadioButton("Translate Model    (T)", (int*)&current_mode, TRANSLATE_MODEL);
		ImGui::RadioButton("Scale Model        (S)", (int*)&current_mode, SCALE_MODEL);

		ImGui::RadioButton("Viewport           (V)", (int*)&current_mode, VIEWPORT);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text("Near: %.1f, Far: %.1f", m_near, m_far);
		ImGui::Text("FOV: %.1f", glm::degrees(m_fov));

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
				updateViewRotation(xPos, yPos);
				break;
			case TRANSLATE_VIEW:
				updateViewTranslation(xPos, yPos);
				break;
			case PERSPECTIVE:
				updatePerspective(xPos, yPos);
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
				if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
					updateViewport(xPos, yPos);
				}
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

		V_translate_rotate = glm::inverse(Rx) * V_translate_rotate;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		Ry = glm::transpose(mat4x4(
			cos(theta),	0,	sin(theta),	0,
			0,	1,	0,	0,
			-sin(theta),	0,	cos(theta),	0,
			0,	0,	0,	1
		));

		V_translate_rotate = glm::inverse(Ry) * V_translate_rotate;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		Rz = glm::transpose(mat4x4(
			cos(theta),	-sin(theta),	0,	0,
			sin(theta),	cos(theta),	0,	0,
			0,	0,	1,	0,
			0,	0,	0,	1
		));

		V_translate_rotate = glm::inverse(Rz) * V_translate_rotate;
	}
}

void A2::updateViewTranslation(double xPos, double yPos) {
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

	V_translate_rotate = glm::inverse(T) * V_translate_rotate;
}

void A2::updatePerspective(double xPos, double yPos) {
	float offset = (xPos - m_prev_xpos) / 300.0f;

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		if (m_fov + offset >= glm::radians(5.0f) && m_fov + offset <= glm::radians(160.0f)) {
			m_fov += offset;
		}
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		if (m_near + offset > 0.0f && m_near + offset < m_far) {
			m_near += offset;
		}
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		if (m_far + offset > m_near) {
			m_far += offset;
		}
	}
}

void A2::updateViewport(double xPos, double yPos) {
	// TODO: implement this

	// Convert the click's absolute pixel location to NDCS coordinate
	glm::vec2 clickedPosition = vec2(
		((m_window_rt.x - m_window_lb.x) / (m_windowWidth )) * 
				xPos + m_window_lb.x, 
		-(((m_window_rt.y - m_window_lb.y) / (m_windowHeight )) * 
				yPos + m_window_lb.y)
	);

	glm::vec2 midPoint = 0.5f * (m_viewport_rt + m_viewport_lb);

	if (clickedPosition.x < midPoint.x && clickedPosition.y < midPoint.y) {
		m_viewport_lb = clickedPosition;
	} else if (clickedPosition.x >= midPoint.x && clickedPosition.y >= midPoint.y) {
		m_viewport_rt = clickedPosition;
	} else if (clickedPosition.x >= midPoint.x && clickedPosition.y < midPoint.y) {
		m_viewport_rt.x = clickedPosition.x;
		m_viewport_lb.y = clickedPosition.y;
	} else {
		m_viewport_lb.x = clickedPosition.x;
		m_viewport_rt.y = clickedPosition.y;
	}
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
			if (current_mode == VIEWPORT && button == GLFW_MOUSE_BUTTON_LEFT) {
				double xPos  = ImGui::GetMousePos().x;
				double yPos  = ImGui::GetMousePos().y;

				updateViewport(xPos, yPos);
			}

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
		if (key == GLFW_KEY_O) {
			current_mode = ROTATE_VIEW;
			eventHandled = true;
		}
		if (key == GLFW_KEY_E) {
			current_mode = TRANSLATE_VIEW;
			eventHandled = true;
		}
		if (key == GLFW_KEY_P) {
			current_mode = PERSPECTIVE;
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
		if (key == GLFW_KEY_V) {
			current_mode = VIEWPORT;
			eventHandled = true;
		}
	}

	return eventHandled;
}
