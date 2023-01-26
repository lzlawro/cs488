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

static const size_t WALL = 0;
static const size_t FLOOR = 1;
static const size_t AVATAR = 2;

static const size_t DIM = 16;
static const size_t WALL_HEIGHT_MIN = 0;
static const size_t WALL_HEIGHT_MAX = 10;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
	, prev_col( -1 )
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;

	
	// ( col_uni, 0, 0, 0 );
	wall_colour[0] = 0.0f;
	wall_colour[1] = 0.0f;
	wall_colour[2] = 0.0f;
	
	// (col_uni, 0, 0, 0.5f);
	floor_colour[0] = 0.0f;
	floor_colour[1] = 0.0f;
	floor_colour[2] = 0.5f;

	// ( col_uni, 1, 0, 0 );
	avatar_colour[0] = 1.0f;
	avatar_colour[1] = 0.0f;
	avatar_colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{
	delete m_maze;
}

void A1::resetParameters() {
	m_prev_xpos = 0.0f;
	m_is_dragging = false;
	m_persistence = 0.0f;
	m_rotation = 0.0f;
	m_scale = 1.0f;
}

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
	// cout << "Random number seed = " << rseed << endl;
	

	// DELETE FROM HERE...
	m_maze = new Maze(DIM);
	// m_maze->digMaze();
	// m_maze->printMaze();
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

	// Initially set wall height to 1
	m_wall_height = 1;

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	resetParameters();

	// Generate new vaos and vbos for the cubes
	// To draw multiple cubes with a single set of vao & vbo

	initGrid();
	initFloor();
	initWall();
	initAvatar();

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

void A1::initFloor() {
	const vector<GLfloat> vertexData = {
		0.0f, 0.0f, 0.0f,
		(GLfloat)(DIM), 0.0f, 0.0f,
		(GLfloat)(DIM), 0.0f, (GLfloat)(DIM),
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, (GLfloat)(DIM),
		(GLfloat)(DIM), 0.0f, (GLfloat)(DIM),
	};

	// Set things up on GPU
	glGenVertexArrays(1, &m_floor_vao);
	glBindVertexArray(m_floor_vao);

	// Start generating Vertex Buffer Objects (VBO)
	glGenBuffers(1, &m_floor_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_floor_vbo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL_FLOAT)*vertexData.size(),
		vertexData.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Vertex information
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE,
						  sizeof(GL_FLOAT)*3,
						  0);

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS; 
}

void A1::initWall()
{
	//----------------------------------------------------------------------------------------
	// Specify vertices

	// Eight 3-dimensional vertices
	const vector<GLfloat> cubeData = 
	{
		// Vertex data
		0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 0.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,
	};

	vector<GLfloat> vertexData = {};

	for (int x = 0; x < DIM; x++) {
		for (int z = 0; z < DIM; z++) {
			for (int y = 0; y <= WALL_HEIGHT_MAX; y++) {
				vector<GLfloat> temp = cubeData;

				for (int i = 0; i < temp.size(); i++) {
					if (i % 3 == 0) temp[i] += (GLfloat)x;
					else if (i % 3 == 1) temp[i] += (GLfloat)y;
					else if (i % 3 == 2) temp[i] += (GLfloat)z;
				}

				vertexData.insert(vertexData.end(), temp.begin(), temp.end());
			}
		}
	}

	// Set things up on GPU
	glGenVertexArrays(1, &m_wall_vao_2);
	glBindVertexArray(m_wall_vao_2);

	// Start generating Vertex Buffer Objects (VBO)
	glGenBuffers(1, &m_wall_vbo_2);
	glBindBuffer(GL_ARRAY_BUFFER, m_wall_vbo_2);
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL_FLOAT)*vertexData.size(),
		vertexData.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Vertex information
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE,
						  sizeof(GL_FLOAT)*3,
						  0);

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

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

