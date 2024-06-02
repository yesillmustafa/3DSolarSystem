#include "Gui.h"

Gui::Gui(GLFWwindow* window,
    Camera& camera,
    std::vector<RenderedBody>& renderedBodies,
    std::vector<BodyConst>& bodyConstants,
    int& earthIdx,
    float& earthOrbitDelay,
    PlanetMath& planetMath,
    vector<OrbitAnimator>& animators,
    SceneState& sceneState,
    vector<GLuint>& skyboxTextures)
    : camera(camera),
    renderedBodies(renderedBodies),
    bodyConstants(bodyConstants),
    window(window),
    earthIdx(earthIdx),
    earthOrbitDelay(earthOrbitDelay),
    planetMath(planetMath),
    animators(animators),
    sceneState(sceneState),
    skyboxTextures(skyboxTextures)
{

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    currentSkybox = skyboxTextures[4];
}

Gui::~Gui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::initFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::update()
{
    if (randomizedOrbitAnglePressed)
        randomizeOrbitAngles();

    if (pausedPressed)
    {
        isPaused = !isPaused;
        if (isPaused)
        {
            sceneState.pauseScene(glfwGetTime(),true);
        }
        else
        {
            sceneState.pauseScene(glfwGetTime(), false);
        }
    }

    // Mevcut buton kontrol mekanizmasý
    if (freePressed && camera.getCameraMode()) {
        newFocusState = FocusState::NONE;
        camera.setCameraMode(false);
    }
    else if (sunPressed)
        newFocusState = FocusState::SUN;
    else if (mercuryPressed)
        newFocusState = FocusState::MERCURY;
    else if (venusPressed)
        newFocusState = FocusState::VENUS;
    else if (earthPressed)
        newFocusState = FocusState::EARTH;
    else if (moonPressed)
        newFocusState = FocusState::MOON;
    else if (marsPressed)
        newFocusState = FocusState::MARS;
    else if (jupiterPressed)
        newFocusState = FocusState::JUPITER;
    else if (saturnPressed)
        newFocusState = FocusState::SATURN;
    else if (uranusPressed)
        newFocusState = FocusState::URANUS;
    else if (neptunePressed)
        newFocusState = FocusState::NEPTUNE;
    else if (plutoPressed)
        newFocusState = FocusState::PLUTO;

    if (newFocusState != focusState) {
        focusState = newFocusState;
    }

    // Kameranýn konumunu her karede güncelle
    switch (focusState) {
    case FocusState::SUN:
        focusOnPlanet(0);
        break;
    case FocusState::MERCURY:
        focusOnPlanet(1);
        break;
    case FocusState::VENUS:
        focusOnPlanet(2);
        break;
    case FocusState::EARTH:
        focusOnPlanet(3);
        break;
    case FocusState::MOON:
        focusOnPlanet(10);
        break;
    case FocusState::MARS:
        focusOnPlanet(4);
        break;
    case FocusState::JUPITER:
        focusOnPlanet(5);
        break;
    case FocusState::SATURN:
        focusOnPlanet(6);
        break;
    case FocusState::URANUS:
        focusOnPlanet(7);
        break;
    case FocusState::NEPTUNE:
        focusOnPlanet(8);
        break;
    case FocusState::PLUTO:
        focusOnPlanet(9);
        break;
    case FocusState::NONE:
    default:
        break;
    }

    freePressed = false;
    sunPressed = false;
    mercuryPressed = false;
    venusPressed = false;
    earthPressed = false;
    moonPressed = false;
    marsPressed = false;
    jupiterPressed = false;
    saturnPressed = false;
    uranusPressed = false;
    neptunePressed = false;
    plutoPressed = false;

    displayCameraWindow();
    displaySettingsWindow();
    displayInfoWindow();

    if (torchPressed) camera.activateTorch();
    else camera.deactivateTorch();
}

void Gui::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

FocusState Gui::getFocusState() const
{
    return focusState;
}

GLuint Gui::getTexture() const
{
    return currentSkybox;
}

const float* Gui::getColors() const
{
    return colors;
}

float Gui::getLightIntensityScale() const
{
    return lightIntensityScale;
}

