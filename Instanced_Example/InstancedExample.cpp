// Spring 2020

#include "InstancedExample.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <vector>
using namespace std;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;



//----------------------------------------------------------------------------------------
static inline float rand_float()
{
	return float(rand()) / RAND_MAX;
}


//----------------------------------------------------------------------------------------
// Constructor
InstancedExample::InstancedExample()
	: m_vao_triangle(0),
	  m_vbo_triangle_verts(0),
	  m_vbo_instanced_colors(0),
	  m_vbo_instanced_transform(0),
	  m_position_attrib_location(0),
	  m_ins_color_attrib_location(0),
	  m_ins_transform_attrib_location(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
InstancedExample::~InstancedExample()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void InstancedExample::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	accquireShaderAtribLocations();

	uploadVertexDataToVbo();

	createInstancedDataVbos();
	uploadInstancedDataToVbos();

	mapVboDataToShaderAttributeLocation();

}

//----------------------------------------------------------------------------------------
void InstancedExample::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void InstancedExample::accquireShaderAtribLocations()
{
	m_shader.enable();
		m_position_attrib_location = uint(m_shader.getAttribLocation( "position" ));
		m_ins_color_attrib_location = uint(m_shader.getAttribLocation( "ins_color" ));
		m_ins_transform_attrib_location = uint(m_shader.getAttribLocation("ins_transform"));
	m_shader.disable();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void InstancedExample::uploadVertexDataToVbo()
{
	// Vertex positions for triangle vertices.
	// Default OpenGL coordinates range from -1 to 1 in all directions (x,y,z).
	float half_sqrt3 = float(std::sqrt(3.0)) * 0.5f;
	vec3 triangleVertices[] = {
		// Construct equalaterial triangle
		vec3(0.0f, 0.0f, 0.0f),
		vec3(0.25f, half_sqrt3 * 0.5f, 0.0),
		vec3(0.5f, 0.0f, 0.0f)
	};

	// Construct triangle centroid and move vertices so triangle is centered at origin
	vec3 centroid(0.0f);
	for(const vec3 & v : triangleVertices) {
		centroid += v;
	}
	centroid /= 3.0f;
	for(vec3 & v : triangleVertices) {
		v -= centroid;
	}


	// Generate a vertex buffer object to hold the triangle's vertex data.
	glGenBuffers(1, &m_vbo_triangle_verts);

	//-- Upload triangle vertex data to the vertex buffer:
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_triangle_verts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices,
			GL_STATIC_DRAW);


	// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void InstancedExample::createInstancedDataVbos()
{
	//-- Create VBO storage for instanced colors:
	{
		glGenBuffers(1, &m_vbo_instanced_colors);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_colors);

		// Allocate a buffer large enough to hold all the instanced color data, where
		// each color is a vec3.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kNumInstances, nullptr, GL_STATIC_DRAW);


		// Unbind the buffer target
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	//-- Create VBO storage for instanced transforms:
	{
		glGenBuffers(1, &m_vbo_instanced_transform);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_transform);

		// Allocate a buffer large enough to hold all the instanced transform data, where
		// each transform is a mat4.
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * kNumInstances, nullptr, GL_STATIC_DRAW);


		// Unbind the buffer target
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

}

//----------------------------------------------------------------------------------------
// Map data from VBOs into vertex shader attribute locations.
// Remember it's the VAO that records this mapping of data, not the shader,
// because a single VAO could be used with different shaders within multiple draw calls.
void InstancedExample::mapVboDataToShaderAttributeLocation()
{
	// Generate and bind the VAO that will store the data mapping.
	glGenVertexArrays(1, &m_vao_triangle);
	glBindVertexArray(m_vao_triangle);

	// Map m_vbo_triangle_verts data into the attribute location index given by
	// m_position_attrib_location.
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_triangle_verts);
		glVertexAttribPointer(m_position_attrib_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// Enable attribute location for rendering
		glEnableVertexAttribArray(m_position_attrib_location);
	}

	// Map m_vbo_instanced_colors data into the attribute location index given by
	// m_ins_color_attrib_location.
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_colors);
		glVertexAttribPointer(m_ins_color_attrib_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// Enable attribute location for rendering
		glEnableVertexAttribArray(m_ins_color_attrib_location);

		// Increment the attribute array index only once per instance rather than once
		// per vertex.
		glVertexAttribDivisor(m_ins_color_attrib_location, 1);
	}


	// Map m_vbo_instanced_transforms data into 4 attribute location slots starting
	// at location m_ins_transform_attrib_location.
	// Note that an attribute location index can only have up to 4 elements mapped to it
	// at a time.  Since a mat4 is 4 columns of vec4's, we require 4 attribute locations
	// for the mapping.
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_transform);

		// The number of bytes between the i-th column of two consecutive mat4's
		// within the VBO.
		GLsizei stride = sizeof(mat4);

		// A mat4 attribute variable has 4 location indices, one for each vec4 column.
		for (int i(0); i < 4; ++i) {

			// The start of column-i is sizeof(vec4) * i bytes away from the beginning
			// of the mat4.
			GLvoid * initialOffset = reinterpret_cast<GLvoid *>(i * sizeof(vec4));

			glVertexAttribPointer(m_ins_transform_attrib_location + i, 4, GL_FLOAT,
					GL_FALSE, stride, initialOffset);

			// Enable attribute location for rendering
			glEnableVertexAttribArray(m_ins_transform_attrib_location + i);

			// Increment the attribute array index only once per instance rather than once
			// per vertex.
			glVertexAttribDivisor(m_ins_transform_attrib_location + i, 1);
		}
	}


	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void InstancedExample::uploadInstancedDataToVbos()
{
	//-- Upload per instance colors:
	{
		std::vector<vec3> instanced_colors;
		for (int i(0); i < kNumInstances; ++i) {
			float red (rand_float());
			float green (rand_float());
			float blue (rand_float());

			instanced_colors.push_back(vec3(red, green, blue));
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_colors);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * instanced_colors.size(),
				instanced_colors.data());

		CHECK_GL_ERRORS;
	}


	//-- Upload per instance transforms:
	{
		std::vector<mat4> instanced_transforms;

		mat4 transform1 = glm::translate(mat4(), vec3(-0.3f, 0.3f, 0.0f));

		mat4 transform2 = glm::translate(mat4(), vec3(0.3f, 0.3f, 0.0f));

		mat4 transform3 = glm::translate(mat4(), vec3(0.3f, -0.3f, 0.0f));

		mat4 transform4 = glm::translate(mat4(), vec3(-0.3f, -0.3f, 0.0f));

		instanced_transforms.push_back(transform1);
		instanced_transforms.push_back(transform2);
		instanced_transforms.push_back(transform3);
		instanced_transforms.push_back(transform4);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_instanced_transform);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * instanced_transforms.size(),
				instanced_transforms.data());

		CHECK_GL_ERRORS;
	}


	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void InstancedExample::appLogic()
{
	// A good place to change per instnace VBO data using glBufferSubData(...).
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void InstancedExample::guiLogic()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void InstancedExample::draw()
{
	glBindVertexArray(m_vao_triangle);

	m_shader.enable();
		glDrawArraysInstanced(GL_TRIANGLES, 0, 3, kNumInstances);
	m_shader.disable();


	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void InstancedExample::cleanup()
{

}
