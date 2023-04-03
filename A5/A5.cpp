// Term-Winter 2023

#include "A5.hpp"
#include "scene_lua.hpp"

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"

#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Trackball_Example/events.h"
#include "../Trackball_Example/trackball.h"

#include <lodepng/lodepng.h>

#include <queue>
#include <cstdlib>
#include <unistd.h>
#include <ctime>

static bool show_gui = true;

const static float MinX = -2.0f;
const static float MaxX = 2.0f;
const static float MinY = -2.75f;
const static float MaxY = 5.0f;
const static float MinZ = -2.0f;
const static float MaxZ = 2.0f;
const static float SurfaceY = -0.25f;
const static float GravityY = -9.81f;

//----------------------------------------------------------------------------------------
// Constructor
A5::A5(const std::string &luaSceneFile)
    : current_mode(POSITION_ORIENTATION),
	  m_prev_xPos(0.0),
	  m_prev_yPos(0.0),
	  m_luaSceneFile(luaSceneFile),
      m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_model_translation(glm::mat4(1.0f)),
	  m_model_rotation(glm::mat4(1.0f)),
	  m_sphere_center(glm::vec3(0.2, 0.75, 0.2)),
	  m_sphere_radius(0.5f),
	  m_sphere_velocity(0.0f),
	  m_sphereNode(nullptr),
	  m_initialSphereTrans(glm::mat4(1.0f)),
	  do_physics(false)
{
    
}

//----------------------------------------------------------------------------------------
// Destructor
A5::~A5()
{

}

//----------------------------------------------------------------------------------------
void A5::reset()
{
	m_prev_xPos = (0.0);
	m_prev_yPos = (0.0);
	m_model_translation = (glm::mat4(1.0f));
	m_model_rotation = (glm::mat4(1.0f));
	// m_sphereNode->translate(glm::vec3(0, 0, -7.5));
	// m_sphereNode->translate(glm::vec3(0.2, 0.75, 0.2));
	m_sphereNode->set_transform(m_initialSphereTrans);
	// std::cout << initialSphereTrans << std::endl;
	m_sphere_center = (glm::vec3(0.2, 0.75, 0.2));
	m_sphere_radius = (0.5f);
	
	do_physics = false;
}

