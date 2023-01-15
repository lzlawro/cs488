// Spring 2020

#include "IndexExample.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;


//----------------------------------------------------------------------------------------
// Constructor
IndexExample::IndexExample()
{

}

//----------------------------------------------------------------------------------------
// Destructor
IndexExample::~IndexExample()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void IndexExample::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	enableVertexAttribIndices();

	uploadVertexDataToVbo();

	mapVboDataToShaderAttributeLocation();
}

//----------------------------------------------------------------------------------------
void IndexExample::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void IndexExample::enableVertexAttribIndices()
{
	glGenVertexArrays(1, &m_vao_square);
	glBindVertexArray(m_vao_square);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void IndexExample::uploadVertexDataToVbo()
{
	vec3 squareVertices[] = {
		vec3(-0.5f, -0.5f, 0.0f),  // Vertex 0
		vec3(0.5f, 0.5f, 0.0f),    // Vertex 1
		vec3(-0.5f, 0.5f, 0.0f),   // Vertex 2
		vec3(0.5f, -0.5f, 0.0f),   // Vertex 3
	};

	// Generate a vertex buffer object to hold the squares's vertex data.
	glGenBuffers(1, &m_vbo_square);

	//-- Copy square's vertex data to the vertex buffer:
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_square);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices,
			GL_STATIC_DRAW);
    

	// NOTE: The data type used here for indices is "GLushort", which must
	// match the index data type given to glDrawElements(...), which below
	// in our draw( ) method is GL_UNSIGNED_SHORT.
	// You can change the type, but they must match in both places.
    GLushort triangleIndices[] = {
		    0,1,2,      // Triangle 0
		    0,3,1       // Triangle 1
    };


	// The VAO keeps track of the last bound GL_ELEMENT_ARRAY_BUFFER, which is where
	// the indices will be stored for rendering.  Bind the VAO, and upload indices to
	// the GL_ELEMENT_ARRAY_BUFFER target.
	glBindVertexArray(m_vao_square);
		glGenBuffers(1, &m_ibo_square);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_square);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices,
				GL_STATIC_DRAW);
	glBindVertexArray(0);


	// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void IndexExample::mapVboDataToShaderAttributeLocation()
{
	glBindVertexArray(m_vao_square);

	// Tell GL how to map data from the vertex buffer "m_vbo_square" into the
	// "position" vertex attribute index of our shader program.
	// This mapping information is stored in the Vertex Array Object "m_vao_square".
	// That is why we must bind "m_vao_square" first in the line above, so
	// that "m_vao_square" captures the data mapping issued by the call to
	// glVertexAttribPointer(...).
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_square);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void IndexExample::uploadUniformsToShader()
{
	m_shader.enable();
		// Set the shape's colour:
		{
			GLint uniformLocation_colour = m_shader.getUniformLocation("colour");
			vec3 colour(1.0f, 0.6f, 0.4f);
			glUniform3f(uniformLocation_colour, colour.r, colour.g, colour.b);
		}
	m_shader.disable();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void IndexExample::appLogic()
{
	uploadUniformsToShader();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void IndexExample::guiLogic()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void IndexExample::draw()
{
	glBindVertexArray(m_vao_square);

	m_shader.enable();
		// 6 indices makes up 2 triangles, where each triangle has 3 vertex indices.
		// The 2 triangles form our square to be rendered.
		const GLsizei numIndices = 6;
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, nullptr);
	m_shader.disable();


	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void IndexExample::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool IndexExample::cursorEnterWindowEvent (
		int entered
) {
	return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool IndexExample::mouseMoveEvent(double xPos, double yPos) {
	return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool IndexExample::mouseButtonInputEvent(int button, int actions, int mods) {
	return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool IndexExample::mouseScrollEvent(double xOffSet, double yOffSet) {
	return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool IndexExample::windowResizeEvent(int width, int height) {
	return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool IndexExample::keyInputEvent(int key, int action, int mods) {
	return false;
}
