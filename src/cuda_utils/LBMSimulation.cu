#include "CudaInterop.h"
#include "Simulations.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

// ============================================================
// LBM D2Q9
// ============================================================
//
// Coordinate convention:
//   - Mouse coordinates are GLFW screen coordinates.
//   - (0, 0) is the TOP-LEFT of the window.
//   - The LBM lattice uses the same coordinate convention.
//   - Therefore NO Y FLIP is performed here.
//
// Visualization:
//   - Default mode is velocity magnitude.
//   - Low velocity  -> blue
//   - Increasing    -> cyan -> green -> yellow
//   - High velocity -> red
//
// Interaction:
//   - Left mouse   : inject/stir velocity
//   - Right mouse  : draw solid wall
//   - Middle mouse : erase solid wall
//
// The implementation uses:
//   collision -> post-collision distributions
//   pull-streaming -> next distributions
//
// Three distribution buffers are intentional. Keeping collision and
// streaming separate makes the algorithm easier to understand and
// maps naturally to a future hardware implementation.
// ============================================================

namespace {

constexpr int Q = 9;

// ------------------------------------------------------------
// LBM parameters
// ------------------------------------------------------------

// tau > 0.5 is required for the BGK model to remain stable.
constexpr float TAU   = 0.6f;
constexpr float OMEGA = 1.0f / TAU;

// Mouse force is deliberately modest so the simulation does not
// immediately become unstable.
constexpr float MOUSE_FORCE = 0.025f;

// Radius in lattice cells.
// Small enough for precise wall drawing.
constexpr float MOUSE_RADIUS = 3.0f;

// Maximum lattice velocity used by the interactive source.
constexpr float MAX_SPEED = 0.20f;

// Velocity scale used only for visualization.
constexpr float VISUALIZATION_SPEED = 0.12f;

// ------------------------------------------------------------
// D2Q9
// ------------------------------------------------------------
//
//        6  2  5
//         \ | /
//        3--0--1
//         / | \
//        7  4  8
//
// +X = right
// +Y = down, because the lattice follows screen coordinates.
// ------------------------------------------------------------

__constant__ int CX[Q] = { 0, 1, 0, -1, 0, 1, -1, -1, 1 };

__constant__ int CY[Q] = { 0, 0, 1, 0, -1, 1, 1, -1, -1 };

// D2Q9 weights.
__constant__ float WEIGHT[Q] = { 4.0f / 9.0f,  1.0f / 9.0f,  1.0f / 9.0f,  1.0f / 9.0f, 1.0f / 9.0f,
                                 1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f };

// Opposite direction used for bounce-back.
__constant__ int OPPOSITE[Q] = { 0, 3, 4, 1, 2, 7, 8, 5, 6 };

// ------------------------------------------------------------
// Device state
// ------------------------------------------------------------
//
// Structure of arrays:
//     f[q * totalCells + cell]
//
// d_f       = current distributions
// d_fPost   = distributions after collision
// d_fNext   = distributions after streaming
//
// Having an explicit post-collision buffer avoids race conditions
// and stale values around obstacle cells.
// ------------------------------------------------------------

float *d_f     = nullptr;
float *d_fPost = nullptr;
float *d_fNext = nullptr;

unsigned char *d_obstacle = nullptr;

int gridWidth  = 0;
int gridHeight = 0;
int cellSize   = 4;

// ------------------------------------------------------------
// Small device helpers
// ------------------------------------------------------------

__device__ float equilibrium(int direction, float rho, float ux, float uy)
{
    float cx = static_cast<float>(CX[direction]);
    float cy = static_cast<float>(CY[direction]);

    float uSquared = ux * ux + uy * uy;
    float cu       = cx * ux + cy * uy;

    return WEIGHT[direction] * rho * (1.0f + 3.0f * cu + 4.5f * cu * cu - 1.5f * uSquared);
}

__device__ float pointSegmentDistanceSquared(float px, float py, float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0;
    float dy = y1 - y0;

    float lengthSquared = dx * dx + dy * dy;

    if (lengthSquared < 1.0e-8f) {
        float ox = px - x0;
        float oy = py - y0;
        return ox * ox + oy * oy;
    }

    float t = ((px - x0) * dx + (py - y0) * dy) / lengthSquared;
    t       = fmaxf(0.0f, fminf(1.0f, t));

    float closestX = x0 + t * dx;
    float closestY = y0 + t * dy;

    float ox = px - closestX;
    float oy = py - closestY;

    return ox * ox + oy * oy;
}

// ------------------------------------------------------------
// Initialization
// ------------------------------------------------------------

__global__ void initializeLBM(float *f, float *fPost, float *fNext, unsigned char *obstacle, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int cell       = y * width + x;
    int totalCells = width * height;

    obstacle[cell] = 0;

    // Initial condition:
    // rho = 1
    // ux  = 0
    // uy  = 0
    //
    // Therefore f_i = w_i.
    for (int q = 0; q < Q; ++q) {
        float value = WEIGHT[q];

        f[q * totalCells + cell]     = value;
        fPost[q * totalCells + cell] = value;
        fNext[q * totalCells + cell] = value;
    }
}

// ------------------------------------------------------------
// Mouse wall brush
// ------------------------------------------------------------
//
// The brush follows the line between the previous and current
// mouse positions. This prevents gaps when the mouse moves quickly.
//
// right  = add wall
// middle = erase wall
// ------------------------------------------------------------

__global__ void updateObstacles(unsigned char *obstacle,
                                int            width,
                                int            height,
                                float          mouseX,
                                float          mouseY,
                                float          previousMouseX,
                                float          previousMouseY,
                                bool           rightMouse,
                                bool           middleMouse)
{
    if (!rightMouse && !middleMouse) return;

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    float distanceSquared = pointSegmentDistanceSquared(static_cast<float>(x), static_cast<float>(y), previousMouseX,
                                                        previousMouseY, mouseX, mouseY);

    if (distanceSquared > MOUSE_RADIUS * MOUSE_RADIUS) return;

    int cell = y * width + x;

    // Right mouse has priority if both buttons happen to be held.
    if (rightMouse) {
        obstacle[cell] = 1;
    }
    else if (middleMouse) {
        obstacle[cell] = 0;
    }
}

// ------------------------------------------------------------
// Collision
// ------------------------------------------------------------
//
// BGK collision:
//
//     f_i' = f_i + omega * (f_eq - f_i)
//
// The mouse acts as a simple interactive velocity source.
// This is intentionally an educational interaction mechanism,
// not a full Guo forcing implementation.
// ------------------------------------------------------------

__global__ void lbmCollision(const float         *f,
                             float               *fPost,
                             const unsigned char *obstacle,
                             int                  width,
                             int                  height,
                             float                mouseX,
                             float                mouseY,
                             float                previousMouseX,
                             float                previousMouseY,
                             bool                 leftMouse)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int cell       = y * width + x;
    int totalCells = width * height;

