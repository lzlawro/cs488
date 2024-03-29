// Termm-Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include "../Trackball_Example/events.h"
#include "../Trackball_Example/trackball.h"

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <set>
#include <unordered_map>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();

	void uploadCommonSceneUniforms();

	void updateJointAngleData();

	void updateJointAngleFromJointAngleData();
	
	void renderSceneNode(
		const SceneNode *node, 
		glm::mat4 view, 
		glm::mat4 model,
		std::stack<glm::mat4> &st
	);
	
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();

	void updatePositionOrientation(double xPos, double yPos);
	void updateJoints(double xPos, double yPos);

	void performTrackballTransformation(double xPos, double yPos);

	void undoJoints();
	void redoJoints();

	void resetPosition();
	void resetOrientation();
	void resetJoints();
	void resetAll();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	glm::mat4 m_model_translation;
	glm::mat4 m_model_rotation;

	bool do_circle;
	bool do_z_buffer;
	bool do_backface_culling;
	bool do_frontface_culling;

	bool do_picking;

	int jointAngleDataIndex;
	
	// std::vector<std::unordered_map<JointNode *, std::pair<float, float>>> jointAngleData;

	std::vector<std::unordered_map<JointNode *, glm::mat4>> jointAngleData;

	std::vector<bool> selected;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	enum Mode {POSITION_ORIENTATION, JOINTS};

	Mode current_mode;

	std::shared_ptr<SceneNode> m_rootNode;

	JointNode* m_headJoint;
	SceneNode* m_headNode;

	double m_prev_xPos;
	double m_prev_yPos;
};