void Gui::randomizeOrbitAngles()
{
    for (int i = 0; i < renderedBodies.size(); i++)
    {
        if (renderedBodies[i].animatorIdx == -1) continue; // not animated
        int animatorIndex = renderedBodies[i].animatorIdx;
        int randAngle1 = rand() % 360;
        int randAngle2 = rand() % 360;
        animators[animatorIndex].setOrbitAngle(randAngle1); // randomize initial orbit angle
        if (renderedBodies[i].randomSpinAngle)
        {
            animators[animatorIndex].setSpinAngle(randAngle2);
        }
        else
        {
            animators[animatorIndex].setSpinAngle(
                randAngle1 + bodyConstants[renderedBodies[i].bodyConstantIdx].defaultSpinAngle);
        }
    }
}

void Gui::focusOnPlanet(const int& planetBodyID)
{

    for (const auto& body : renderedBodies)
    {
        if (body.bodyConstantIdx == planetBodyID)
        {
            std::vector<float> m_finalPosition = body.finalPosition;
            glm::vec3 finalPositionVec3 = glm::vec3(m_finalPosition[0], m_finalPosition[1], m_finalPosition[2]);
            glm::vec3 newCameraPosition = finalPositionVec3;

            float modelViewDistance = bodyConstants[planetBodyID].radius / 4.0f;
            if (planetBodyID == 0) modelViewDistance = bodyConstants[planetBodyID].radius / 16.0f;
            // Kamerayý yeni pozisyona ayarla
            camera.moveAndOrientCamera(newCameraPosition, modelViewDistance);
            camera.setCameraMode(true);
            break;
        }
    }
}

void Gui::button(const std::string& label, const std::string& info, bool* value)
{
    if (ImGui::Button(label.c_str()))
        *value = true;
    else
        *value = false;
    ImGui::SameLine();
    ImGui::Text(info.c_str());
}

void Gui::displayCameraWindow()
{
    ImGui::Begin("CAMERA");

    fovScale = camera.getFOV();
    ImGui::SliderFloat("##Zoom", &fovScale, 70.0, 15.0, "- Zoom +");
    camera.setFOV(fovScale);

    button("FREE   ", "Press to free the camera", &freePressed);
    button("Sun    ", "Press to observe the Sun", &sunPressed);
    button("Mercury", "Press to observe Mercury", &mercuryPressed);
    button("Venus  ", "Press to observe Venus", &venusPressed);
    button("Earth  ", "Press to observe Earth", &earthPressed);
    button("Moon   ", "Press to observe Moon", &moonPressed);
    button("Mars   ", "Press to observe Mars", &marsPressed);
    button("Jupiter", "Press to observe Jupiter", &jupiterPressed);
    button("Saturn ", "Press to observe Saturn", &saturnPressed);
    button("Uranus ", "Press to observe Uranus", &uranusPressed);
    button("Neptune", "Press to observe Neptune", &neptunePressed);
    button("Pluto  ", "Press to observe Pluto", &plutoPressed);

    ImGui::End();
}