    // Keep obstacle distributions around so the streaming kernel
    // can use them for bounce-back. Obstacle cells are not rendered
    // as fluid.
    if (obstacle[cell]) {
        for (int q = 0; q < Q; ++q) {
            fPost[q * totalCells + cell] = f[OPPOSITE[q] * totalCells + cell];
        }
        return;
    }

    float localF[Q];

    float rho = 0.0f;
    float ux  = 0.0f;
    float uy  = 0.0f;

    for (int q = 0; q < Q; ++q) {
        float value = f[q * totalCells + cell];

        localF[q] = value;

        rho += value;
        ux += value * static_cast<float>(CX[q]);
        uy += value * static_cast<float>(CY[q]);
    }

    if (rho < 1.0e-6f) {
        rho = 1.0f;
        ux  = 0.0f;
        uy  = 0.0f;
    }
    else {
        ux /= rho;
        uy /= rho;
    }

    // --------------------------------------------------------
    // Mouse interaction
    // --------------------------------------------------------

    if (leftMouse) {
        float dx = static_cast<float>(x) - mouseX;
        float dy = static_cast<float>(y) - mouseY;

        float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared < MOUSE_RADIUS * MOUSE_RADIUS) {
            // Mouse movement is already in screen/lattice coordinates.
            // Moving upward therefore gives a negative Y velocity,
            // which is visually upward.
            float mouseVX = mouseX - previousMouseX;
            float mouseVY = mouseY - previousMouseY;

            // Avoid a huge force if the cursor jumps between frames.
            float mouseSpeedSquared = mouseVX * mouseVX + mouseVY * mouseVY;

            constexpr float MAX_MOUSE_DELTA = 8.0f;

            if (mouseSpeedSquared > MAX_MOUSE_DELTA * MAX_MOUSE_DELTA) {

                float scale = MAX_MOUSE_DELTA / sqrtf(mouseSpeedSquared);

                mouseVX *= scale;
                mouseVY *= scale;
            }

            mouseVX *= MOUSE_FORCE;
            mouseVY *= MOUSE_FORCE;

            float distance  = sqrtf(distanceSquared);
            float influence = 1.0f - distance / MOUSE_RADIUS;
            influence       = fmaxf(0.0f, influence);

            ux += mouseVX * influence;
            uy += mouseVY * influence;
        }
    }

    // --------------------------------------------------------
    // Velocity clamp
    // --------------------------------------------------------

    float speedSquared = ux * ux + uy * uy;

    if (speedSquared > MAX_SPEED * MAX_SPEED) {
        float scale = MAX_SPEED / sqrtf(speedSquared);

        ux *= scale;
        uy *= scale;
    }

    // --------------------------------------------------------
    // BGK collision
    // --------------------------------------------------------

    for (int q = 0; q < Q; ++q) {
        float feq = equilibrium(q, rho, ux, uy);

        fPost[q * totalCells + cell] = localF[q] + OMEGA * (feq - localF[q]);
    }
}

