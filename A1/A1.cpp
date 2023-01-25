// Termm--Fall 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	

	// DELETE FROM HERE...
	Maze m(DIM);
	m.digMaze();
	m.printMaze();
	// ...TO HERE
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	// Build cube shader
	m_cube_shader.generateProgramObject();
	m_cube_shader.attachVertexShader(
		getAssetFilePath("CubeVertexShader.vs").c_str());
	m_cube_shader.attachFragmentShader(
		getAssetFilePath("CubeFragmentShader.fs").c_str());
	m_cube_shader.link();

	initGrid();
	initCube();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

//----------------------------------------------------------------------------------------
/*
 * Compile glsl shader language string, 
 * create shader object and returns its id
 */
GLuint A1::compileShader(std::string shader, GLenum type) {
	const char *shaderCode = shader.c_str();
	GLuint shaderId = glCreateShader(type);

	if (shaderId == 0) {
		// If shaderId is zero, it means glCreateShader
		// failed to create shader object
		cout << "Error creating shader object!" << endl;
		return 0;
	}

	// Attach source code to this shader object
	glShaderSource(shaderId, 1, &shaderCode, NULL);
	glCompileShader(shaderId);

	GLint compileStatus;

	// Check for compilation status
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

	if (!compileStatus) {
		// If compilation is not successful
		int length;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
		char *cMessage = new char[length];

		// Get additional info about the failure
		glGetShaderInfoLog(shaderId, length, &length, cMessage);
		cout << "Cannot compile shader: " << cMessage;

		delete [] cMessage;

		glDeleteShader(shaderId);

		return 0;
	}

	return shaderId;

}

void A1::initCube() 
{
	//----------------------------------------------------------------------------------------
	// Specify vertices

	// Eight 3-dimensional vertices
	const vector<GLfloat> *vertexData = new vector<GLfloat> {
		-1, 0, 0,

		0, 0, 0, 

		-1, 0, -1, 

		0, 0, -1, 

		-1, 1, 0, 
		
		0, 1, 0, 

		-1, 1, -1, 
		
		0, 1, -1,

		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
	};

	// Set things up on GPU
	glGenVertexArrays(1, &m_cube_vao);
	glBindVertexArray(m_cube_vao);

	// Start generating Vertex Buffer Objects (VBO)
	glGenBuffers(1, &m_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL_FLOAT)*vertexData->size(),
		vertexData->data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_cube_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Data for IBO
	const vector<GLuint> indexBufferData = {
		0, 2, 3, 0, 1, 3,
		0, 4, 5, 0, 1, 5, 
		0, 4, 6, 0, 1, 6, 
		7, 5, 4, 7, 6, 4, 
		7, 5, 1, 7, 3, 1, 
		7, 6, 2, 7, 3, 2
	};

	// Set up the IBO
	glGenBuffers(1, &m_cube_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cube_ibo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBufferData.size() * sizeof(GLuint), 
		indexBufferData.data(),
		GL_STATIC_DRAW
	);

	// GL_STATIC_DRAW means the information will not be changed
	// while the program is running

	// Layout established, generated VBOs
	// Actually get to the data

	// Vertex information
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE,
						  sizeof(GL_FLOAT)*3,
						  (GLvoid*) 0);

	// Color information
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE,
						  sizeof(GL_FLOAT)*3,
						  (GLvoid*) (sizeof(GL_FLOAT)*3*8));

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	//----------------------------------------------------------------------------------------
	// Create shader object
	/*
	string vertexShaderSource = R"(
		#version 330 core

		layout(location=0) in vec3 position;
		layout(location=1) in vec3 vertexColors;

		out vec3 v_vertexColors;

		void main()
		{
			v_vertexColors = vertexColors;

			gl_Position = vec4(position.x, position.y, position.z, 1.0f);
		}
	)";

	string fragmentShaderSource = R"(
		#version 330 core

		in vec3 v_vertexColors;

		out vec4 color;

		void main()
		{
			color = vec4(v_vertexColors.r,
						v_vertexColors.g, 
						v_vertexColors.b,
						1.0f);
		}
	)";
	*/

	//----------------------------------------------------------------------------------------

	delete vertexData;

	CHECK_GL_ERRORS; 
}

void A1::initGrid()
{
	// Total of the grid is 18*18, live area is 16*16
	// There are 19 * 19 vertices in a layer
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
		
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the grid vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			// Select this colour.
		}
		ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes (?)

		// Highlight the active square.
	m_shader.disable();

	// m_cube_shader.enable();
	m_cube_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Draw the cubes
		glBindVertexArray(m_cube_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
		glDrawArrays(GL_TRIANGLES, 0, 3);

	// m_cube_shader.disable();
	m_cube_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 ); 
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.

		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);

			eventHandled = true;
		}
	}

	return eventHandled;
}
