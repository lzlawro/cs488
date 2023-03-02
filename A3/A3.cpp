// Termm-Fall 2020

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <queue>

#include <math.h>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
static JointNode* findParentJointFromId(unsigned int id, SceneNode *node) {
	std::queue<SceneNode *> nodeQueue;
	// BFS with queue
	nodeQueue.push(node);

	while (!nodeQueue.empty()) {
		SceneNode *currentNode = nodeQueue.front();
		nodeQueue.pop();

		for (SceneNode *child: currentNode->children) {
			if (child->m_nodeId == id && currentNode->m_nodeType == NodeType::JointNode) {
				return (JointNode *)currentNode;
			} else {
				nodeQueue.push(child);
			}
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------------------------
static SceneNode* findHeadNode(SceneNode *node) {
	std::queue<SceneNode *> nodeQueue;
	// BFS with queue
	nodeQueue.push(node);

	while (!nodeQueue.empty()) {
		SceneNode *currentNode = nodeQueue.front();
		nodeQueue.pop();

		if (currentNode->m_name == "head") {
			// cout << "head found" << endl;
			return currentNode;
		}

		for (SceneNode *child: currentNode->children) {
				nodeQueue.push(child);
		}
	}

	// cout << "head not found" << endl;
	return nullptr;
}

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  current_mode(POSITION_ORIENTATION),
	  m_model_translation(mat4(1.0f)),
	  m_model_rotation(mat4(1.0f)),
	  m_model_z_rotation(mat4(1.0f)),
	  m_headJoint(nullptr),
	  m_headNode(nullptr)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.4, 0.4, 0.4, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("pyramid.obj"),
			getAssetFilePath("cylinder.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	// cout << m_rootNode->totalSceneNodes() << endl;
	for (int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
		selected.push_back(false);
	}

	do_circle=(false);
	do_z_buffer=(true);
	do_backface_culling=(false);
	do_frontface_culling=(false);

	do_picking = false;

	m_headNode = findHeadNode(m_rootNode.get());

	if (m_headNode != nullptr) 
		m_headJoint = findParentJointFromId(m_headNode->m_nodeId, m_rootNode.get());

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
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
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
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


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
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

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
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

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}

//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = vec3(0.75f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i(location, do_picking ? 1 : 0);
		CHECK_GL_ERRORS;
		if (!do_picking){
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
				vec3 ambientIntensity(0.25f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Application"))
			{
				if (ImGui::MenuItem("Reset Position       (I)")) resetPosition();
				if (ImGui::MenuItem("Reset Orientation    (O)")) resetOrientation();
				if (ImGui::MenuItem("Reset Joints         (S)")) resetJoints();
				if (ImGui::MenuItem("Reset All            (A)")) resetAll();
				if( ImGui::MenuItem("Quit                 (Q)") ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo        (U)")) undoJoints();
				if (ImGui::MenuItem("Redo        (R)")) redoJoints();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::Checkbox("Circle               (C)", &do_circle);
				ImGui::Checkbox("Z-buffer             (Z)", &do_z_buffer);
				ImGui::Checkbox("Backface Culling     (B)", &do_backface_culling);
				ImGui::Checkbox("Frontface Culling    (F)", &do_frontface_culling);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::RadioButton("Position/Orientation    (P)", (int*)&current_mode, POSITION_ORIENTATION);
		ImGui::RadioButton("Joints                  (J)", (int*)&current_mode, JOINTS);

		// Create Button, and check if it was clicked:
		if( ImGui::Button("Quit Application    (Q)") ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix,
		const bool & picking,
		const vector<bool>& selected
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
		if (picking) {
			float r = float(node.m_nodeId & 0xff) / 255.0f;
			float g = float((node.m_nodeId >> 8) & 0xff) / 255.0f;
			float b = float((node.m_nodeId >> 16) & 0xff) / 255.0f;

			location = shader.getUniformLocation("material.kd");
			glUniform3f(location, r, g, b);
			CHECK_GL_ERRORS;
		} else {
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;

			vec3 kd = node.material.kd;
			vec3 ks = node.material.ks;
			float shininess = node.material.shininess;

			if (selected[node.m_nodeId]) {
				kd = vec3(242.0f/255.0f, 210.0f/255.0f, 189.0f/255.0f);
				ks = vec3(1.0f, 1.0f, 1.0f);
				shininess = 1.0f;
			}

			// TODO: If node is selected, use fixed material values

			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("material.ks");
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, shininess);
			CHECK_GL_ERRORS;
		}
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if (do_z_buffer) glEnable( GL_DEPTH_TEST );

	if (do_backface_culling && do_frontface_culling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	} else if (do_backface_culling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else if (do_frontface_culling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	renderSceneGraph(*m_rootNode);

	if (do_circle) renderArcCircle();

	if (do_z_buffer) glDisable( GL_DEPTH_TEST );

	if (do_backface_culling || do_frontface_culling) {
		glDisable(GL_CULL_FACE);
	}
}

//----------------------------------------------------------------------------------------
void A3::renderSceneNode(
	const SceneNode *node, 
	glm::mat4 view, 
	glm::mat4 model,
	stack<glm::mat4> &st
	) {
	if (node == nullptr) return;

	mat4 M_push = node->get_transform();
	st.push(M_push);
	model = model * M_push;

	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(
			m_shader, 
			*geometryNode, 
			view, 
			model, 
			do_picking,
			selected
			);

		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	for (const SceneNode *child: node->children) {
		renderSceneNode(child, view, model, st);
	}

	mat4 M_pop = st.top();
	st.pop();
	model = model * glm::inverse(M_pop);
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

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

	mat4 rootModel = root.get_transform();

	stack<mat4> matStack;

	renderSceneNode(
		&root, 
		m_view, 
		m_model_z_rotation * m_model_translation * rootModel * m_model_rotation * glm::inverse(rootModel), 
		matStack
		);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::updatePositionOrientation(double xPos, double yPos) {
	float deltaX = (xPos - m_prev_xPos) / 200.0f;
	float deltaY = (yPos - m_prev_yPos) / 200.0f;

	vec3 translationVector = vec3(0.0f, 0.0f, 0.0f);

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
void A3::updateJoints(double xPos, double yPos) {
	float deltaX = (xPos - m_prev_xPos) / 200.0f;
	float deltaY = (yPos - m_prev_yPos) / 200.0f;

	// Rotate the selected nodes' parent joints
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		for (int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
			if (selected[i]) {
				JointNode *parentJoint = findParentJointFromId(i, m_rootNode.get());
				if (parentJoint == nullptr) continue;

				// cout << parentJoint->m_name << endl;

				parentJoint->rotate('y', radiansToDegrees( deltaX ));
				parentJoint->rotate('x', radiansToDegrees( deltaY ));
			}
		}
	}
	// Rotate head if head is selected
	if (ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)) {
		if (m_headNode != nullptr && selected[m_headNode->m_nodeId] &&
			m_headJoint != nullptr) {
				// cout << "rotating head" << endl;
				m_headJoint->rotate('y', radiansToDegrees( deltaX ));
				m_headJoint->rotate('x', radiansToDegrees( deltaY ));
			}
	}
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
				 vec3 *fNewVec, vec3 *fOldVec,
                 float *fVecX, float *fVecY, float *fVecZ) {
   long  nXOrigin, nYOrigin;
   float fNewVecX, fNewVecY, fNewVecZ,
         fOldVecX, fOldVecY, fOldVecZ,
         fLength;

   fNewVecX    = fNewX * 2.0 / fDiameter;
   fNewVecY    = fNewY * 2.0 / fDiameter;
   fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);

   *fNewVec = vec3(fNewVecX, fNewVecY, fNewVecZ);

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

   *fOldVec = vec3(fOldVecX, fOldVecY, fOldVecZ);

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
		mNewMat = mat4(1.0f);
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
void A3::performTrackballTransformation(double xPos, double yPos) {
	float fOldX = m_prev_xPos, fNewX = xPos, fOldY = m_prev_yPos, fNewY = yPos;
	float fDiameter;
	int iCenterX, iCenterY;
	float fNewModX, fNewModY, fOldModX, fOldModY;

	float  fRotVecX, fRotVecY, fRotVecZ;
    glm::mat4 mNewMat = mat4(1.0f);

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

	vec3 fNewVec, fOldVec;

	vCalcRotVec(fNewModX, fNewModY,
                        fOldModX, fOldModY,
                        fDiameter,
						&fNewOutside, &fOldOutside,
						&fNewVec, &fOldVec,
                        &fRotVecX, &fRotVecY, &fRotVecZ);

	vAxisRotMatrix(fRotVecX, -fRotVecY, fRotVecZ, mNewMat);

	mNewMat = glm::transpose(mNewMat);

	m_model_rotation = mNewMat * m_model_rotation;

	// if (!(fNewOutside && fOldOutside)) m_model_rotation = mNewMat * m_model_rotation;
	// else {

	// }
}

//----------------------------------------------------------------------------------------
void A3::undoJoints() {

}

//----------------------------------------------------------------------------------------
void A3::redoJoints() {

}

void A3::resetPosition() {
	m_model_translation = mat4(1.0f);
}
void A3::resetOrientation() {
	m_model_rotation = mat4(1.0f);
}
void A3::resetJoints() {}
void A3::resetAll() { resetPosition(); resetOrientation(); resetJoints(); }

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Fill in with event handling code...
		switch(current_mode) {
			case POSITION_ORIENTATION:
				updatePositionOrientation(xPos, yPos);
				eventHandled = true;
				break;
			case JOINTS:
				updateJoints(xPos, yPos);
				eventHandled = true;
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
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow() &&
		actions == GLFW_PRESS &&
		current_mode == JOINTS &&
		button == GLFW_MOUSE_BUTTON_LEFT) {
			// TODO: handle picking
			// cout << "Time to handle picking" << endl;
			double xPos, yPos;
			glfwGetCursorPos(m_window, &xPos, &yPos);

			do_picking = true;

			uploadCommonSceneUniforms();
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.4, 0.4, 0.4, 1.0);

			draw();

			CHECK_GL_ERRORS;

			// glFlush();
			// glFinish();

			xPos *= double(m_framebufferWidth) / double(m_windowWidth);
			yPos = m_windowHeight - yPos;
			yPos *= double(m_framebufferHeight) / double(m_windowHeight);

			GLubyte buffer[4] = {0, 0, 0, 0};

			glReadBuffer(GL_BACK);
			// Actually read the pixel at the mouse location.
			glReadPixels(int(xPos), int(yPos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			CHECK_GL_ERRORS;

			unsigned int nodeId = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

			if (nodeId < m_rootNode->totalSceneNodes()) {
				selected[nodeId] = !selected[nodeId];
			}

			do_picking = false;

			CHECK_GL_ERRORS;

			eventHandled = true;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, true);
			eventHandled = true;
		}
		// Modes
		if (key == GLFW_KEY_P) {
			current_mode = POSITION_ORIENTATION;
			eventHandled = true;
		}
		if (key == GLFW_KEY_J) {
			current_mode = JOINTS;
			eventHandled = true;
		}
		// Options
		if (key == GLFW_KEY_C) {
			do_circle = !do_circle;
			eventHandled = true;
		}
		if (key == GLFW_KEY_Z) {
			do_z_buffer = !do_z_buffer;
			eventHandled = true;
		}
		if (key == GLFW_KEY_B) {
			do_backface_culling = !do_backface_culling;
			eventHandled = true;
		}
		if (key == GLFW_KEY_F) {
			do_frontface_culling = !do_frontface_culling;
			eventHandled = true;
		}
		// Edit
		if (key == GLFW_KEY_U) {
			undoJoints();
			eventHandled = true;
		}
		if (key == GLFW_KEY_R) {
			redoJoints();
			eventHandled = true;
		}
		// Application
		if (key == GLFW_KEY_I) {
			resetPosition();
		}
		if (key == GLFW_KEY_O) {
			resetOrientation();
		}
		if (key == GLFW_KEY_S) {
			resetJoints();
		}
		if (key == GLFW_KEY_A) {
			resetAll();
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