// ------------------------------------------------------------
// Pull streaming
// ------------------------------------------------------------
//
// For each destination cell, pull the required distribution
// from the corresponding source cell.
//
// This is preferable to the previous push implementation here
// because obstacle boundaries cannot leave stale values in d_fNext.
//
// If the source is outside the domain or is a wall:
//     bounce back using the opposite direction.
// ------------------------------------------------------------

__global__ void lbmStreaming(const float *fPost, float *fNext, const unsigned char *obstacle, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int cell       = y * width + x;
    int totalCells = width * height;

    // Obstacle state is bounced back locally.
    if (obstacle[cell]) {
        for (int q = 0; q < Q; ++q) {
            fNext[q * totalCells + cell] = fPost[OPPOSITE[q] * totalCells + cell];
        }
        return;
    }

    for (int q = 0; q < Q; ++q) {
        // Pull from the cell that would have streamed into (x, y).
        int sourceX = x - CX[q];
        int sourceY = y - CY[q];

        // Solid outer boundary.
        if (sourceX < 0 || sourceX >= width || sourceY < 0 || sourceY >= height) {

            fNext[q * totalCells + cell] = fPost[OPPOSITE[q] * totalCells + cell];

            continue;
        }

        int sourceCell = sourceY * width + sourceX;

        // Solid obstacle boundary.
        if (obstacle[sourceCell]) {
            fNext[q * totalCells + cell] = fPost[OPPOSITE[q] * totalCells + cell];

            continue;
        }

        // Normal streaming.
        fNext[q * totalCells + cell] = fPost[q * totalCells + sourceCell];
    }
}

// ------------------------------------------------------------
// Velocity -> blue/cyan/green/yellow/red
// ------------------------------------------------------------

