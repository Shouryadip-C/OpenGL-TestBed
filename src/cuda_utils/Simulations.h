#pragma once

#include <cstdint>

void runSimulationStep(uint32_t *devPtr, int width, int height, float time);

void initLifeSimulation(int screenWidth, int screenHeight, int cellSize_);
void runLifeSimulationStep(uint32_t *devPtr, int width, int height, float time);
void cleanupLifeSimulation();

void initForestSimulation(int screenWidth, int screenHeight, int cellSize_);
void runForestSimulationStep(uint32_t *devPtr, int width, int height, float time);
void cleanupForestSimulation();

struct LBMMouseInput
{
    float x = 0.0f;
    float y = 0.0f;

    float previousX = 0.0f;
    float previousY = 0.0f;

    bool left   = false;
    bool right  = false;
    bool middle = false;
};

void initLBMSimulation(int screenWidth, int screenHeight, int cellSize);
void runLBMSimulationStep(uint32_t *devPtr, int screenWidth, int screenHeight, float time, const LBMMouseInput &mouse);
void cleanupLBMSimulation();
