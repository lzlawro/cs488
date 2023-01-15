// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

class TutorialExample : public CS488Window {
public:
	TutorialExample();
	virtual ~TutorialExample();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	void createShaderProgram();
	void accquireShaderAtribLocations();
	void uploadVertexDataToVbo();
	void mapVboDataToShaderAttributeLocations();
    void drawTriangle();

	ShaderProgram m_shader;

	GLuint m_vao;   // Vertex Array Object
	GLuint m_vbo;   // Vertex Buffer Object

	//-- Vertex Attribute Index Locations:
	GLuint m_position_attrib_location;
	GLuint m_color_attrib_location;
};