__device__ uint32_t velocityToColor(float speed)
{
    float t = fminf(speed / VISUALIZATION_SPEED, 1.0f);
    t       = fmaxf(t, 0.0f);

    float r;
    float g;
    float b;

    // Piecewise approximation of a CFD-style
    // blue -> cyan -> green -> yellow -> red map.

    if (t < 0.25f) {
        float u = t / 0.25f;

        // Blue -> Cyan
        r = 0.0f;
        g = u;
        b = 1.0f;
    }
    else if (t < 0.50f) {
        float u = (t - 0.25f) / 0.25f;

        // Cyan -> Green
        r = 0.0f;
        g = 1.0f;
        b = 1.0f - u;
    }
    else if (t < 0.75f) {
        float u = (t - 0.50f) / 0.25f;

        // Green -> Yellow
        r = u;
        g = 1.0f;
        b = 0.0f;
    }
    else {
        float u = (t - 0.75f) / 0.25f;

        // Yellow -> Red
        r = 1.0f;
        g = 1.0f - u;
        b = 0.0f;
    }

    unsigned char red = static_cast<unsigned char>(255.0f * r);

    unsigned char green = static_cast<unsigned char>(255.0f * g);

    unsigned char blue = static_cast<unsigned char>(255.0f * b);

    // Same 0xAARRGGBB packing convention used by the PBO
    // rendering path in the existing simulations.
    return (0xFFu << 24) | (static_cast<uint32_t>(blue) << 16) | (static_cast<uint32_t>(green) << 8)
           | static_cast<uint32_t>(red);
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
//
// The displayed scalar is VELOCITY MAGNITUDE, not pressure.
//
// For isothermal D2Q9:
//
//     p = rho * cs^2
//     cs^2 = 1/3
//
// A pressure visualization can be added later. Velocity is used
// here because it gives a much clearer interactive flow field.
// ------------------------------------------------------------

__global__ void renderLBM(uint32_t            *pixels,
                          const float         *f,
                          const unsigned char *obstacle,
                          int                  screenWidth,
                          int                  screenHeight,
                          int                  renderCellSize,
                          int                  width,
                          int                  height)
{
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;

    if (px >= screenWidth || py >= screenHeight) return;

    int cellX = px / renderCellSize;
    int cellY = py / renderCellSize;

    if (cellX >= width || cellY >= height) return;

    int cell       = cellY * width + cellX;
    int totalCells = width * height;

    // Walls are dark gray.
    if (obstacle[cell]) {
        pixels[py * screenWidth + px] = 0xFF303030;
        return;
    }

    float rho = 0.0f;
    float ux  = 0.0f;
    float uy  = 0.0f;

    for (int q = 0; q < Q; ++q) {
        float value = f[q * totalCells + cell];

        rho += value;
        ux += value * static_cast<float>(CX[q]);
        uy += value * static_cast<float>(CY[q]);
    }

    if (rho > 1.0e-6f) {
        ux /= rho;
        uy /= rho;
    }

    float speed = sqrtf(ux * ux + uy * uy);

    pixels[py * screenWidth + px] = velocityToColor(speed);
}

}  // namespace

// ============================================================
// Host API
// ============================================================

void initLBMSimulation(int screenWidth, int screenHeight, int requestedCellSize)
{
    cleanupLBMSimulation();

    cellSize = std::max(1, requestedCellSize);

    gridWidth  = screenWidth / cellSize;
    gridHeight = screenHeight / cellSize;

    if (gridWidth <= 0 || gridHeight <= 0) return;

    int totalCells = gridWidth * gridHeight;

    size_t distributionSize = static_cast<size_t>(Q) * static_cast<size_t>(totalCells) * sizeof(float);

    size_t obstacleSize = static_cast<size_t>(totalCells) * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc(&d_f, distributionSize));
    CUDA_CHECK(cudaMalloc(&d_fPost, distributionSize));
    CUDA_CHECK(cudaMalloc(&d_fNext, distributionSize));

    CUDA_CHECK(cudaMalloc(&d_obstacle, obstacleSize));

    dim3 block(16, 16);

    dim3 grid((gridWidth + block.x - 1) / block.x, (gridHeight + block.y - 1) / block.y);

    initializeLBM<<<grid, block>>>(d_f, d_fPost, d_fNext, d_obstacle, gridWidth, gridHeight);

    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

