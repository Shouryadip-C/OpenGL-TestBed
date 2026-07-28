#pragma once

#include <cstdint>

void runSimulationStep(uint32_t *devPtr, int width, int height, float time);

void initLifeSimulation(int screenWidth, int screenHeight, int cellSize_);
void runLifeSimulationStep(uint32_t *devPtr, int width, int height, float time);
void cleanupLifeSimulation();

void initForestSimulation(int screenWidth, int screenHeight, int cellSize_);
void runForestSimulationStep(uint32_t *devPtr, int width, int height, float time);
void cleanupForestSimulation();
