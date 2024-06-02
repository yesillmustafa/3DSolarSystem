#pragma once

#include <iostream>

class SceneState
{
public:
    SceneState();
    void pauseScene(double time, bool force = false);
    double getMsPlayTime(double time);
    bool getPause();
    void addSPlayTime(double s);
    bool getJustStarted();
    void setJustStarted(bool value);
    bool getCanUpdateAnimation();
private:
    bool pausing = false;
    bool justStarted = true;
    double pauseStartTime = 0.0;
    double totalPausedTime = 0.0;
};
