#pragma once

#include <chrono>


class Clock
{
public:
    Clock();

    void start();
    void stop();
    void reset();
    void update();  // Still valid for manual use if needed

    void beginFrame();  // Call at the start of a frame
    void endFrame();    // Call at the end of a frame (does sleep)

    void setTargetFps(int fps);
    int  getTargetFps() const;

    float getDeltaTime() const;
    float getElapsedTime() const;
    float getFps() const;

    bool isRunning() const;

private:
    using ClockType = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<ClockType>;

    TimePoint startTime;
    TimePoint lastFrameTime;
    TimePoint frameStartTime;

    float deltaTime;
    float elapsedTime;
    bool  running;

    int   targetFPS;
    float targetFrameDuration;  // In seconds
};
