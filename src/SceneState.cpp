#include "SceneState.h"

SceneState::SceneState() {}

void SceneState::pauseScene(double time, bool force)
{
    if ((pausing && !force) || (!pausing && force))
    {
        if (!pausing)
        {
            pauseStartTime = time;
        }
        else
        {
            totalPausedTime += time - pauseStartTime;
        }
        pausing = !pausing;
    }
}

double SceneState::getMsPlayTime(double time)
{
    return (time - totalPausedTime) * 1000.0;
}

bool SceneState::getPause()
{
    return pausing;
}

void SceneState::addSPlayTime(double s)
{
    totalPausedTime += s;
}

bool SceneState::getJustStarted()
{
    return justStarted;
}

void SceneState::setJustStarted(bool value)
{
    justStarted = value;
}

bool SceneState::getCanUpdateAnimation()
{
    return !pausing || justStarted;
}
