#include "Simulations.h"

// cuda
#include "CudaInterop.h"
#include <device_launch_parameters.h>

__device__ unsigned char *d_cells = nullptr;
__device__ unsigned char *d_next  = nullptr;

static int gridWidth  = 0;
static int gridHeight = 0;
static int cellSize   = 10;


__global__ void simulate(uint32_t *pixels, int width, int height, float t)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    int idx = y * width + x;

    // Simple evolving pattern (replace with Game of Life or forest fire logic)
    unsigned char r = (unsigned char)(127.5f * (sinf(x * 0.05f + t) + 1.0f));
    unsigned char g = (unsigned char)(127.5f * (sinf(y * 0.05f + t) + 1.0f));
    unsigned char b = (unsigned char)(127.5f * (sinf((x + y) * 0.02f + t) + 1.0f));
    pixels[idx]     = (255 << 24) | (b << 16) | (g << 8) | r;
}

void runSimulationStep(uint32_t *devPtr, int width, int height, float time)
{
    dim3 block(16, 16);
    dim3 grid((width + 15) / 16, (height + 15) / 16);
    simulate<<<grid, block>>>(devPtr, width, height, time);
    CUDA_CHECK(cudaDeviceSynchronize());
}


__global__ void updateLife(unsigned char *current, unsigned char *next, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    int idx            = y * width + x;
    int aliveNeighbors = 0;

    // Count alive neighbors
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + width) % width;
            int ny = (y + dy + height) % height;
            aliveNeighbors += current[ny * width + nx];
        }

    unsigned char alive = current[idx];

    // Standard Game of Life rules
    next[idx] = (alive && (aliveNeighbors == 2 || aliveNeighbors == 3)) || (!alive && aliveNeighbors == 3);
}

__global__ void renderLifeCells(uint32_t      *pixels,
                                unsigned char *cells,
                                int            screenWidth,
                                int            screenHeight,
                                int            cellSize,
                                int            gridWidth,
                                int            gridHeight)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= screenWidth || y >= screenHeight) return;

    int cellX = x / cellSize;
    int cellY = y / cellSize;
    if (cellX >= gridWidth || cellY >= gridHeight) return;

    unsigned char alive    = cells[cellY * gridWidth + cellX];
    bool          gridLine = (x % cellSize == 0) || (y % cellSize == 0);
    // Turn off gridlines
    // bool gridLine = 0;

    // Colors in ARGB (0xAABBGGRR)
    const uint32_t col_background   = 0xFF0A0A0A;  // (10,10,10)
    const uint32_t col_about_to_die = 0xFF208FF5;  // (245,143,32)
    const uint32_t col_alive        = 0xFF4D20CA;  // (202,32,77)

    // Count neighbors again for “about to die” coloring
    int aliveNeighbors = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = (cellX + dx + gridWidth) % gridWidth;
            int ny = (cellY + dy + gridHeight) % gridHeight;
            aliveNeighbors += cells[ny * gridWidth + nx];
        }

    uint32_t color;
    if (!gridLine) {
        if (alive) {
            // “About to die” = alive but will die next frame
            if (aliveNeighbors < 2 || aliveNeighbors > 3)
                color = col_about_to_die;
            else
                color = col_alive;
        }
        else {
            color = col_background;
        }
    }
    else {
        color = col_background;
    }

    pixels[y * screenWidth + x] = color;
}

void initLifeSimulation(int screenWidth, int screenHeight, int cellSize_)
{
    cellSize   = cellSize_;
    gridWidth  = screenWidth / cellSize;
    gridHeight = screenHeight / cellSize;

    size_t size = gridWidth * gridHeight * sizeof(unsigned char);
    CUDA_CHECK(cudaMalloc(&d_cells, size));
    CUDA_CHECK(cudaMalloc(&d_next, size));

    // Randomize initial state
    unsigned char *h_init = new unsigned char[gridWidth * gridHeight];
    for (int i = 0; i < gridWidth * gridHeight; ++i) h_init[i] = rand() % 2;
    CUDA_CHECK(cudaMemcpy(d_cells, h_init, size, cudaMemcpyHostToDevice));
    delete[] h_init;
}

void runLifeSimulationStep(uint32_t *devPtr, int width, int height, float time)
{
    if (d_cells == nullptr) {
        initLifeSimulation(width, height, 10);  // 10-pixel cells
    }

    dim3 block(16, 16);
    dim3 grid((gridWidth + 15) / 16, (gridHeight + 15) / 16);
    updateLife<<<grid, block>>>(d_cells, d_next, gridWidth, gridHeight);
    CUDA_CHECK(cudaDeviceSynchronize());

    // Swap buffers
    unsigned char *tmp = d_cells;
    d_cells            = d_next;
    d_next             = tmp;

    // Render to the shared OpenGL texture
    dim3 block2(16, 16);
    dim3 grid2((width + 15) / 16, (height + 15) / 16);
    renderLifeCells<<<grid2, block2>>>(devPtr, d_cells, width, height, cellSize, gridWidth, gridHeight);
    CUDA_CHECK(cudaDeviceSynchronize());
}

void cleanupLifeSimulation()
{
    if (d_cells) {
        CUDA_CHECK(cudaFree(d_cells));
    }
    if (d_next) {
        CUDA_CHECK(cudaFree(d_next));
    }
    d_cells = d_next = nullptr;
}
