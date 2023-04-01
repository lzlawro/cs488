// Term-Winter 2023

#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"

#include <stack>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

class A5 : public CS488Window {
public:
    A5(const std::string &luaSceneFile);
    virtual ~A5();

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

	void renderSceneNode(
		const SceneNode *node, 
		glm::mat4 view, 
		glm::mat4 model,
		std::stack<glm::mat4> &st
	);

	void renderSceneGraph(const SceneNode &node);

    glm::mat4 m_perspective;
    glm::mat4 m_view;

    LightSource m_light;

    //-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	glm::mat4 m_model_translation;
	glm::mat4 m_model_rotation;

    // BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
    BatchInfoMap m_batchInfoMap;

    ShaderProgram m_sphere_shader;
    ShaderProgram m_water_shader;
    ShaderProgram m_pool_shader;

    std::string m_luaSceneFile;

    std::shared_ptr<SceneNode> m_rootNode;
};