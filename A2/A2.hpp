// Termm--Fall 2023

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

enum Mode {
	ROTATE_VIEW, 
	TRANSLATE_VIEW,
	PERSPECTIVE, 
	ROTATE_MODEL,
	TRANSLATE_MODEL,
	SCALE_MODEL,
	VIEWPORT
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
	void clearMotion(int button);

	void updateModelRotation(double xPos, double yPos);
	void updateModelScale(double xPos, double yPos);
	void updateModelTranslation(double xPos, double yPos);
	void updateViewRotation(double xPos, double yPos);
	void updateViewTranslation(double xPos, double yPos);
	void updatePerspective(double xPos, double yPos);
	void updateViewport(double xPos, double yPos);
	void normalize(glm::vec4 &v);
	void printMessages();
	std::pair<glm::vec2, glm::vec2> vec4PairToVec2Pair(std::pair<glm::vec4, glm::vec4> v);
	std::pair<glm::vec2, glm::vec2> windowToViewport(std::pair<glm::vec2, glm::vec2> vec2_pair);

	void clipping(std::array<bool, 12> &cubeRejected, 
				  std::array<bool, 3> &cubeGnomonRejected, 
				  std::array<bool, 3> &worldGnomonRejected);
	bool clipLine(std::pair<glm::vec4, glm::vec4> &line);
	void setEndpointCode(glm::vec4 *p1, glm::vec4 *p2, std::array<bool, 6> &c1, std::array<bool, 6> &c2);
	bool isTrivialAccept(std::array<bool, 6> &c1, std::array<bool, 6> &c2);
	bool isTrivialReject(std::array<bool, 6> &c1, std::array<bool, 6> &c2);
	float calculateIntersection(glm::vec4 *p1, glm::vec4 *p2, int i);

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

	std::vector<glm::vec4> cubeModel;
	std::vector<glm::vec4> cubeGnomon;

	std::vector<glm::vec4> worldGnomon;

	glm::vec3 m_lookat;
	glm::vec3 m_lookfrom;
	glm::vec3 m_up;

	glm::vec2 m_window_lb;
	glm::vec2 m_window_rt;

	glm::vec2 m_viewport_lb;
	glm::vec2 m_viewport_rt;

	// glm::vec2 m_window_lt;
	// glm::vec2 m_window_rb;

	// glm::vec2 m_viewport_lt;
	// glm::vec2 m_viewport_rb;

	GLfloat m_near;
	GLfloat m_far;

	GLfloat m_fov;
	GLfloat m_aspect;

	glm::vec3 vz;
	glm::vec3 vx;
	glm::vec3 vy;

	glm::mat4 P;

	glm::mat4 V_view;
	glm::mat4 V_translate_rotate;

	glm::mat4 M_scale;
	glm::mat4 M_translate_rotate;

	double m_prev_xpos;
	double m_prev_ypos;

	glm::vec4 cubeProj[8];
	glm::vec4 cubeGnomonProj[4];
	glm::vec4 worldGnomonProj[4];

	std::array<std::pair<glm::vec4, glm::vec4>, 12> cubeClipped;
	std::array<std::pair<glm::vec4, glm::vec4>, 3> cubeGnomonClipped;
	std::array<std::pair<glm::vec4, glm::vec4>, 3> worldGnomonClipped;

	std::array<std::pair<glm::vec2, glm::vec2>, 12> cubeFinal;
	std::array<std::pair<glm::vec2, glm::vec2>, 3> cubeGnomonFinal;
	std::array<std::pair<glm::vec2, glm::vec2>, 3> worldGnomonFinal;

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	Mode current_mode;
};
