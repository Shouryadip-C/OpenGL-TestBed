#include "Clock.h"

#include <thread>


Clock::Clock()
  : deltaTime(0.0), elapsedTime(0.0), running(false), targetFPS(0), targetFrameDuration(0.0) {}  // 0 FPS = unlimited

void Clock::start()
{
    startTime     = ClockType::now();
    lastFrameTime = startTime;
    deltaTime     = 0.0;
    elapsedTime   = 0.0;
    running       = true;
}

void Clock::stop()
{
    running = false;
}

void Clock::reset()
{
    start();
}

void Clock::update()
{
    if (!running) return;

    TimePoint                    currentTime   = ClockType::now();
    std::chrono::duration<float> frameDuration = currentTime - lastFrameTime;
    std::chrono::duration<float> totalDuration = currentTime - startTime;

    deltaTime     = frameDuration.count();
    elapsedTime   = totalDuration.count();
    lastFrameTime = currentTime;
}

void Clock::beginFrame()
{
    if (!running) return;
    frameStartTime = ClockType::now();
}

void Clock::setTargetFps(int fps)
{
    if (fps > 0) {
        targetFPS           = fps;
        targetFrameDuration = 1.0 / fps;
    }
    else {
        targetFPS           = 0;
        targetFrameDuration = 0.0;  // Disable FPS cap
    }
}

void Clock::endFrame()
{
    if (!running) return;

    TimePoint                    frameEndTime  = ClockType::now();
    std::chrono::duration<float> frameDuration = frameEndTime - frameStartTime;

    deltaTime   = frameDuration.count();
    elapsedTime = std::chrono::duration<float>(frameEndTime - startTime).count();

    if (targetFrameDuration > 0.0) {
        float sleepTime = targetFrameDuration - deltaTime;
        if (sleepTime > 0.0) {
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
            deltaTime += sleepTime;  // account for the sleep
        }
    }

    lastFrameTime = ClockType::now();
}

int Clock::getTargetFps() const
{
    return targetFPS;
}

float Clock::getDeltaTime() const
{
    return deltaTime;
}

float Clock::getElapsedTime() const
{
    return elapsedTime;
}

float Clock::getFps() const
{
    return deltaTime > 0.0 ? 1.0 / deltaTime : 0.0;
}

bool Clock::isRunning() const
{
    return running;
}
