// Spring 2020

#include "PickingExample.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>
#include <iostream>

using namespace glm;

static bool show_gui = true;

//----------------------------------------------------------------------------------------
// Constructor
PickingExample::PickingExample()
	: m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0)
{
}

//----------------------------------------------------------------------------------------
// Destructor
PickingExample::~PickingExample()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void PickingExample::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj")
	});

	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(0.0,1.0);

	for( size_t idx = 0; idx < 100; ++idx ) {
		glm::mat4 T = glm::translate( glm::mat4(), glm::vec3(
			(distribution(generator) - 0.5) * 5.0, 
			(distribution(generator) - 0.5) * 5.0, 
			distribution(generator) * -5.0 - 5.0 ) );
		glm::vec3 col( distribution(generator), distribution(generator), distribution(generator) );

		xforms.push_back( T );
		cols.push_back( col );
		selected.push_back( false );
	}

	do_picking = false;

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void PickingExample::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void PickingExample::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void PickingExample::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void PickingExample::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void PickingExample::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void PickingExample::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void PickingExample::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void PickingExample::uploadCommonSceneUniforms()
{
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, do_picking ? 1 : 0 );

		//-- Set LightSource uniform for the scene:
		if( !do_picking ) {
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;

			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void PickingExample::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void PickingExample::guiLogic()
{
	if( !show_gui ) {
		return;
	}

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


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void PickingExample::updateShaderUniforms(
		const glm::mat4& MV, unsigned int idx, const glm::vec3& col )
{
	m_shader.enable();

	//-- Set ModelView matrix:
	GLint location = m_shader.getUniformLocation("ModelView");
	mat4 modelView = MV;
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
	CHECK_GL_ERRORS;

	if( do_picking ) {
		float r = float(idx&0xff) / 255.0f;
		float g = float((idx>>8)&0xff) / 255.0f;
		float b = float((idx>>16)&0xff) / 255.0f;

		location = m_shader.getUniformLocation("material.kd");
		glUniform3f( location, r, g, b );
		CHECK_GL_ERRORS;
	} else {
		//-- Set NormMatrix:
		location = m_shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;

		//-- Set Material values:
		location = m_shader.getUniformLocation("material.kd");
		glUniform3fv(location, 1, value_ptr(col));
		CHECK_GL_ERRORS;
	}

	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void PickingExample::draw() {
	glEnable( GL_DEPTH_TEST );
	glBindVertexArray( m_vao_meshData );

	for( size_t idx = 0; idx < xforms.size(); ++idx ) {
		const glm::mat4& T = xforms[idx];
		glm::vec3 col = cols[idx];
		if( selected[idx] ) {
			col = glm::vec3( 1.0, 1.0, 0.0 );
		}

		updateShaderUniforms( m_view * T, (unsigned int)idx, col );
		BatchInfo batchInfo = m_batchInfoMap[ "cube" ];

		m_shader.enable();
			glDrawArrays( GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices );
		m_shader.disable();
	}

	glBindVertexArray( 0 );
	glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void PickingExample::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool PickingExample::cursorEnterWindowEvent (
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
bool PickingExample::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool PickingExample::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos( m_window, &xpos, &ypos );

		do_picking = true;

		uploadCommonSceneUniforms();
		glClearColor(1.0, 1.0, 1.0, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor(0.35, 0.35, 0.35, 1.0);

		draw();

		// I don't know if these are really necessary anymore.
		// glFlush();
		// glFinish();

		CHECK_GL_ERRORS;

		// Ugly -- FB coordinates might be different than Window coordinates
		// (e.g., on a retina display).  Must compensate.
		xpos *= double(m_framebufferWidth) / double(m_windowWidth);
		// WTF, don't know why I have to measure y relative to the bottom of
		// the window in this case.
		ypos = m_windowHeight - ypos;
		ypos *= double(m_framebufferHeight) / double(m_windowHeight);

		GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
		// A bit ugly -- don't want to swap the just-drawn false colours
		// to the screen, so read from the back buffer.
		glReadBuffer( GL_BACK );
		// Actually read the pixel at the mouse location.
		glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
		CHECK_GL_ERRORS;

		// Reassemble the object ID.
		unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

		if( what < xforms.size() ) {
			selected[what] = !selected[what];
		}

		do_picking = false;

		CHECK_GL_ERRORS;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool PickingExample::mouseScrollEvent (
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
bool PickingExample::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool PickingExample::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