void A1::updateWallSlow()
{
	//----------------------------------------------------------------------------------------
	// Specify vertices

	// Eight 3-dimensional vertices
	const vector<GLfloat> cubeData = 
	{
		// Vertex data
		0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 0.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,
	};

	vector<GLfloat> vertexData = {};

	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (m_maze->getValue(j, i) == 1) {
				vector<GLfloat> temp = cubeData;
				for (int k = 0; k < cubeData.size(); k++) {
					if (k % 3 == 0) temp[k] += (GLfloat)(i);
					else if (k % 3 == 2) temp[k] += (GLfloat)(j);
				}

				for (int h = 0; h < m_wall_height; h++) {
					vertexData.insert(vertexData.end(), temp.begin(), temp.end());
					for (int y = 0; y < cubeData.size(); y++) {
						if (y % 3 == 1) temp[y] += 1.0f;
					}
				}
			}
		}
	}
	

	// Generate new vaos and vbos for the cubes
	// To draw multiple cubes with a single set of vao & vbo

	// Set things up on GPU
	glGenVertexArrays(1, &m_wall_vao);
	glBindVertexArray(m_wall_vao);

	// Start generating Vertex Buffer Objects (VBO)
	glGenBuffers(1, &m_wall_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_wall_vbo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL_FLOAT)*vertexData.size(),
		vertexData.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Why does deleting this portion not matter?
	//----------------------------------------------------------------------------------------
	// // Specify the means of extracting the position values properly.
	// GLint posAttrib = m_cube_shader.getAttribLocation( "position" );
	// glEnableVertexAttribArray( posAttrib );
	// glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
	//----------------------------------------------------------------------------------------

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
						  // (GLvoid*) (sizeof(GL_FLOAT)*(m_ncubes++)*(18*6))
						  0
						  );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
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

	CHECK_GL_ERRORS; 
}

void A1::initAvatar() {
	vector<GLfloat> cubeData = 
	{
		// Vertex data
		0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 0.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,
	};

	vector<GLfloat> vertexData = {};

	for (auto it = cubeData.begin(); it != cubeData.end(); it++) {
		*it *= 0.75f;
	}

	for (int i = 0; i < cubeData.size(); i++) {
		if (i % 3 == 0 || i % 3 == 2) cubeData[i] += 0.125;
	}

	for (int x = 0; x < DIM; x++) {
		for (int z = 0; z < DIM; z++) {
			vector<GLfloat> temp = cubeData;

			for (int i = 0; i < temp.size(); i++) {
				if (i % 3 == 0) temp[i] += x;
				else if (i % 3 == 2) temp[i] += z;
			}

			vertexData.insert(vertexData.end(), temp.begin(), temp.end());
		}
	}

	glGenVertexArrays(1, &m_avatar_vao);
	glBindVertexArray(m_avatar_vao);

	// Start generating Vertex Buffer Objects (VBO)
	glGenBuffers(1, &m_avatar_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_avatar_vbo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL_FLOAT)*vertexData.size(),
		vertexData.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Vertex information
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE,
						  sizeof(GL_FLOAT)*3,
						  0
						  );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS; 
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...

	// updateWallSlow();
	m_rotation += m_persistence;
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

		ImGui::ColorEdit3( "##Colour", colour );

		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "Wall Colour", &current_col, WALL ) ) {
			// Select this colour.
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Floor Colour", &current_col, FLOOR ) ) {
			// Select this colour.
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Avatar Colour", &current_col, AVATAR ) ) {
			// Select this colour.
		}

		prev_col = current_col;

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