//----------------------------------------------------------------------------------------
void A5::processLuaSceneFile(const std::string &filename) {
    // This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.

    m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
static SceneNode* findSphereNode(SceneNode *node, unsigned int node_count) {
	bool visited[node_count];
	for (int i = 0; i < node_count; i++) visited[i] = false;

	std::queue<SceneNode *> nodeQueue;
	// BFS with queue
	visited[node->m_nodeId] = true;
	nodeQueue.push(node);

	while (!nodeQueue.empty()) {
		SceneNode *currentNode = nodeQueue.front();
		nodeQueue.pop();

		if (currentNode->m_name == "sphere") {
			// cout << "head found" << endl;
			// initialSphereTrans = currentNode->get_transform();
			return currentNode;
		}

		for (SceneNode *child: currentNode->children) {
			if (visited[child->m_nodeId]) continue;

			visited[child->m_nodeId] = true;

			nodeQueue.push(child);
		}
	}

	// cout << "head not found" << endl;
	return nullptr;
}

//----------------------------------------------------------------------------------------
void A5::createShaderProgram()
{
    m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A5::createSphereShader()
{
	m_sphere_shader.generateProgramObject();
	m_sphere_shader.attachVertexShader(getAssetFilePath("SphereVertexShader.vs").c_str());
	m_sphere_shader.attachFragmentShader(getAssetFilePath("SphereFragmentShader.fs").c_str());
	m_sphere_shader.link();
}

//----------------------------------------------------------------------------------------
void A5::createWaterShader()
{
	m_water_shader.generateProgramObject();
	m_water_shader.attachVertexShader(getAssetFilePath("WaterVertexShader.vs").c_str());
	m_water_shader.attachFragmentShader(getAssetFilePath("WaterFragmentShader.fs").c_str());
	m_water_shader.link();
}

//----------------------------------------------------------------------------------------
void A5::createPoolShader()
{
	m_pool_shader.generateProgramObject();
	// m_pool_shader.attachVertexShader(getAssetFilePath("PoolVertexShader copy.vs").c_str());
	// m_pool_shader.attachFragmentShader(getAssetFilePath("PoolFragmentShader copy.fs").c_str());
	m_pool_shader.attachVertexShader(getAssetFilePath("PoolVertexShader.vs").c_str());
	m_pool_shader.attachFragmentShader(getAssetFilePath("PoolFragmentShader.fs").c_str());
	m_pool_shader.link();
}

//----------------------------------------------------------------------------------------
void A5::enableVertexShaderInputSlots()
{
    //-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A5::uploadVertexDataToVbos(
    const MeshConsolidator &meshConsolidator
) {
    // Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
static void playWaterSound() {
	pid_t pid = fork();
	if (pid == 0) {
		srand(time(0));
		int num = rand() % 4;
		switch (num) {
			case 0:
				std::system("canberra-gtk-play -f Assets/Splash-2CloseDistance.ogg");
				break;
			case 1:
				std::system("canberra-gtk-play -f Assets/Splash-4CloseDistance.ogg");
				break;
			case 2:
				std::system("canberra-gtk-play -f Assets/Splash-6CloseDistance.ogg");
				break;
			case 3:
				std::system("canberra-gtk-play -f Assets/Splash-7CloseDistance.ogg");
				break;
		}
		exit(0);
	}
}

//----------------------------------------------------------------------------------------
void A5::mapVboDataToVertexShaderInputLocations()
{
    // Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A5::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perspective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A5::initViewMatrix() {
	m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A5::initLightSources() {
	// World-space position
	m_light.position = glm::vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = glm::vec3(0.75f); // light
}

//----------------------------------------------------------------------------------------
void A5::initPoolTexture() {
	glGenTextures(1, &m_pool_texture);
	glBindTexture(GL_TEXTURE_2D, m_pool_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	std::vector<unsigned char> image;
	unsigned int width, height;
	unsigned int error = lodepng::decode(image, width, height, getAssetFilePath("tiles.png").c_str());

	if (error != 0) {
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		std::cout << "error with decoding png" << std::endl;
		return;
	}

	// Here the PNG is loaded in "image". All the rest of the code is SDL and OpenGL stuff

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	// std::cout << width << ", " << height << std::endl;
	// std::cout << image.size() << std::endl;

	m_pool_shader.enable();
	{
		GLint location = m_pool_shader.getUniformLocation("tiles");
		glUniform1i(location, m_pool_texture);
		// std::cout << m_pool_texture << std::endl;
		CHECK_GL_ERRORS;
	}
	m_pool_shader.disable();
}

//----------------------------------------------------------------------------------------
void A5::uploadCommonSceneUniforms() {
	GLint location;

	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			glm::vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();

	m_sphere_shader.enable();
	{
		location = m_sphere_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
		CHECK_GL_ERRORS;	
	}
	m_sphere_shader.disable();

	m_pool_shader.enable();
	{
		location = m_pool_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
		CHECK_GL_ERRORS;

		// location = m_pool_shader.getUniformLocation("tiles");
		// glUniform1i(location, m_pool_texture);
		// std::cout << m_pool_texture << std::endl;
		// CHECK_GL_ERRORS;
	}
	m_pool_shader.disable();

	m_water_shader.enable();
	{
		location = m_water_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
		CHECK_GL_ERRORS;	
	}
	m_water_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A5::init() 
{

    // Set the background colour.
    glClearColor(0.4, 0.4, 0.4, 1.0);

    createShaderProgram();

	createSphereShader();
	createPoolShader();
	createWaterShader();

    glGenVertexArrays(1, &m_vao_meshData);

    enableVertexShaderInputSlots();

    processLuaSceneFile(m_luaSceneFile);

    // Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.

    std::unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator {
        getAssetFilePath("cube.obj"),
        getAssetFilePath("sphere.obj"),
		getAssetFilePath("plane.obj")
    });

    meshConsolidator->getBatchInfoMap(m_batchInfoMap);

    // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU
    uploadVertexDataToVbos(*meshConsolidator);

    mapVboDataToVertexShaderInputLocations();

    initPerspectiveMatrix();

    initViewMatrix();
    
    initLightSources();

	initPoolTexture();

	m_sphereNode = findSphereNode(m_rootNode.get(), m_rootNode->totalSceneNodes());

	m_initialSphereTrans = m_sphereNode->get_transform();

	// std::cout << m_sphereNode->m_name << std::endl;

    // Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A5::appLogic()
{
    // Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A5::guiLogic() 
{
    if (!show_gui) {
        return;
    }

    static bool firstRun(true);

    if (firstRun) {
        ImGui::SetNextWindowPos(ImVec2(50, 50));
        firstRun = false;
    }

    static bool showDebugWindow(true);
    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.5f);

    ImGui::Begin("Properties", &showDebugWindow, ImVec2(100, 100), opacity, windowFlags);

    // Add more gui elements here here ...

	ImGui::Checkbox("Physics                 (G)", &do_physics);

	ImGui::RadioButton("Position/Orientation    (P)", (int*)&current_mode, POSITION_ORIENTATION);

	ImGui::RadioButton("Move Sphere             (O)", (int*)&current_mode, MOVE_SPHERE);

	ImGui::RadioButton("Produce Ripples         (I)", (int*)&current_mode, PRODUCE_RIPPLES);

	// if( ImGui::Button( "Play Sound              (H)" ) ) {
    //     playWaterSound();
    // }

    if( ImGui::Button( "Reset                   (A)" ) ) {
        reset();
    }

    // Create Button, and check if it was clicked:
    if( ImGui::Button( "Quit Application        (Q)" ) ) {
        glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);

    ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A5::updateShaderUniforms(
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix
		) {
	m_shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = m_shader.getUniformLocation("ModelView");
		glm::mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = m_shader.getUniformLocation("NormalMatrix");
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		glm::vec3 kd = node.material.kd;
		glm::vec3 ks = node.material.ks;
		float shininess = node.material.shininess;

		location = m_shader.getUniformLocation("material.kd");
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
void A5::updateSphereShaderUniforms(
	const glm::mat4 & viewMatrix,
	const glm::mat4 & modelMatrix
) {
	m_sphere_shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = m_sphere_shader.getUniformLocation("ModelView");
		glm::mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
	}
	m_sphere_shader.disable();
}

//----------------------------------------------------------------------------------------
void A5::updatePoolShaderUniforms(
	const glm::mat4 & viewMatrix,
	const glm::mat4 & modelMatrix
) {
	m_pool_shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = m_pool_shader.getUniformLocation("ModelView");
		glm::mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
	}
	m_pool_shader.disable();
}

//----------------------------------------------------------------------------------------
void A5::updateWaterShaderUniforms(
	const glm::mat4 & viewMatrix,
	const glm::mat4 & modelMatrix
) {
	m_water_shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = m_water_shader.getUniformLocation("ModelView");
		glm::mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		// location = m_sphere_shader.getUniformLocation("sphereCenter");
		// glUniform3fv(location, 1, value_ptr(m_sphere_center));
		// CHECK_GL_ERRORS;

		// location = m_sphere_shader.getUniformLocation("sphereRadius");
		// glUniform1f(location, m_ball_radius);
		// CHECK_GL_ERRORS;
	}
	m_water_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A5::draw() 
{
    glEnable( GL_DEPTH_TEST );
	renderSceneGraph(*m_rootNode);
    glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
void A5::renderSceneNode(
		const SceneNode *node, 
		glm::mat4 view, 
		glm::mat4 model,
		std::stack<glm::mat4> &st
	) {
	
	if (node == nullptr) return;

	glm::mat4 M_push = node->get_transform();
	st.push(M_push);
	model = model * M_push;

	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		if (node->m_name == "sphere") {
			updateSphereShaderUniforms(
				view,
				model
			);

			m_sphere_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_sphere_shader.disable();
		} else if (node->m_name == "water") {
			updateWaterShaderUniforms(
				view,
				model
			);

			m_water_shader.enable();
			// std::cout << GL_TEXTURE0 + m_pool_texture << std::endl;
			glActiveTexture(GL_TEXTURE0 + m_pool_texture);
			glBindTexture(GL_TEXTURE_2D, m_pool_texture);
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_water_shader.disable();
		} else if (node->m_name == "pool") {
			updatePoolShaderUniforms(
				view,
				model
			);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			m_pool_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_pool_shader.disable();
			glDisable(GL_CULL_FACE);
		}
	}

	for (const SceneNode *child: node->children) {
		renderSceneNode(child, view, model, st);
	}

	glm::mat4 M_pop = st.top();
	st.pop();
	model = model * glm::inverse(M_pop);
}

//----------------------------------------------------------------------------------------
void A5::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	// for (const SceneNode * node : root.children) {

	// 	if (node->m_nodeType != NodeType::GeometryNode)
	// 		continue;

	// 	const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

	// 	updateShaderUniforms(m_shader, *geometryNode, m_view);


	// 	// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	// 	BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

	// 	//-- Now render the mesh:
	// 	m_shader.enable();
	// 	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	// 	m_shader.disable();
	// }

	// glBindVertexArray(0);
	// CHECK_GL_ERRORS;

	glm::mat4 rootModel = root.get_transform();

	std::stack<glm::mat4> matStack;

	renderSceneNode(
		&root,
		m_view,
		m_model_translation * rootModel * m_model_rotation * glm::inverse(rootModel),
		matStack
	);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A5::updatePositionOrientation(double xPos, double yPos) {
	float deltaX = (xPos - m_prev_xPos) / 200.0f;
	float deltaY = (yPos - m_prev_yPos) / 200.0f;

	glm::vec3 translationVector = glm::vec3(0.0f, 0.0f, 0.0f);

	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
		translationVector.x = deltaX;
		translationVector.y = -deltaY;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		translationVector.z = deltaY;
	}
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		// trackball...
		performTrackballTransformation(xPos, yPos);
	}

	m_model_translation = glm::translate(m_model_translation, translationVector);
}

//----------------------------------------------------------------------------------------
void vTransposeMatrix(Matrix mSrcDst) {
    GLdouble temp;
    int i,j;

    // Transpose matrix
    for ( i=0; i<4; ++i ) {
        for ( j=i+1; j<4; ++j ) {
            temp = mSrcDst[i][j];
            mSrcDst[i][j] = mSrcDst[j][i];
            mSrcDst[j][i] = temp;
        }
    }
}

//----------------------------------------------------------------------------------------
void vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
				 bool *fNewOutside, bool *fOldOutside,
				 glm::vec3 *fNewVec, glm::vec3 *fOldVec,
                 float *fVecX, float *fVecY, float *fVecZ) {
   long  nXOrigin, nYOrigin;
   float fNewVecX, fNewVecY, fNewVecZ,
         fOldVecX, fOldVecY, fOldVecZ,
         fLength;

   fNewVecX    = fNewX * 2.0 / fDiameter;
   fNewVecY    = fNewY * 2.0 / fDiameter;
   fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);

   *fNewVec = glm::vec3(fNewVecX, fNewVecY, fNewVecZ);

    /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fNewVecZ < 0.0) {
      fLength = sqrt(1.0 - fNewVecZ);
      fNewVecZ  = 0.0;
      fNewVecX /= fLength;
      fNewVecY /= fLength;
	  *fNewOutside = true;
   } else {
      fNewVecZ = sqrt(fNewVecZ);
	  *fNewOutside = false;
   }

   fOldVecX    = fOldX * 2.0 / fDiameter;
   fOldVecY    = fOldY * 2.0 / fDiameter;
   fOldVecZ    = (1.0 - fOldVecX * fOldVecX - fOldVecY * fOldVecY);

   *fOldVec = glm::vec3(fOldVecX, fOldVecY, fOldVecZ);

   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fOldVecZ < 0.0) {
      fLength = sqrt(1.0 - fOldVecZ);
      fOldVecZ  = 0.0;
      fOldVecX /= fLength;
      fOldVecY /= fLength;
	  *fOldOutside = true;
   } else {
      fOldVecZ = sqrt(fOldVecZ);
	  *fOldOutside = false;
   }

   *fVecX = fOldVecY * fNewVecZ - fNewVecY * fOldVecZ;
   *fVecY = fOldVecZ * fNewVecX - fNewVecZ * fOldVecX;
   *fVecZ = fOldVecX * fNewVecY - fNewVecX * fOldVecY;
}