void Gui::displaySettingsWindow()
{
    ImGui::Begin("SETTINGS");

    ImGui::Text("Display:");
    if (ImGui::RadioButton("FullScreen", isFullscreen))
    {
        isFullscreen = true;
        setWindowMode(isFullscreen);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Windowed", !isFullscreen))
    {
        isFullscreen = false;
        setWindowMode(isFullscreen);

    }

    // Resolution dropdown menüsü
    static const char* resolutionOptions[] = { "1280 x 800", "1600 x 900", "1920 x 1080" };
    if (ImGui::BeginCombo("Resolution", resolutionOptions[selectedResolution])) {
        for (int i = 0; i < IM_ARRAYSIZE(resolutionOptions); i++) {
            bool isSelected = (selectedResolution == i);
            if (ImGui::Selectable(resolutionOptions[i], isSelected)) {
                selectedResolution = i;
                // Seçilen çözünürlüðe göre iþlemler
                if (selectedResolution == 0) {

                    windowWidth = 1280;
                    windowHeight = 800;
                    setResolution(windowWidth, windowHeight);

                }
                else if (selectedResolution == 1) {

                    windowWidth = 1600;
                    windowHeight = 900;
                    setResolution(windowWidth, windowHeight);
                }
                else if (selectedResolution == 2) {

                    windowWidth = 1920;
                    windowHeight = 1080;
                    setResolution(windowWidth, windowHeight);
                }
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus(); // Varsayýlan olarak seçili olan öðeyi vurgula
        }
        ImGui::EndCombo();
    }

    //ImGui::Text("Audio:");

    //ImGui::SliderInt("Volume", &volume, 0, 100, "%d%%");
    //static const char* soundOptions[] = { "sound1", "sound2", "sound3" };
    //static int selectedSound = 0;
    //if (ImGui::BeginCombo("Sound", soundOptions[selectedSound])) {
    //    for (int i = 0; i < IM_ARRAYSIZE(soundOptions); i++) {
    //        bool isSelected = (selectedSound == i);
    //        if (ImGui::Selectable(soundOptions[i], isSelected)) {
    //            selectedSound = i;
    //            if (selectedSound == 0) {



    //            }
    //            else if (selectedSound == 1) {


    //            }
    //            else if (selectedSound == 2) {


    //            }
    //        }
    //        if (isSelected)
    //            ImGui::SetItemDefaultFocus(); // Varsayýlan olarak seçili olan öðeyi vurgula
    //    }
    //    ImGui::EndCombo();
    //}

    ImGui::Text("Scene:");

    ImGui::SliderFloat("Time", &earthOrbitDelay, 1., 3600., "1 yr : %.0f sec");
    if (earthOrbitDelay < 1.0f) earthOrbitDelay = 1.0f;
    if (earthOrbitDelay > 3600.0f) earthOrbitDelay = 3600.0f;
    updateAnimatorsDelays();

    ImGui::SliderFloat("Light intensity", &lightIntensityScale, 0.0, 1.0, "%.2f");
    ImGui::ColorEdit3("Light color", colors);

    static const char* backgroundOptions[] = { "All Black", "Blue", "Colorful","Grayscale" ,"Milky Way","Red" };
    static int selectedBackground = 4;
    if (ImGui::BeginCombo("Background", backgroundOptions[selectedBackground])) {
        for (int i = 0; i < IM_ARRAYSIZE(backgroundOptions); i++) {
            bool isSelected = (selectedBackground == i);
            if (ImGui::Selectable(backgroundOptions[i], isSelected)) {
                selectedBackground = i;
                if (selectedBackground >= 0 && selectedBackground < 21) {
                    currentSkybox = skyboxTextures[selectedBackground];
                }
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus(); // Varsayýlan olarak seçili olan öðeyi vurgula
        }
        ImGui::EndCombo();
    }


    button("Random Orbit", "", &randomizedOrbitAnglePressed);
    ImGui::SameLine();
    // Play/Pause butonu
    button(isPaused ? "Play" : "Pause", "", &pausedPressed);



    torchPressed = camera.isTorchPressed();
    ImGui::Checkbox("Torch Light (F)", &torchPressed);





    ImGui::End();
}

void Gui::displayInfoWindow()
{
    ImGui::Begin("INFO");
    ImGui::PushItemWidth(500);
    switch (newFocusState) {
    case FocusState::NONE: {
        ImGui::Text("Free camera, use:");
        ImGui::Bullet(); ImGui::Text("WSAD          to move around");
        ImGui::Bullet(); ImGui::Text("LEFT MOUSE    to rotate camera");
        ImGui::Bullet(); ImGui::Text("SPACE         to move up");
        ImGui::Bullet(); ImGui::Text("LEFT CTRL     to move down");
        ImGui::Bullet(); ImGui::Text("SHIFT         to increase velocity");
        ImGui::Bullet(); ImGui::Text("ESC           to exit");

    }
                         break;

    case FocusState::SUN: {
        ImGui::Text("SUN");
        ImGui::Bullet(); ImGui::Text("Radius:       695 508 km");
        ImGui::Bullet(); ImGui::Text("Mass:         1,989 * 10^30 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  5 778 K = 5 504 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,603 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            274 m/s²");
    }
                        break;

    case FocusState::MERCURY: {
        ImGui::Text("MERCURY");
        ImGui::Bullet(); ImGui::Text("Radius:       2 440 km");
        ImGui::Bullet(); ImGui::Text("Mass:         3,285 * 10^23 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  700 K = 427 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,503 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            3,7 m/s²");
    }
                            break;

    case FocusState::VENUS: {
        ImGui::Text("VENUS");
        ImGui::Bullet(); ImGui::Text("Radius:       6 052 km");
        ImGui::Bullet(); ImGui::Text("Mass:         4,867 * 10^24 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  735 K = 462 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,503 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            8,87 m/s²");
    }
                          break;

    case FocusState::EARTH: {
        ImGui::Text("EARTH");
        ImGui::Bullet(); ImGui::Text("Radius:       6 371 km");
        ImGui::Bullet(); ImGui::Text("Mass:         5,972 * 10^24 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  287 K = 14 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,543 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            9,81 m/s²");
    }
                          break;

    case FocusState::MOON: {
        ImGui::Text("MOON");
        ImGui::Bullet(); ImGui::Text("Radius:       1 737 km");
        ImGui::Bullet(); ImGui::Text("Mass:         7,346 * 10^22 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  300 K = 27 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,543 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            1,62 m/s²");
    }
                         break;

    case FocusState::MARS: {
        ImGui::Text("MARS");
        ImGui::Bullet(); ImGui::Text("Radius:       3 390 km");
        ImGui::Bullet(); ImGui::Text("Mass:         6,39 * 10^23 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  213 K = -60 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,603 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            3,71 m/s²");
    }
                         break;

    case FocusState::JUPITER: {
        ImGui::Text("JUPITER");
        ImGui::Bullet(); ImGui::Text("Radius:       69 911 km");
        ImGui::Bullet(); ImGui::Text("Mass:         1,898 * 10^27 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  128 K = -145 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,503 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            24,79 m/s²");
    }
                            break;

    case FocusState::SATURN: {
        ImGui::Text("SATURN");
        ImGui::Bullet(); ImGui::Text("Radius:       58 232 km");
        ImGui::Bullet(); ImGui::Text("Mass:         5,683 * 10^26 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  95 K = -178 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,603 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            10,44 m/s²");
    }
                           break;

    case FocusState::URANUS: {
        ImGui::Text("URANUS");
        ImGui::Bullet(); ImGui::Text("Radius:       25 362 km");
        ImGui::Bullet(); ImGui::Text("Mass:         8,681 * 10^25 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  49 K = -224 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,503 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            8,87 m/s²");
    }
                           break;

    case FocusState::NEPTUNE: {
        ImGui::Text("NEPTUNE");
        ImGui::Bullet(); ImGui::Text("Radius:       24 622 km");
        ImGui::Bullet(); ImGui::Text("Mass:         1,024 * 10^26 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  59 K = -214 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,503 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            11,15 m/s²");
    }
                            break;
    case FocusState::PLUTO: {
        ImGui::Text("PLUTO");
        ImGui::Bullet(); ImGui::Text("Radius:       1 188 km");
        ImGui::Bullet(); ImGui::Text("Mass:         1,303 * 10^22 kg");
        ImGui::Bullet(); ImGui::Text("Temperature:  38 K = -235 C");
        ImGui::Bullet(); ImGui::Text("Age:          4,603 * 10^9 years");
        ImGui::Bullet(); ImGui::Text("g:            0.62 m/s²");
    }
                          break;
    }

    ImGui::End();
}

void Gui::updateAnimatorsDelays()
{
    // calculate delays relative to earth orbiting period
    std::vector<float> delays;
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

    for (int i = 0; i < renderedBodies.size(); i++)
    {
        if (renderedBodies[i].animatorIdx == -1) continue; // not animated
        int animatorIndex = renderedBodies[i].animatorIdx;
        animators[animatorIndex].setOrbitalDelay(delays[animatorIndex]);
    }
}

void Gui::setResolution(int width, int height)
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int posX = (mode->width - width) / 2;
    int posY = (mode->height - height) / 2;
    glfwSetWindowSize(window, width, height);
    glfwSetWindowPos(window, posX, posY);
}

void Gui::setWindowMode(bool fullscreen)
{
    if (isFullscreen)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        windowWidth = mode->width;
        windowHeight = mode->height;
        selectedResolution = 2;
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, windowWidth, windowHeight, mode->refreshRate);
    }
    else
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        int posX = (mode->width - windowWidth) / 2;
        int posY = (mode->height - windowHeight) / 2;
        glfwSetWindowMonitor(window, NULL, posX, posY, windowWidth, windowHeight, 0);
    }
}
