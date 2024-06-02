#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <iostream>
#include <sstream>
#include <future>
#include <vector>
#include <map>
#include <stdlib.h>
#include "shader.h"
#include "window.h"
#include "ModelReader.h"
#include "shapes.h"
#include "OrbitAnimator.h"
#include "SceneState.h"
#include "PlanetMath.h"
#include "Camera.h"
#include "Gui.h"


using namespace std;

// ======================= prototype =======================

// load function
unsigned int loadCubemap(vector<string> filename);
unsigned int loadTexture(const char* filename);

// opengl helper
void glSetupVertexObject(unsigned int& VAO, unsigned int& VBO, vector<float>& data, vector<int> attribLayout);
void glDrawVertexTriangles(unsigned int VAO, GLuint texture, int numberOfVertex);
void glSetModelViewProjection(unsigned int shaderProgram, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, Camera camPos, int torch, Gui& gui);

// helper
glm::vec3 vecToVec3(vector<float> vec);
vector<float> vec3ToVec(glm::vec3 vec3);

// opengl code dump
void displayLoadingScreen(GLFWwindow* window);
void displaySkyBox(unsigned int& VAO, GLuint texture, unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection);


// ====================== global variable =======================

// settings
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 800;
const char* WINDOW_TITLE = "3D Solar System";


// scene 
int sunIdx = 0;		// THIS MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
int earthIdx = 3;   // THIS MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
float earthOrbitDelay = 3600;
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
PlanetMath planetMath;
vector<RenderedBody> renderedBodies;
vector<BodyConst> bodyConstants;
vector<OrbitAnimator> animators;


SceneState sceneState;
Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT);

// ==================== main =======================

