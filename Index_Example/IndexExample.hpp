// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

class IndexExample : public CS488Window {
public:
	IndexExample();
	virtual ~IndexExample();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void uploadVertexDataToVbo();
	void mapVboDataToShaderAttributeLocation();
	void uploadUniformsToShader();

	GLuint m_vao_square; // Vertex Array Object
	GLuint m_vbo_square; // Vertex Buffer Object
	GLuint m_ibo_square; // Index Buffer Object
	ShaderProgram m_shader;

};