void A1::drawWallSlow()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	
	// m_cube_shader.enable();
	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Draw the cubes
		glBindVertexArray(m_wall_vao);
		glUniform3f( col_uni, 0, 0, 0 );
		glBindBuffer(GL_ARRAY_BUFFER, m_wall_vbo);
		// 12 is the number of triangles needed for each cube
		glDrawArrays(GL_TRIANGLES, 0, 3*12*m_wall_height*DIM*DIM*DIM);

	// m_cube_shader.disable();
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 ); 
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::scale(W, vec3(m_scale, m_scale, m_scale));
	W = glm::rotate(W, 
					2.0f * pi<float>() * (m_rotation), 
					vec3(0.0f, 1.0f, 0.0f));
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

		// Draw floor

		glBindVertexArray(m_floor_vao);
		// if (current_col == FLOOR && current_col != prev_col) {
		// 	glUniform3f(col_uni, 
		// 				colour[0], 
		// 				colour[1],
		// 				colour[2]);
		// } else {
			glUniform3f(col_uni, 
						floor_colour[0], 
						floor_colour[1],
						floor_colour[2]);
		// }
		glBindBuffer(GL_ARRAY_BUFFER, m_floor_vbo);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Draw wall
		glBindVertexArray(m_wall_vao_2);
		// if (current_col == WALL && current_col != prev_col) {
		// 	glUniform3f(col_uni, 
		// 				colour[0], 
		// 				colour[1],
		// 				colour[2]);
		// } else {
			glUniform3f(col_uni, 
						wall_colour[0], 
						wall_colour[1],
						wall_colour[2]);
		// }
		glBindBuffer(GL_ARRAY_BUFFER, m_wall_vbo_2);
		
		for (int x = 0; x < DIM; x++) {
			for (int z = 0; z < DIM; z++) {
				if (m_maze->getValue(z, x) == 1) {
					glDrawArrays(GL_TRIANGLES, 
								(DIM*x + z)*(WALL_HEIGHT_MAX+1)*(3*12), 
								m_wall_height*(3*12));
				}
			}
		}

		// Actually perform the draw
	
		// m_cube_shader.disable();

		// Draw avatar
		glBindVertexArray(m_avatar_vao);
		// if (current_col == AVATAR && current_col != prev_col) {
		// 	glUniform3f(col_uni, 
		// 				colour[0], 
		// 				colour[1],
		// 				colour[2]);
		// } else {
			glUniform3f(col_uni, 
						avatar_colour[0], 
						avatar_colour[1],
						avatar_colour[2]);
		// }
		glBindBuffer(GL_ARRAY_BUFFER, m_avatar_vbo);
		// 12 is the number of triangles needed for each cube
		// 3*12 is for a single cube
		// x and z are coordinates with respect to grid

		int r = m_maze->getAvatarR();
		int c = m_maze->getAvatarC();

		glDrawArrays(GL_TRIANGLES, (c+r*(DIM))*(3*12), (3*12));

	// m_cube_shader.disable();
	m_shader.disable();

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
{
	
}

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
		if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_1)) {
			m_rotation += (xPos - m_prev_xpos) / 16000.0f;
			m_persistence += (xPos - m_prev_xpos) / 32000.0f;

			m_is_dragging = true;
		}
		m_prev_xpos = xPos;
		eventHandled = true;
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
		if (actions == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
			m_is_dragging = false;

			eventHandled = true;

			// cout << "m1 pressed" << endl;
		}
		if (actions == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
			if (m_is_dragging) {
				m_is_dragging = !m_is_dragging;
			} else {
				m_persistence = 0.0f;
			}

			eventHandled = true;

			// cout << "m1 released" << endl;

		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	if (yOffSet > 0.0f) {
		if (m_scale <= 5.0f) {
			if (m_scale >= 1.0f) {
				m_scale += 0.25f;
			} else if (m_scale >= 0.4f) {
				m_scale += 0.1f;
			} else {
				m_scale += 0.025f;
			}
		}
	} else {
		if (m_scale >= 0.01f) {
			if (m_scale >= 1.0f) {
				m_scale -= 0.25f;
			} else if (m_scale >= 0.1f) {
				m_scale -= 0.1f;
			} else {
				m_scale -= 0.005f;
			}
		}
	}

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

		if (key == GLFW_KEY_D) {
			m_maze->setAvatarValue(0,0);
			m_maze->digMaze();
			m_maze->printMaze();

			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			resetParameters();
			m_maze->setAvatarValue(0,0);
			m_maze->reset();
			m_maze->printMaze();
			printf("Avatar position: (%d, %d)\n", 
				m_maze->getAvatarR(), m_maze->getAvatarC());

			eventHandled = true;
		}

		if (key == GLFW_KEY_SPACE) {
			if (m_wall_height < WALL_HEIGHT_MAX) {
				++m_wall_height;
			}

			// cout << "Wall height is now: " << m_wall_height << endl;

			eventHandled = true;
		}

		if (key == GLFW_KEY_BACKSPACE) {
			if (m_wall_height > WALL_HEIGHT_MIN) {
				--m_wall_height;
			}

			// cout << "Wall height is now: " << m_wall_height << endl;

			eventHandled = true;
		}

		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN ||
			key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
			if (key == GLFW_KEY_UP) {
				if (mods == GLFW_MOD_SHIFT)
				m_maze->moveAvatarUp(true);
				else
				m_maze->moveAvatarUp(false);
			}

			if (key == GLFW_KEY_DOWN) {
				if (mods == GLFW_MOD_SHIFT)
				m_maze->moveAvatarDown(true);
				else
				m_maze->moveAvatarDown(false);
			}

			if (key == GLFW_KEY_LEFT) {
				if (mods == GLFW_MOD_SHIFT)
				m_maze->moveAvatarLeft(true);
				else
				m_maze->moveAvatarLeft(false);
			}

			if (key == GLFW_KEY_RIGHT) {
				if (mods == GLFW_MOD_SHIFT)
				m_maze->moveAvatarRight(true);
				else
				m_maze->moveAvatarRight(false);
			}

			eventHandled = true;
		}
	}

	return eventHandled;
}