int main(int argc, char** argv)
{

	// ======================= SETUP ======================	

	srand((int)time(NULL));
	GLFWwindow* window = createWindow(
		WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);	// create window
	setWindowCenter(window);							// adjust window position
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disable mouse
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // init glad


	// ====================== OPENGL ======================		

	displayLoadingScreen(window);	// loading screen (contains gl code)
	int startLoadingTime = (int)glfwGetTime(); // to calculate loading time


	// ========= load objects =========

	ObjFileReader ofr;
	ObjectFileData sphereObj, ufoObj, rocket2Obj, saturnRingObj, uranusRingObj, astroid1Obj,
		commandModuleObj, electronRocketObj, satelite1Obj, superHeavyRocketObj;
	cout << "Loading Objects...\n";
	try
	{
		sphereObj = ofr.read("assets/sphere/sphere.obj");
		saturnRingObj = ofr.read("assets/saturn_ring/ring_huge.obj");
		uranusRingObj = ofr.read("assets/uranus_ring/ring_small.obj");

	}
	catch (const std::exception& e)
	{
		cerr << "Fail to load object file\n";
		cerr << e.what() << endl;
		return -1;
	}
	vector<float> skyboxVert = getSkyboxCube();
	vector<float>& sphereVert = sphereObj.subObjects[0].expandedVertices;
	vector<float>& saturnRingVert = saturnRingObj.subObjects[0].expandedVertices;
	vector<float>& uranusRingVert = uranusRingObj.subObjects[0].expandedVertices;
	cout << "Objects Loaded\n\n";


	// ======== load shaders =========

	cout << "Loading Shaders...\n";
	unsigned int illumShaderProgram = LoadShader("src/shaders/illuminated.vert", "src/shaders/illuminated.frag");
	unsigned int earthShaderProgram = LoadShader("src/shaders/earth.vert", "src/shaders/earth.frag");
	unsigned int basicShaderProgram = LoadShader("src/shaders/basic.vert", "src/shaders/basic.frag");
	unsigned int skyShaderProgram = LoadShader("src/shaders/sky.vert", "src/shaders/sky.frag");
	cout << "Shaders Loaded\n\n";

	vector<unsigned int> shaders{
		basicShaderProgram,
		illumShaderProgram,
		earthShaderProgram,
	};


	// ======= load all textures =======

	cout << "Loading Textures...\n";
	GLuint sunTexture = loadTexture("assets/textures/2k_sun.jpg");
	GLuint mercuryTexture = loadTexture("assets/textures/2k_mercury.jpg");
	GLuint venusTexture = loadTexture("assets/textures/2k_venus_surface.jpg");
	//GLuint venusAtmosphereTexture = loadTexture("assets/textures/2k_venus_atmosphere.jpg");
	GLuint earthTexture = loadTexture("assets/textures/2k_earth_daymap.jpg");
	GLuint earthNightTexture = loadTexture("assets/textures/8k_earth_nightmap.jpg");
	GLuint earthCloudsTexture = loadTexture("assets/textures/2k_earth_clouds.jpg");
	GLuint moonTexture = loadTexture("assets/textures/2k_moon.jpg");
	GLuint marsTexture = loadTexture("assets/textures/2k_mars.jpg");
	GLuint jupiterTexture = loadTexture("assets/textures/2k_jupiter.jpg");
	GLuint saturnTexture = loadTexture("assets/textures/2k_saturn.jpg");
	GLuint saturnRingTexture = loadTexture("assets/textures/saturn_ring_2.png");
	GLuint uranusTexture = loadTexture("assets/textures/2k_uranus.jpg");
	GLuint uranusRingTexture = loadTexture("assets/textures/uranus_ring_2.png");
	GLuint neptuneTexture = loadTexture("assets/textures/2k_neptune.jpg");
	GLuint plutoTexture = loadTexture("assets/textures/pluto.jpg");

	std::vector<std::string> skyboxNames = { "black", "blue", "colorful","grayscale" ,"milkyway","red" };
	std::vector<std::vector<std::string>> fileSets;

	for (const auto& name : skyboxNames) {
		std::vector<std::string> files;
		for (const std::string& direction : { "right", "left", "bottom", "top", "front", "back" }) {
			files.push_back("assets/skybox/" + name + "/" + direction + ".png");
		}
		fileSets.push_back(files);
	}

	std::vector<GLuint> skyTextures;

	for (const auto& files : fileSets) {
		GLuint texture = loadCubemap(files);
		skyTextures.push_back(texture);
	}

	cout << "Textures Loaded\n\n";


	vector<vector<GLuint>> textures{
		{ sunTexture },
		{ mercuryTexture },
		{ venusTexture },
		{ earthTexture, earthCloudsTexture, earthNightTexture},
		{ marsTexture },
		{ jupiterTexture },
		{ saturnTexture },
		{ uranusTexture },
		{ neptuneTexture },
		{ plutoTexture },
		{ moonTexture },
		{ saturnRingTexture },
		{ uranusRingTexture }
	};


	// ======= prepre scene rendering =======

	cout << "Setting Up Scene...\n";
	// gen buffers
	unsigned int sphereVAO, sphereVBO;
	glSetupVertexObject(sphereVAO, sphereVBO, sphereVert, vector<int>{3, 2, 3});
	unsigned int saturnRingVAO, saturnRingVBO;
	glSetupVertexObject(saturnRingVAO, saturnRingVBO, saturnRingVert, vector<int>{3, 2, 3});
	unsigned int uranusRingVAO, uranusRingVBO;
	glSetupVertexObject(uranusRingVAO, uranusRingVBO, uranusRingVert, vector<int>{3, 2, 3});
	unsigned int skyVAO, skyVBO;
	glSetupVertexObject(skyVAO, skyVBO, skyboxVert, vector<int>{3});

	vector<unsigned int> VAOs{
		sphereVAO,
		saturnRingVAO,
		uranusRingVAO
	};

	vector<int> vertexSize{
		(int)sphereVert.size() / 8,
		(int)saturnRingVert.size() / 8,
		(int)uranusRingVert.size() / 8,
	};

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// =========== MODEL & ANIMATION CONFIG ==============


	// model constants
	float SPHERE_OBJECT_RADIUS = 2;		// 3d vertex sphere radius (do not change)

	// model hyper params				(tweak these to adjust scene)

	float distanceModifier = 240;		// master distance margin scale
	float earthScale = 200;				// master scale

	// all these values have to change if customization structure is changed
	int attributeCount = 10;
	vector<float> bodiesCustomization{

		// esthetic
		// a. is animated boolean 
		// s. scale
		// m. margin, the distance between the previous orbiting planet and 
		//		current planet
		// ov. oval ratio, major minor axis ratio
		// rd. randomize spin angle boolean (this is the switch to enable fixed spin 
		//		angle initialization relative to orbit angle, as some object have to be 
		//		parpendicular to other object to make sense)
		// 
		// resource linking
		// pr. parent index (-1: not orbiting, * > -1: orbiting pr when a == 1, 
		//		following pr when a == 0) [refer to this array]
		// bc. body constant index [refer to "bodyConstants" array]
		// vao. VAO and vertex size index [refer to "vertexSize" & "VAOs" array]
		// tx. texture index [refer to "textures" array]
		// mv. model view boolean to disable model view option of certain objects
		//
		// rules: orbited object must come before orbiting object as some calculations 
		//			are depending on their primary object
		// earthIdx MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
		// sunIdx MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
		// 
		//  a		s		m		ov		rd		pr*		bc		vao		tx		mv
			0.f,	0.3f,	0.f,	1.f,	1.f,	-1.f,	0.f,	0.f,	0.f,	1.f, // 0. sun
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	1.f,	0.f,	1.f,	1.f, // 1. mercury
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	2.f,	0.f,	2.f,	1.f, // 2. venus
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	3.f,	0.f,	3.f,	1.f, // 3. earth
			1.f,	1.f,	5.f,	1.f,	0.f,	3.f,	10.f,	0.f,	10.f,	1.f, // 4. moon
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	4.f,	0.f,	4.f,	1.f, // 5. mars
			1.f,	1.f,	55.f,	1.f,	1.f,	0.f,	5.f,	0.f,	5.f,	1.f, // 6. jupiter
			1.f,	1.f,	60.f,	1.f,	1.f,	0.f,	6.f,	0.f,	6.f,	1.f, // 7. saturn
			0.f,	0.9,	0.f,	1.f,	1.f,	7.f,	11.f,	1.f,	11.f,	0.f, // 8. saturn ring
			1.f,	1.f,	100.f,	1.f,	1.f,	0.f,	7.f,	0.f,	7.f,	1.f, // 9. uranus
			0.f,	0.9,	0.f,	1.f,	1.f,	9.f,	12.f,	2.f,	12.f,	0.f, // 10. uranus ring
			1.f,	1.f,	80.f,	1.f,	1.f,	0.f,	8.f,	0.f,	8.f,	1.f, // 11. neptune
			1.f,	1.f,	40.f,	1.f,	1.f,	0.f,	9.f,	0.f,	9.f,	1.f, // 12. pluto
	};
	// TODO: add as much model as possible
	// TODO: earth use multi textures, night city lights

	renderedBodies.resize(bodiesCustomization.size() / attributeCount);
	renderedBodies[sunIdx].position = vec3ToVec(lightPos);

	// get predefined body constants for solar system
	bodyConstants = planetMath.getSolarSystemConstants();


	// ========== math section ============

	// parse configurations
	int animIndexCount = 0;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if ((bool)bodiesCustomization[i * attributeCount])
		{
			renderedBodies[i].animatorIdx = animIndexCount;
			animIndexCount++;
		}
		else
		{
			renderedBodies[i].animatorIdx = -1;
		}
		renderedBodies[i].scaleModifier = bodiesCustomization[i * attributeCount + 1];
		renderedBodies[i].distanceMargin = bodiesCustomization[i * attributeCount + 2];
		renderedBodies[i].ovalRatio = bodiesCustomization[i * attributeCount + 3];
		renderedBodies[i].randomSpinAngle = (bool)bodiesCustomization[i * attributeCount + 4];
		renderedBodies[i].orbitParentIdx = bodiesCustomization[i * attributeCount + 5];
		renderedBodies[i].bodyConstantIdx = bodiesCustomization[i * attributeCount + 6];
		renderedBodies[i].VAOIdx = bodiesCustomization[i * attributeCount + 7];
		renderedBodies[i].textureIdx = bodiesCustomization[i * attributeCount + 8];
		renderedBodies[i].modelViewed = (bool)bodiesCustomization[i * attributeCount + 9];
	}

	// compute scale relative to earth
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (i == earthIdx) continue;
		renderedBodies[i].scale = planetMath.getRelativeValue(bodyConstants[renderedBodies[i].bodyConstantIdx].radius,
			PConst::EARTH_RADIUS, earthScale, renderedBodies[i].scaleModifier);
	}
	renderedBodies[earthIdx].scale = earthScale; // set earth scale

	// compute sphere radius for orbit radius calculation
	vector<float> sphereRadius;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		sphereRadius.push_back(renderedBodies[i].scale * SPHERE_OBJECT_RADIUS);
	}

	// calculate orbit radius distance for individual bodies
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		// TODO following object
		if (renderedBodies[i].orbitParentIdx == -1) continue;

		// find last previous orbiting body that has the same parent 
		// if doesn't exist use current body parent index (next section)
		int preIdx = -1;
		for (int j = i - 1; j > -1; j--)
		{
			if (renderedBodies[i].orbitParentIdx == renderedBodies[j].orbitParentIdx)
			{
				preIdx = j;
				break;
			}
		}

		// calculate orbit radius, if this body is the first object that orbits it's parent then do this
		if (preIdx == -1)
		{
			renderedBodies[i].orbitRadius =
				sphereRadius[renderedBodies[i].orbitParentIdx] + sphereRadius[i] +
				(renderedBodies[i].distanceMargin * distanceModifier);
		}
		// else use previous neighbour's values to calculate orbit radius
		else
		{
			renderedBodies[i].orbitRadius =
				renderedBodies[preIdx].orbitRadius + sphereRadius[preIdx] + sphereRadius[i] +
				(renderedBodies[i].distanceMargin * distanceModifier);
		}
	}

	// calculate delays relative to earth orbiting period
	vector<float> delays;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue;
		if (i == earthIdx) // use delay specified by user
		{
			delays.push_back(earthOrbitDelay);
		}
		else // use delay relative to earth delay
		{
			delays.push_back(planetMath.getRelativeValue(
				bodyConstants[renderedBodies[i].bodyConstantIdx].orbitalPeriod,
				bodyConstants[renderedBodies[earthIdx].bodyConstantIdx].orbitalPeriod, earthOrbitDelay));
		}
	}

	// calculate sum of all parents ascending node angle and sum of all inlinations including it's own
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		// not animated and not following other object
		if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx == -1) continue;
		int parentIndex = renderedBodies[i].orbitParentIdx;
		renderedBodies[i].parentsAscendingNodeSum = planetMath.sumAllAscendingNodes(
			renderedBodies, bodyConstants, parentIndex);
		renderedBodies[i].allInclinationSum = planetMath.sumAllInclinations(
			renderedBodies, bodyConstants, i);
	}

	// initialize animators
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue; // not animated
		int animatorIndex = renderedBodies[i].animatorIdx;
		int bodyConstantIndex = renderedBodies[i].bodyConstantIdx;
		animators.push_back(
			OrbitAnimator(delays[animatorIndex], bodyConstants[bodyConstantIndex].localOrbitalPeriod,
				renderedBodies[i].ovalRatio, renderedBodies[i].orbitRadius, renderedBodies[i].allInclinationSum)
		);
	}



	Gui gui(window, camera, renderedBodies, bodyConstants, earthIdx, earthOrbitDelay, planetMath, animators, sceneState, skyTextures);
	gui.randomizeOrbitAngles();

	cout << "Scene Set up\n";
	cout << "\nLoading Time: " << (int)glfwGetTime() - startLoadingTime << "s\n\n";


	// ==================== RENDER LOOP =========================

	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // set texture to 0

	glUseProgram(earthShaderProgram);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture1"), 0);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture2"), 1);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture3"), 2);

	sceneState.addSPlayTime(glfwGetTime());		// add asset loading time to paused time (rectify animation time)
	//sceneState.pauseScene(glfwGetTime(), true);

	glm::vec3 Xaxis = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 Yaxis = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 Zaxis = glm::vec3(0.f, 0.f, 1.f);

	double previousTime = glfwGetTime();
	double currentTime;
	int fpsCount = 0;

	// Vsync
	//glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window))
	{

		// input
		camera.processInputs(window);

		currentTime = glfwGetTime();
		fpsCount++;
		if (currentTime - previousTime >= 1.0) {

			std::stringstream ss;
			ss << WINDOW_TITLE << " - " << fpsCount << " FPS";
			glfwSetWindowTitle(window, ss.str().c_str());
			fpsCount = 0;
			previousTime = currentTime;
		}

		// animate animated objects (some object might just require rendering but not animating)
		if (sceneState.getCanUpdateAnimation())
		{
			float ms_time = (float)sceneState.getMsPlayTime(glfwGetTime()); // get animation time

			// animate all objects
			for (int i = 0; i < renderedBodies.size(); i++)
			{
				// if object is not aniamated and not orbiting anithing then skip
				if (renderedBodies[i].animatorIdx == -1) continue;

				// if object is not animated but orbiting something then it must be following it
				if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx != -1)
				{
					renderedBodies[i].position = renderedBodies[renderedBodies[i].orbitParentIdx].position;
					continue;
				}

				// current orbiting object's parent index
				int parentIdx = renderedBodies[i].orbitParentIdx;
				int animatorIdx = renderedBodies[i].animatorIdx;

				// parent must update its animated position before the child is, thus parent index < child index
				if (parentIdx > i)
				{
					cout << "revolved celestial bodies must be animated before the it's children\n";
					exit(-1);
				}

				// animate orbit of current object with a origin of parent's position
				animators[animatorIdx].animate(ms_time, 5, 5, sceneState.getJustStarted());

				// retrieve position
				renderedBodies[i].position = animators[animatorIdx].getOrbitPosition();
				renderedBodies[i].rotation = animators[animatorIdx].getSpinAngle();

			}

			sceneState.setJustStarted(false);
		}

		// ImGui

		gui.initFrame();

		// render
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		// view and perspective
		glm::mat4 model = glm::mat4(1.f);
		glm::mat4 view = glm::mat4(1.f);
		glm::mat4 projection = glm::mat4(1.f);
		view = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getOrientation(), camera.getUp());
		projection = glm::perspective(glm::radians(camera.getFOV()),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 10.f, 400000.f);

		for (int i = 0; i < renderedBodies.size(); i++)
		{
			int bcIdx = renderedBodies[i].bodyConstantIdx;
			int txIdx = renderedBodies[i].textureIdx;
			BodyConst& bc = bodyConstants[bcIdx];
			RenderedBody& rb = renderedBodies[i];

			// if object is not animated and not following any other object
			if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx == -1)
			{
				// if object is light source use different shader, default to illum shader
				unsigned int shaderProg = illumShaderProgram;
				if (i == sunIdx) shaderProg = basicShaderProgram;

				glUseProgram(shaderProg);
				model = glm::mat4(1.f);
				model = glm::translate(model, vecToVec3(rb.position)); // sum all position
				model = glm::rotate(model, glm::radians(bc.axialTilt), Zaxis);
				model = glm::scale(model, glm::vec3(rb.scale));
				glSetModelViewProjection(shaderProg, model, view, projection);
				glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[txIdx][0], vertexSize[rb.VAOIdx]);

			}
			// if object is animated or following animated object
			else
			{
				RenderedBody& pr = renderedBodies[rb.orbitParentIdx];

				// use special shader for earth
				if (i == earthIdx) glUseProgram(earthShaderProgram);
				else glUseProgram(illumShaderProgram);

				model = glm::mat4(1.f);
				// rotate using parents' ascending node to rectify orbit shift due to parent's shift of their own ascending node angle
				model = glm::rotate(model, glm::radians(rb.parentsAscendingNodeSum), Yaxis);
				// move world centered orbit to body centered orbit (such as moon orbiting sun translate to moon orbiting earth)
				model = glm::translate(model, vecToVec3(planetMath.sumAllPositions(renderedBodies, rb.orbitParentIdx)));
				// rotate whole orbit to change orbit ascending starting point
				model = glm::rotate(model, glm::radians(bc.ascendingNode), Yaxis);
				// move orbit to world centered orbit position
				model = glm::translate(model, vecToVec3(rb.position));
				// create sphere tilt and add // TODO all parents' inclidnations
				model = glm::rotate(model, glm::radians(bc.axialTilt + rb.allInclinationSum), Zaxis);
				// create sphere spin
				model = glm::rotate(model, glm::radians(rb.rotation), Yaxis);
				// scale to correct size
				model = glm::scale(model, glm::vec3(rb.scale));

				// store actual position for model view camera
				glm::vec4 actualPos = model * glm::vec4(0, 0, 0, 1);
				rb.finalPosition = vec3ToVec(
					glm::vec3(
						actualPos[0] / actualPos[3],
						actualPos[1] / actualPos[3],
						actualPos[2] / actualPos[3]
					)
				);

				// for earth use special shader
				if (i == earthIdx)
				{
					glSetLightingConfig(earthShaderProgram, lightPos, camera, camera.isTorchPressed(), gui);
					glUniform1f(glGetUniformLocation(earthShaderProgram, "light[0].ambientStrength"), gui.getLightIntensityScale());
					glSetModelViewProjection(earthShaderProgram, model, view, projection);
					glBindVertexArray(VAOs[rb.VAOIdx]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][0]);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][1]);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][2]);
					glDrawArrays(GL_TRIANGLES, 0, vertexSize[rb.VAOIdx]);
				}
				else
				{
					glSetLightingConfig(illumShaderProgram, lightPos, camera, camera.isTorchPressed(), gui);
					glSetModelViewProjection(illumShaderProgram, model, view, projection);
					glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[txIdx][0], vertexSize[rb.VAOIdx]);
				}
			}
		}

		// skybox (contains gl code)
		displaySkyBox(skyVAO, gui.getTexture(), skyShaderProgram, view, projection);

		// Gui
		gui.update();
		gui.render();


		// return errors if there's any
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) { cout << "OpenGL Error Occured. Error Code: " << err << endl; }

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}


