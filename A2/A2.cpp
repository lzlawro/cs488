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
	m_lookfrom = vec3(6, 7, 8);

	m_up = vec3(0, 1, 0);

	m_far = 20.0f;
	m_near = 1.0f;
	m_fov = radians(30.0f);

	m_aspect = 1.0f;

	m_prev_xpos = 0.0f;

	current_mode = ROTATE_MODEL;

	M_translate_rotate = M_scale = mat4x4(1.0f);

	P = V_view = mat4x4(1.0f);

	V_translate_rotate = mat4x4(1.0f);

	vz = (m_lookat - m_lookfrom) / length(m_lookat - m_lookfrom);
	vx = cross(m_up, vz) / length(m_up * vz);
	vy = cross(vz, vx);

	m_window_lb = vec2(-1.0f, -1.0f);
	m_window_rt = vec2(1.0f, 1.0f);

	m_viewport_lb = 0.90f * vec2(-1.0f, -1.0f);
	m_viewport_rt = 0.90f * vec2(1.0f, 1.0f);

}

void A2::clearMotion(int button)
{

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
		const vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const vec2 & V0,   // Line Start (NDC coordinate)
		const vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

// bool A2::isTrivialAccept(bool *c1, bool *c2) {
// 	bool c[6];
// 	for (int i = 0; i < 6; i++) { c[i] = c1[i] || c2[i]; }

// 	for (int i = 0; i < 6; i++) {
// 		if (c[i] == 1) return false;
// 	}
// 	return true;
// }

// bool A2::isTrivialReject(bool *c1, bool *c2) {
// 	bool c[6];
// 	for (int i = 0; i < 6; i++) { c[i] = c1[i] && c2[i]; }

// 	for (int i = 0; i < 6; i++) {
// 		if (c[i] == 1) return true;
// 	}

// 	return false;
// }

bool A2::isTrivialAccept(array<bool, 6> &c1, array<bool, 6> &c2) {
	array<bool, 6> c;

	for (int i = 0; i < 6; i++) { c[i] = c1[i] || c2[i]; }

	for (int i = 0; i < 6; i++) {
		if (c[i] == 1) return false;
	}
	return true;
}
bool A2::isTrivialReject(array<bool, 6> &c1, array<bool, 6> &c2) {
	array<bool, 6> c;

	for (int i = 0; i < 6; i++) { c[i] = c1[i] && c2[i]; }

	for (int i = 0; i < 6; i++) {
		if (c[i] == 1) return true;
	}

	return false;
}

void A2::setEndpointCode(vec4 *p1, vec4 *p2, array<bool, 6> &c1, array<bool, 6> &c2) {
	c1[0] = (p1->w + p1->x < 0.0f) ? 1 : 0;
	c1[1] = (p1->w - p1->x < 0.0f) ? 1 : 0;
	c1[2] = (p1->w + p1->y < 0.0f) ? 1 : 0;
	c1[3] = (p1->w - p1->y < 0.0f) ? 1 : 0;
	c1[4] = (p1->w + p1->z < 0.0f) ? 1 : 0;
	c1[5] = (p1->w - p1->z < 0.0f) ? 1 : 0;

	c2[0] = (p2->w + p2->x < 0.0f) ? 1 : 0;
	c2[1] = (p2->w - p2->x < 0.0f) ? 1 : 0;
	c2[2] = (p2->w + p2->y < 0.0f) ? 1 : 0;
	c2[3] = (p2->w - p2->y < 0.0f) ? 1 : 0;
	c2[4] = (p2->w + p2->z < 0.0f) ? 1 : 0;
	c2[5] = (p2->w - p2->z < 0.0f) ? 1 : 0;
}

float A2::calculateIntersection(vec4 *p1, vec4 *p2, int i) {
	float B1, B2;

	switch (i) {
		case 0:
			B1 = p1->w + p1->x;
			B2 = p2->w + p2->x;
			break;
		case 1:
			B1 = p1->w - p1->x;
			B2 = p2->w - p2->x;
			break;
		case 2:
			B1 = p1->w + p1->y;
			B2 = p2->w + p2->y;
			break;
		case 3:
			B1 = p1->w - p1->y;
			B2 = p2->w - p2->y;
			break;
		case 4:
			B1 = p1->w + p1->z;
			B2 = p2->w + p2->z;
			break;
		case 5:
			B1 = p1->w - p1->z;
			B2 = p2->w - p2->z;
			break;
	}

	float coefficient = B1 / (B1 - B2);

	return coefficient;
}

bool A2::clipLine(pair<vec4, vec4> &line) {

	vec4 *p1 = &(line.first);
	vec4 *p2 = &(line.second);

	vec4 p1_new = *p1;
	vec4 p2_new = *p2;

	array<bool, 6> c1 = {0};
	array<bool, 6> c2 = {0};

	setEndpointCode(p1, p2, c1, c2);

	if (isTrivialReject(c1, c2)) {
		return true;
	}

	if (isTrivialAccept(c1, c2)) {
		return false;
	}

	// P(a) = (1-a)*p1 - a*p2
	// If not trivial accept or reject, then the line has to cross two planes
	for (int i = 0; i < 6; i++) {
		if (c1[i] == 1) {
			float coefficient = calculateIntersection(p1, p2, i);
			p1_new = (1-coefficient) * (*p1) + coefficient * (*p2);
		}
	}

	for (int i = 0; i < 6; i++) {
		if (c2[i] == 1) {
			float coefficient = calculateIntersection(p1, p2, i);
			p2_new = (1-coefficient) * (*p1) + coefficient * (*p2);
		}
	}

	*p1 = p1_new;
	*p2 = p2_new;

	return false;
}
/*
 * Note: I had to create temporary std::array variables inside the applogic() function, I could not
 * simply add array fields in A2.hpp, because however I tried to update the array fields, The arrays
 * would always have garbage values
 * Question: is adding temporary variables like this OK? Can this be more efficient?
 */

//----------------------------------------------------------------------------------------
void A2::clipping(array<bool, 12> &cubeRejected, 
				  array<bool, 3> &cubeGnomonRejected, 
				  array<bool, 3> &worldGnomonRejected) {

	for (int i = 0; i < 12; i++) {
		bool trivial_reject = clipLine(cubeClipped[i]);
		if (trivial_reject) cubeRejected[i] = true;
	}

	for (int i = 0; i < 3; i++) {
		bool trivial_reject = clipLine(cubeGnomonClipped[i]);
		if (trivial_reject) cubeGnomonRejected[i] = true;
	}

	for (int i = 0; i < 3; i++) {
		bool trivial_reject = clipLine(worldGnomonClipped[i]);
		if (trivial_reject) worldGnomonRejected[i] = true;
	}
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
	// Init / Reset view

	mat4 T = transpose(mat4x4(
		1,	0,	0,	-m_lookfrom.x,
		0,	1,	0,	-m_lookfrom.y,
		0,	0,	1,	-m_lookfrom.z,
		0,	0,	0,	1
	));

	mat4 R = transpose(mat4x4(
		vx[0],	vx[1],	vx[2],	0,
		vy[0],	vy[1],	vy[2],	0,
		vz[0],	vz[1],	vz[2],	0,
		0,		0,		0,		1
	));

	V_view = R * T;

	//----------------------------------------------------------------------------------------
	// Projection
	P = transpose(mat4x4(
		(cos(m_fov/2)/sin(m_fov/2)) / m_aspect,	0,	0,	0,
		0,	cos(m_fov/2)/sin(m_fov/2),	0,	0,
		0,	0,	(m_far + m_near) / (m_far - m_near),	
		(-2.0f * m_far * m_near) / (m_far - m_near),
		0,	0,	1,	0
	));

	//----------------------------------------------------------------------------------------
	// Apply matrices
	
	for (int i = 0; i < 8; i++) {
		cubeProj[i] = P*V_translate_rotate*V_view*M_translate_rotate*M_scale*cubeModel[i];
	}

	for (int i = 0; i < 4; i++) {
		cubeGnomonProj[i] = P*V_translate_rotate*V_view*M_translate_rotate*cubeGnomon[i];
	}

	for (int i = 0; i < 4; i++) {
		worldGnomonProj[i] = P*V_translate_rotate*V_view*worldGnomon[i];
	}
	//----------------------------------------------------------------------------------------
	// Upload pairs of points
	int edgeIndex = 0;
	for (int i = 0; i <= 4; i += 4) {
		cubeClipped[edgeIndex++] = {cubeProj[0+i], cubeProj[1+i]};
		cubeClipped[edgeIndex++] = {cubeProj[1+i], cubeProj[3+i]};
		cubeClipped[edgeIndex++] = {cubeProj[3+i], cubeProj[2+i]};
		cubeClipped[edgeIndex++] = {cubeProj[2+i], cubeProj[0+i]};
	}

	edgeIndex = 8;
	for (int i = 0; i < 4; i++) {
		cubeClipped[edgeIndex++] = {cubeProj[0+i], cubeProj[4+i]};
	}

	edgeIndex = 0; 
	for (int i = 1; i <= 3; i++) { cubeGnomonClipped[edgeIndex++] = {cubeGnomonProj[0], cubeGnomonProj[i]}; }

	edgeIndex = 0; 
	for (int i = 1; i <= 3; i++) { worldGnomonClipped[edgeIndex++] = {worldGnomonProj[0], worldGnomonProj[i]}; }
	//----------------------------------------------------------------------------------------
	// Clip the lines
	array<bool, 12> cubeRejected = {0};
	array<bool, 3> cubeGnomonRejected = {0};
	array<bool, 3> worldGnomonRejected = {0};
	clipping(cubeRejected, cubeGnomonRejected, worldGnomonRejected);

	//----------------------------------------------------------------------------------------
	// Normalize

	for (int i = 0; i < 12; i++) { 
		normalize(cubeClipped[i].first); 
		normalize(cubeClipped[i].second);}
	for (int i = 0; i < 4; i++) { normalize(cubeGnomonClipped[i].first); normalize(cubeGnomonClipped[i].second);}
	for (int i = 0; i < 4; i++) { normalize(worldGnomonClipped[i].first); normalize(worldGnomonClipped[i].second);}

	//----------------------------------------------------------------------------------------
	// Proj to window
	for (int i = 0; i < 12; i++) {
		cubeFinal[i] = vec4PairToVec2Pair(cubeClipped[i]);
	}

	for (int i = 0; i < 3; i++) {
		cubeGnomonFinal[i] = vec4PairToVec2Pair(cubeGnomonClipped[i]);
		worldGnomonFinal[i] = vec4PairToVec2Pair(worldGnomonClipped[i]);
	}

	//----------------------------------------------------------------------------------------
	// Window to viewport  

	for (int i = 0; i < 12; i++) {
		cubeFinal[i] = windowToViewport(cubeFinal[i]);
	}

	for (int i = 0; i < 3; i++) {
		cubeGnomonFinal[i] = windowToViewport(cubeGnomonFinal[i]);

		worldGnomonFinal[i] = windowToViewport(worldGnomonFinal[i]);
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

	for (int i = 0; i < 12; i++) {
		if (!cubeRejected[i])
			drawLine(cubeFinal[i].first, cubeFinal[i].second);
	}

	//----------------------------------------------------------------------------------------
	// Draw the cube gnomon

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
	if (!cubeGnomonRejected[0])
		drawLine(cubeGnomonFinal[0].first, cubeGnomonFinal[0].second);

	setLineColour(vec3(0.0f, 1.0f, 0.0f));
	if (!cubeGnomonRejected[1])
		drawLine(cubeGnomonFinal[1].first, cubeGnomonFinal[1].second);

	setLineColour(vec3(0.0f, 0.0f, 1.0f));
	if (!cubeGnomonRejected[2])
		drawLine(cubeGnomonFinal[2].first, cubeGnomonFinal[2].second);

	//----------------------------------------------------------------------------------------
	// Draw world gnomon

	setLineColour(vec3(0.0f, 1.0f, 1.0f));
	if (!worldGnomonRejected[0])
		drawLine(worldGnomonFinal[0].first, worldGnomonFinal[0].second);

	setLineColour(vec3(1.0f, 0.0f, 1.0f));
	if (!worldGnomonRejected[1])
		drawLine(worldGnomonFinal[1].first, worldGnomonFinal[1].second);

	setLineColour(vec3(1.0f, 1.0f, 0.0f));
	if (!worldGnomonRejected[2])
		drawLine(worldGnomonFinal[2].first, worldGnomonFinal[2].second);

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
		if( ImGui::Button( "Reset               (A)" ) ) {
			reset();
		}
		if( ImGui::Button( "Quit Application    (Q)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text("Near: %.1f, Far: %.1f", m_near, m_far);
		ImGui::Text("FOV: %.1f", degrees(m_fov));

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

void A2::normalize(vec4 &v) {
	v = vec4(v[0]/v[3], 
			v[1]/v[3],
			v[2]/v[3],
			1.0f);
}

void A2::updateModelRotation(double xPos, double yPos) {
	//----------------------------------------------------------------------------------------
	// Rotate

	float theta = (xPos - m_prev_xpos) / 300.0f;

	mat4 Rz = mat4x4(1.0f);
	mat4 Rx = mat4x4(1.0f);
	mat4 Ry = mat4x4(1.0f);

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		Rx = transpose(mat4x4(
			1,	0,	0,	0,
			0,	cos(theta),	-sin(theta),	0,
			0,	sin(theta),	cos(theta),	0,
			0,	0,	0,	1
		));
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		Ry = transpose(mat4x4(
			cos(theta),	0,	sin(theta),	0,
			0,	1,	0,	0,
			-sin(theta),	0,	cos(theta),	0,
			0,	0,	0,	1
		));
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		Rz = transpose(mat4x4(
			cos(theta),	-sin(theta),	0,	0,
			sin(theta),	cos(theta),	0,	0,
			0,	0,	1,	0,
			0,	0,	0,	1
		));
	}

	M_translate_rotate = M_translate_rotate * Rz * Ry * Rx;
}

void A2::updateModelScale(double xPos, double yPos) {
	mat4 S = mat4x4(1.0f);

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
	mat4 T = mat4x4(1.0f);

	float delta_xyz = (xPos - m_prev_xpos) / 200.0f;

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

	mat4 Rz = mat4x4(1.0f);
	mat4 Rx = mat4x4(1.0f);
	mat4 Ry = mat4x4(1.0f);

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		Rx = transpose(mat4x4(
			1,	0,	0,	0,
			0,	cos(theta),	-sin(theta),	0,
			0,	sin(theta),	cos(theta),	0,
			0,	0,	0,	1
		));

		V_translate_rotate = Rx * V_translate_rotate;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		Ry = transpose(mat4x4(
			cos(theta),	0,	sin(theta),	0,
			0,	1,	0,	0,
			-sin(theta),	0,	cos(theta),	0,
			0,	0,	0,	1
		));

		V_translate_rotate = Ry * V_translate_rotate;
	}

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		Rz = transpose(mat4x4(
			cos(theta),	-sin(theta),	0,	0,
			sin(theta),	cos(theta),	0,	0,
			0,	0,	1,	0,
			0,	0,	0,	1
		));

		V_translate_rotate = Rz * V_translate_rotate;
	}
}

void A2::updateViewTranslation(double xPos, double yPos) {
	mat4 T = mat4x4(1.0f);

	float delta_xyz = (xPos - m_prev_xpos) / 200.0f;

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		T[3][0] = delta_xyz;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		T[3][1] = delta_xyz;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		T[3][2] = delta_xyz;
	}

	V_translate_rotate = T * V_translate_rotate;
}

void A2::updatePerspective(double xPos, double yPos) {
	float offset = (xPos - m_prev_xpos) / 300.0f;

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		if (m_fov + offset >= radians(5.0f) && m_fov + offset <= radians(160.0f)) {
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
	vec2 clickedPosition = vec2(
		((m_window_rt.x - m_window_lb.x) / (m_windowWidth )) * 
				xPos + m_window_lb.x, 
		-(((m_window_rt.y - m_window_lb.y) / (m_windowHeight )) * 
				yPos + m_window_lb.y)
	);

	vec2 midPoint = 0.5f * (m_viewport_rt + m_viewport_lb);

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

void A2::printMessages() {
	
		cout << "---------------------------------Debug message---------------------------------" << endl;
		cout << endl;
		for (int i = 0; i < 12; i++) {
		cout << to_string(cubeClipped[i].first) << ", " << to_string(cubeClipped[i].second) << endl;
		}
		cout << endl;
		cout << "---------------------------------Debug message---------------------------------" << endl;
}

pair<vec2, vec2> A2::vec4PairToVec2Pair(pair<vec4, vec4> vec4_pair) {
	return {
		vec2(vec4_pair.first.x, vec4_pair.first.y), 
		vec2(vec4_pair.second.x, vec4_pair.second.y)
		};
}

pair<vec2, vec2> A2::windowToViewport(pair<vec2, vec2> vec2_pair) {
		float Lw = m_window_rt.x - m_window_lb.x, Hw = m_window_rt.y - m_window_lb.y,
		  Lv = m_viewport_rt.x - m_viewport_lb.x, Hv = m_viewport_rt.y - m_viewport_lb.y;

	return {
		vec2(
			(Lv / Lw) * (vec2_pair.first.x - m_window_lb.x) + m_viewport_lb.x, 
			(Hv / Hw) * (vec2_pair.first.y - m_window_lb.y) + m_viewport_lb.y
		),
		vec2(
			(Lv / Lw) * (vec2_pair.second.x - m_window_lb.x) + m_viewport_lb.x, 
			(Hv / Hw) * (vec2_pair.second.y - m_window_lb.y) + m_viewport_lb.y
		)
	};
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
		if (key == GLFW_KEY_B) {
			printMessages();
			eventHandled = true;
		}
	}

	return eventHandled;
}
