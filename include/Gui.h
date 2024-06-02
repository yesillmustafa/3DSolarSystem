#pragma once

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <string>
#include <vector>

#include "Camera.h"
#include "PlanetMath.h"
#include "OrbitAnimator.h"
#include "SceneState.h"

using namespace std;

enum class FocusState {
    NONE,
    SUN,
    MERCURY,
    VENUS,
    EARTH,
    MOON,
    MARS,
    JUPITER,
    SATURN,
    URANUS,
    NEPTUNE,
    PLUTO
};

class Gui
{
public:
    Gui(GLFWwindow* window,
        Camera& camera,
        std::vector<RenderedBody>& renderedBodies,
        std::vector<BodyConst>& bodyConstants,
        int& earthIdx,
        float& earthOrbitDelay,
        PlanetMath& planetMath,
        vector<OrbitAnimator>& animators,
        SceneState& sceneState,
        vector<GLuint>& skyboxTextures
    );
    ~Gui();

    void initFrame();
    void update();
    void render();

    FocusState getFocusState() const;
    GLuint getTexture() const;
    const float* getColors() const;
    float getLightIntensityScale() const;

    void randomizeOrbitAngles();


private:
    GLFWwindow* window;
    Camera& camera;
    std::vector<RenderedBody>& renderedBodies;
    std::vector<BodyConst>& bodyConstants;
    PlanetMath& planetMath;
    vector<OrbitAnimator>& animators;
    SceneState& sceneState;

    std::vector<GLuint>& skyboxTextures;
    GLuint currentSkybox;

    int& earthIdx;
    float& earthOrbitDelay;

    bool randomizedOrbitAnglePressed;
    bool torchPressed;
    bool isPaused = false;
    bool pausedPressed;
    bool isFullscreen = false;

    int windowWidth = 1280;
    int windowHeight = 800;
    int selectedResolution = 0;

    int volume = 100;


    int selectedItem = 0;

    bool freePressed;
    bool sunPressed;
    bool mercuryPressed;
    bool venusPressed;
    bool earthPressed;
    bool moonPressed;
    bool marsPressed;
    bool jupiterPressed;
    bool saturnPressed;
    bool uranusPressed;
    bool neptunePressed;
    bool plutoPressed;

    float timeScale = 1.0f;
    float fovScale;
    float lightIntensityScale = 0.06f;
    float colors[3] = { 1, 1, 1 };

    FocusState newFocusState = FocusState::NONE;
    FocusState focusState = FocusState::NONE;
    void focusOnPlanet(const int& planetBodyID);
    void button(const std::string& label, const std::string& info, bool* value);
    void displayCameraWindow();
    void displaySettingsWindow();
    void displayInfoWindow();

    void updateAnimatorsDelays();

    void setWindowMode(bool fullscreen);
    void setResolution(int width, int height);
};