// ====================== general functions ========================


unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Tek seferde ayarlamalar
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		std::cout << "Texture Loaded: " << path << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}

	// Belleði serbest býrak
	stbi_image_free(data);

	return textureID;
}


unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	std::vector<std::future<unsigned char*>> futures;

	// Paralel olarak görüntüleri yükle
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		futures.push_back(std::async(std::launch::async, [&, i]() {
			return stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			}));
	}

	// Yüklenen görüntüleri iþleme
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = futures[i].get();
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
			std::cout << "Texture Loaded: " << faces[i] << std::endl;
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	// OpenGL ayarlarýný bir kez yap
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


// ============ extra openGL stuff ===============

void displayLoadingScreen(GLFWwindow* window)
{
	// this is not refactored and will never be
	vector<float> rectVert = getRectangle();
	GLuint loadingTexture = loadTexture("assets/loading_screen/loading800p.png");
	unsigned int loadingShaderProgram = LoadShader("src/shaders/load.vert", "src/shaders/load.frag");
	unsigned int loadVAO, loadVBO;
	glSetupVertexObject(loadVAO, loadVBO, rectVert, vector<int>{3, 2});

	glUseProgram(loadingShaderProgram);
	glm::mat4 lsModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
	glm::mat4 lsView = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getOrientation(), camera.getUp());
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lsModel));
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(lsView));

	glDrawVertexTriangles(loadVAO, loadingTexture, 6);
	glfwSwapBuffers(window);
}

