// Spring 2020

#include "TutorialExample.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <vector>
using namespace std;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;



//----------------------------------------------------------------------------------------
// Constructor
TutorialExample::TutorialExample()
	: m_vao(0),
	  m_vbo(0),
	  m_position_attrib_location(0),
	  m_color_attrib_location(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
TutorialExample::~TutorialExample()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void TutorialExample::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	accquireShaderAtribLocations();

	uploadVertexDataToVbo();

	mapVboDataToShaderAttributeLocations();
}

//----------------------------------------------------------------------------------------
void TutorialExample::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void TutorialExample::accquireShaderAtribLocations()
{
	m_shader.enable();
		m_position_attrib_location = GLuint(m_shader.getAttribLocation( "position" ));
		m_color_attrib_location = GLuint(m_shader.getAttribLocation( "color" ));
	m_shader.disable();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void TutorialExample::uploadVertexDataToVbo()
{
    vec3 red(0.7, 0.3, 0.3);
    vec3 blue(0.3, 0.3, 0.7);
    
	// Vertex positions for triangle vertices.
	vec3 triangleVertices[] = {
        //     Positions            Colors
		vec3(-0.4f, -0.5f, 0.0f),     red,
		vec3(-0.0f, -0.5f, 0.0f),     red,
		vec3(-0.4f,  0.0f, 0.0f),     red,
        
		vec3( 0.0f,  0.2f, 0.0f),     blue,
		vec3( 0.4f,  0.2f, 0.0f),     blue,
		vec3( 0.4f,  0.7f, 0.0f),     blue,
	};

	// Generate a vertex buffer object to hold the triangle's vertex data.
	glGenBuffers(1, &m_vbo);

	//-- Upload triangle vertex data to the vertex buffer:
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(triangleVertices),
                 triangleVertices,
                 GL_STATIC_DRAW
    );


	// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Map data from VBOs into vertex shader attribute locations.
void TutorialExample::mapVboDataToShaderAttributeLocations()
{
	// Generate and bind the VAO that will store the data mapping.
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Map vertex positions from m_vbo into vertex attribute slot
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        GLsizei stride = sizeof(vec3) * 2;
        const GLvoid * offset = 0;
		glVertexAttribPointer(m_position_attrib_location, 3, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const GLvoid *>(offset));

		// Enable attribute location for rendering
		glEnableVertexAttribArray(m_position_attrib_location);
	}

	// Map vertex colors from m_vbo into vertex attribute slot
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        GLsizei stride = sizeof(vec3) * 2;
        GLuint offset = sizeof(vec3);
		glVertexAttribPointer(m_color_attrib_location, 3, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const GLvoid *>(offset));

		// Enable attribute location for rendering
		glEnableVertexAttribArray(m_color_attrib_location);
	}


	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void TutorialExample::appLogic()
{
	// A good place to change per instnace VBO data using glBufferSubData(...).
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void TutorialExample::guiLogic()
{

}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void TutorialExample::draw()
{
	glBindVertexArray(m_vao);

    GLsizei numVertices = 6;
	m_shader.enable();
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	m_shader.disable();


	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void TutorialExample::cleanup()
{

}