//----------------------------------------------------------------------------------------
void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, glm::mat4 &mNewMat) {
    float fRadians, fInvLength, fNewVecX, fNewVecY, fNewVecZ;

    /* Find the length of the vector which is the angle of rotation
     * (in radians)
     */
    fRadians = sqrt(fVecX * fVecX + fVecY * fVecY + fVecZ * fVecZ);

    /* If the vector has zero length - return the identity matrix */
    if (fRadians > -0.000001 && fRadians < 0.000001) {
		mNewMat = glm::mat4(1.0f);
        return;
    }

    /* Normalize the rotation vector now in preparation for making
     * rotation matrix. 
     */
    fInvLength = 1 / fRadians;
    fNewVecX   = fVecX * fInvLength;
    fNewVecY   = fVecY * fInvLength;
    fNewVecZ   = fVecZ * fInvLength;

    /* Create the arbitrary axis rotation matrix */
    double dSinAlpha = sin(fRadians);
    double dCosAlpha = cos(fRadians);
    double dT = 1 - dCosAlpha;

    mNewMat[0][0] = dCosAlpha + fNewVecX*fNewVecX*dT;
    mNewMat[0][1] = fNewVecX*fNewVecY*dT + fNewVecZ*dSinAlpha;
    mNewMat[0][2] = fNewVecX*fNewVecZ*dT - fNewVecY*dSinAlpha;
    mNewMat[0][3] = 0;

    mNewMat[1][0] = fNewVecX*fNewVecY*dT - dSinAlpha*fNewVecZ;
    mNewMat[1][1] = dCosAlpha + fNewVecY*fNewVecY*dT;
    mNewMat[1][2] = fNewVecY*fNewVecZ*dT + dSinAlpha*fNewVecX;
    mNewMat[1][3] = 0;

    mNewMat[2][0] = fNewVecZ*fNewVecX*dT + dSinAlpha*fNewVecY;
    mNewMat[2][1] = fNewVecZ*fNewVecY*dT - dSinAlpha*fNewVecX;
    mNewMat[2][2] = dCosAlpha + fNewVecZ*fNewVecZ*dT;
    mNewMat[2][3] = 0;

    mNewMat[3][0] = 0;
    mNewMat[3][1] = 0;
    mNewMat[3][2] = 0;
    mNewMat[3][3] = 1;
}