void displaySkyBox(unsigned int& VAO, GLuint texture, unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection)
{
	glDepthFunc(GL_LEQUAL);
	glUseProgram(shaderProgram);

	view = glm::mat4(glm::mat3(view)); // remove translation from view matrix
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
}


void glSetupVertexObject(unsigned int& VAO, unsigned int& VBO, vector<float>& data, vector<int> attribLayout)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

	int strideCount = 0;
	for (int i = 0; i < attribLayout.size(); i++)
	{
		if (attribLayout[i] < 1) throw invalid_argument("glSetupVertexObject : attribute layout must be larger than 0");
		strideCount += attribLayout[i];
	}

	int offset = 0;
	for (int i = 0; i < attribLayout.size(); i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, attribLayout[i], GL_FLOAT, GL_FALSE, strideCount * sizeof(float), (void*)(offset * sizeof(float)));
		offset += attribLayout[i];
	}
}

void glDrawVertexTriangles(unsigned int VAO, GLuint texture, int numberOfVertex)
{
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, numberOfVertex);
}

void glSetModelViewProjection(unsigned int shaderProgram, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, Camera cam, int torch, Gui& gui)
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[0].position"), 1, &lightPos[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "light[0].color"), gui.getColors()[0], gui.getColors()[1], gui.getColors()[2]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[0].camPos"), 1, &cam.getPosition()[0]);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].ambientStrength"), gui.getLightIntensityScale());
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].specularStrength"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].shininess"), 16.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].linear"), 0.000000014f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].quadratic"), 0.00000000007f);

	glUniform1i(glGetUniformLocation(shaderProgram, "torchLight"), torch);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].direction"), 1, &cam.getOrientation()[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].position"), 1, &cam.getPosition()[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "light[1].color"), 1.f, 1.f, 1.f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].camPos"), 1, &cam.getPosition()[0]);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].ambientStrength"), 0.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].specularStrength"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].shininess"), 16.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].linear"), 0.0000005f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].quadratic"), 0.000000015f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].phi"), 25.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].gamma"), 35.f);
}

glm::vec3 vecToVec3(vector<float> vec)
{
	return glm::vec3(vec[0], vec[1], vec[2]);
}

vector<float> vec3ToVec(glm::vec3 vec3)
{
	return vector<float>{vec3.x, vec3.y, vec3.z};
}