// ============================================================

void runLBMSimulationStep(uint32_t *devPtr, int screenWidth, int screenHeight, float time, const LBMMouseInput &mouse)
{
    (void)time;

    if (d_f == nullptr || d_fPost == nullptr || d_fNext == nullptr || d_obstacle == nullptr) {

        initLBMSimulation(screenWidth, screenHeight, 4);
    }

    if (d_f == nullptr) return;

    dim3 block(16, 16);

    dim3 grid((gridWidth + block.x - 1) / block.x, (gridHeight + block.y - 1) / block.y);

    // --------------------------------------------------------
    // Convert GLFW screen coordinates to lattice coordinates.
    //
    // IMPORTANT:
    // There is deliberately NO Y flip here.
    //
    // GLFW:
    //     top-left = (0, 0)
    //
    // LBM:
    //     top-left = (0, 0)
    //
    // This keeps mouse interaction and the rendered lattice
    // visually aligned.
    // --------------------------------------------------------

    float latticeX = mouse.x / static_cast<float>(cellSize);

    float latticeY = mouse.y / static_cast<float>(cellSize);

    float previousLatticeX = mouse.previousX / static_cast<float>(cellSize);

    float previousLatticeY = mouse.previousY / static_cast<float>(cellSize);

    // Clamp the cursor to the lattice.
    latticeX = std::max(0.0f, std::min(latticeX, static_cast<float>(gridWidth - 1)));

    latticeY = std::max(0.0f, std::min(latticeY, static_cast<float>(gridHeight - 1)));

    previousLatticeX = std::max(0.0f, std::min(previousLatticeX, static_cast<float>(gridWidth - 1)));

    previousLatticeY = std::max(0.0f, std::min(previousLatticeY, static_cast<float>(gridHeight - 1)));

    // --------------------------------------------------------
    // 1. Update walls
    // --------------------------------------------------------

    if (mouse.right || mouse.middle) {
        updateObstacles<<<grid, block>>>(d_obstacle, gridWidth, gridHeight, latticeX, latticeY, previousLatticeX,
                                         previousLatticeY, mouse.right, mouse.middle);

        CUDA_CHECK(cudaGetLastError());
    }

    // --------------------------------------------------------
    // 2. Collision
    // --------------------------------------------------------

    lbmCollision<<<grid, block>>>(d_f, d_fPost, d_obstacle, gridWidth, gridHeight, latticeX, latticeY, previousLatticeX,
                                  previousLatticeY, mouse.left);

    CUDA_CHECK(cudaGetLastError());

    // --------------------------------------------------------
    // 3. Pull streaming
    // --------------------------------------------------------

    lbmStreaming<<<grid, block>>>(d_fPost, d_fNext, d_obstacle, gridWidth, gridHeight);

    CUDA_CHECK(cudaGetLastError());

    // d_fNext is now the state for the next timestep.
    std::swap(d_f, d_fNext);

    // --------------------------------------------------------
    // 4. Render to the CUDA/OpenGL PBO
    // --------------------------------------------------------

    dim3 renderGrid((screenWidth + block.x - 1) / block.x, (screenHeight + block.y - 1) / block.y);

    renderLBM<<<renderGrid, block>>>(devPtr, d_f, d_obstacle, screenWidth, screenHeight, cellSize, gridWidth,
                                     gridHeight);

    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

// ============================================================

void cleanupLBMSimulation()
{
    if (d_f) cudaFree(d_f);

    if (d_fPost) cudaFree(d_fPost);

    if (d_fNext) cudaFree(d_fNext);

    if (d_obstacle) cudaFree(d_obstacle);

    d_f        = nullptr;
    d_fPost    = nullptr;
    d_fNext    = nullptr;
    d_obstacle = nullptr;

    gridWidth  = 0;
    gridHeight = 0;
}