//----------------------------------------------------------------------------------------
void A5::performTrackballTransformation(double xPos, double yPos) {
	float fOldX = m_prev_xPos, fNewX = xPos, fOldY = m_prev_yPos, fNewY = yPos;
	float fDiameter;
	int iCenterX, iCenterY;
	float fNewModX, fNewModY, fOldModX, fOldModY;

	float  fRotVecX, fRotVecY, fRotVecZ;
    glm::mat4 mNewMat = glm::mat4(1.0f);

	int nWinWidth = m_windowWidth;
	int nWinHeight = m_windowHeight;

	fDiameter = (nWinWidth < nWinHeight) ? nWinWidth * 0.5 : nWinHeight * 0.5;
	iCenterX = nWinWidth / 2;
	iCenterY = nWinHeight / 2;
	fOldModX = fOldX - iCenterX;
	fOldModY = fOldY - iCenterY;
	fNewModX = fNewX - iCenterX;
	fNewModY = fNewY - iCenterY;

	bool fNewOutside, fOldOutside;

	glm::vec3 fNewVec, fOldVec;

	vCalcRotVec(fNewModX, fNewModY,
                        fOldModX, fOldModY,
                        fDiameter,
						&fNewOutside, &fOldOutside,
						&fNewVec, &fOldVec,
                        &fRotVecX, &fRotVecY, &fRotVecZ);

	vAxisRotMatrix(fRotVecX, -fRotVecY, fRotVecZ, mNewMat);

	mNewMat = glm::transpose(mNewMat);

	m_model_rotation = mNewMat * m_model_rotation;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A5::cleanup() 
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A5::cursorEnterWindowEvent(int entered) 
{
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
void A5::moveSphere(glm::vec3 translationVector) {
	if (m_sphereNode == nullptr) return;

	// if (m_sphere_center.x + translationVector.x)

	if (m_sphere_center.x - m_sphere_radius + translationVector.x < MinX ||
		m_sphere_center.x + m_sphere_radius + translationVector.x > MaxX) {
			translationVector.x = 0.0f;
	}

	if (m_sphere_center.y - m_sphere_radius + translationVector.y < MinY ||
		m_sphere_center.y + m_sphere_radius + translationVector.y > MaxY) {
			translationVector.y = 0.0f;
	}

	if (m_sphere_center.z - m_sphere_radius + translationVector.z < MinZ ||
		m_sphere_center.z + m_sphere_radius + translationVector.z > MaxZ) {
			translationVector.z = 0.0f;
	}


	m_sphere_center += translationVector;

	m_sphereNode->translate(translationVector);
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A5::mouseMoveEvent(double xPos, double yPos) 
{
    bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Fill in with event handling code...
		switch(current_mode) {
			case POSITION_ORIENTATION:
				{
					updatePositionOrientation(xPos, yPos);
					eventHandled = true;
				}
				break;
			case MOVE_SPHERE: 
				{
					float deltaX = (xPos - m_prev_xPos) / 200.0f;
					float deltaY = (yPos - m_prev_yPos) / 200.0f;

					glm::vec3 translationVector = glm::vec3(0.0f, 0.0f, 0.0f);
		
					if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)) {
						translationVector.x = deltaX;
					}
					if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
						translationVector.y = deltaX;
					}
					if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
						translationVector.z = deltaX;
					}
					moveSphere(translationVector);

					eventHandled = true;
				}
				break;
			case PRODUCE_RIPPLES:
				{

				}
				break;
		}
	}

	m_prev_xPos = xPos;
	m_prev_yPos = yPos;

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A5::mouseButtonInputEvent(int button, int actions, int mods) 
{
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A5::mouseScrollEvent(double xOffSet, double yOffSet) 
{
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A5::windowResizeEvent(int width, int height) 
{
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A5::keyInputEvent(int key, int action, int mods) 
{
    bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if( key == GLFW_KEY_P ) {
			current_mode = POSITION_ORIENTATION;
			eventHandled = true;
		}
		if( key == GLFW_KEY_O ) {
			current_mode = MOVE_SPHERE;
			eventHandled = true;
		}
		if( key == GLFW_KEY_I ) {
			current_mode = PRODUCE_RIPPLES;
			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, true);
			eventHandled = true;
		}
		if (key == GLFW_KEY_A) {
			reset();
			eventHandled = true;
		}
		if (key == GLFW_KEY_G) {
			do_physics = !do_physics;
			eventHandled = true;
		}
		if (key == GLFW_KEY_H) {
			playWaterSound();
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}