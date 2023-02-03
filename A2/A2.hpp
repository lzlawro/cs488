// Termm--Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;

	GLuint index;
	GLsizei numVertices;
};

class CoordinateSystem4 {
public:

	std::vector<glm::vec4> bases;
	glm::vec4 origin;
};

class CoordinateSystem3 {
public:

	std::vector<glm::vec3> bases;
	glm::vec3 origin;
};

class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

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
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void reset();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	CoordinateSystem4 m_mcs;
	CoordinateSystem4 m_wcs;
	CoordinateSystem4 m_vcs;

	glm::vec3 m_lookat;
	glm::vec3 m_lookfrom;
	glm::vec3 m_up;

	glm::vec3 vz;
	glm::vec3 vx;
	glm::vec3 vy;

	glm::vec4 p_prime[8];

	glm::mat4 P;
	glm::mat4 V;
	glm::mat4 M;

	std::vector<glm::vec4> cubeModel;

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

};
