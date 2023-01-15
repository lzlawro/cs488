// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

// The number of instances to be rendered
const GLsizei kNumInstances = 4;

class InstancedExample : public CS488Window {
public:
	InstancedExample();
	virtual ~InstancedExample();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	void createShaderProgram();
	void accquireShaderAtribLocations();
	void uploadVertexDataToVbo();
	void createInstancedDataVbos();
	void mapVboDataToShaderAttributeLocation();
	void uploadInstancedDataToVbos();

	ShaderProgram m_shader;

	GLuint m_vao_triangle;         // Vertex Array Object
	GLuint m_vbo_triangle_verts;   // Vertex Buffer Object

	//-- VBOs to hold instanced Data:
	GLuint m_vbo_instanced_colors;
	GLuint m_vbo_instanced_transform;

	//-- Vertex Attribute Index Locations:
	GLuint m_position_attrib_location;
	GLuint m_ins_color_attrib_location;
	GLuint m_ins_transform_attrib_location;
};
