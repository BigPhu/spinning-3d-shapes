#include <iostream>
#include <cstring>
#include <math.h>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

const float PI = 3.14;
// ========================  SETTINGS ========================
// Cube settings
float CUBE_WIDTH = 1.5;
float HORIZONTAL_SCALE = 30;
float VERTICAL_SCALE = 15;
const bool APPLY_LIGHTING = true;
const char* GRADIENT = ".'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

// Environment settings 
int SCREEN_WIDTH = 80;
int SCREEN_HEIGHT = 22;
char BACKGROUND = ' ';
const char* COLOR = "Color  03";    // https://www.geeksforgeeks.org/how-to-print-COLORed-text-in-c/
const float DISTANCE_FROM_CAM = 8;
const float LIGHT_X = 0;
const float LIGHT_Y = 1;
const float LIGHT_Z = -1;
const unsigned int LIGHT_INTENSITY = 70;

// Animation settings
const float INCREMENT_ANGLE = 0.04;
const int ANIMATION_DELAY = 20;      // In miliseconds
// ============================================================

// Calculate necessary variables
float A = 0.0;
float B = 0.0;
float C = 0.0;
float sinA, cosA;
float sinB, cosB;
float sinC, cosC;
const int screenArea = SCREEN_WIDTH*SCREEN_HEIGHT;
const int GRADIENT_SIZE = strlen(GRADIENT);
// Normalize light source position vector
const float LIGHT_VEC_LENGTH = sqrt(LIGHT_X*LIGHT_X + LIGHT_Y*LIGHT_Y + LIGHT_Z*LIGHT_Z);
const float NORMAL_LIGHT_X = LIGHT_X/LIGHT_VEC_LENGTH;
const float NORMAL_LIGHT_Y = -LIGHT_Y/LIGHT_VEC_LENGTH;
const float NORMAL_LIGHT_Z = LIGHT_Z/LIGHT_VEC_LENGTH;

// Initialize output frame
char* output = new char[screenArea];
// Initialize z-buffer
float* zBuffer = new float[screenArea];

float calcX(float i, float j, float k) {
    return (cosB*cosC)*i + (cosC*sinB*sinA - sinC*cosA)*j + (cosC*sinB*cosA + sinC*sinA)*k;
}

float calcY(float i, float j, float k) {
    return (cosB*sinC)*i + (sinC*sinB*sinA + cosC*cosA)*j + (sinC*sinB*cosA - cosC*sinA)*k;
}

float calcZ(float i, float j, float k) {
    return (-sinB)*i + (cosB*sinA)*j + (cosB*cosA)*k;
}

float* normalize(float vec[3]) {
    float vecLen = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);

    if (vecLen != 1) {
        vec[0] /= vecLen;
        vec[1] /= vecLen;
        vec[2] /= vecLen;
    }

    return vec;
}

void renderFace(float baseX, float baseY, float cubeZ, float normalVec[3]) {
    float x = calcX(baseX, baseY, cubeZ);
    float y = calcY(baseX, baseY, cubeZ);
    float z = calcZ(baseX, baseY, cubeZ);
    
    float ooz = 1/(z + DISTANCE_FROM_CAM);

    // Project donut to 2D screen
    int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz*2));
    int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz*2));
    int idx = xp + yp*SCREEN_WIDTH;
                
    if (idx >= 0 && idx < screenArea) {
        // Calculate lighting
        float normalX = calcX(normalVec[0], normalVec[1], normalVec[2]);
        float normalY = calcY(normalVec[0], normalVec[1], normalVec[2]);
        float normalZ = calcZ(normalVec[0], normalVec[1], normalVec[2]);
        float L = normalX*NORMAL_LIGHT_X + normalY*NORMAL_LIGHT_Y + normalZ*NORMAL_LIGHT_Z;
        
        if (L > 0) {
            if (ooz > zBuffer[idx]) {
                zBuffer[idx] = ooz;

                int lightLevel = min((float) GRADIENT_SIZE - 1, L*LIGHT_INTENSITY);
                output[idx] = GRADIENT[lightLevel];
            }
        }
    }
}

void renderFace(float baseX, float baseY, float cubeZ, char faceTexture) {
    float x = calcX(baseX, baseY, cubeZ);
    float y = calcY(baseX, baseY, cubeZ);
    float z = calcZ(baseX, baseY, cubeZ);
    
    float ooz = 1/(z + DISTANCE_FROM_CAM);

    // Project donut to 2D screen
    int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz*2));
    int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz*2));
    int idx = xp + yp*SCREEN_WIDTH;
                
    if (idx >= 0 && idx < screenArea) {

        if (ooz > zBuffer[idx]) {
            // Add texture
            zBuffer[idx] = ooz;
            output[idx] = faceTexture;
        }
    }
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    // Get current cursor information
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    
    // Set the cursor visibility to false
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

int main() {
    hideCursor();
    system(COLOR);

    // Normal vectors. NOTE: NORMAL VECTORS OF ANY FACE SHOULD BE NORMALIZED TO MAKE THE LIGHTING ACCURE
    float topNormalVec[3] = {0, -1, 0};
    float baseNormalVec[3] = {0, 1, 0};
    float frontNormalVec[3] = {0, 0, -1};
    float backNormalVec[3] = {0, 0, 1};
    float rightNormalVec[3] = {1, 0, 0};
    float leftNormalVec[3] = {-1, 0, 0};

    // Main render loop
    while (true) {
        // Reset output & z-buffer
        for (int i = 0; i < screenArea; i++) {
            output[i] = BACKGROUND;
            zBuffer[i] = 0;
        }

        sinA = sin(A), cosA = cos(A);
        sinB = sin(B), cosB = cos(B);
        sinC = sin(C), cosC = cos(C);
        
        // Render cube
        for (float cubeX = -CUBE_WIDTH; cubeX < CUBE_WIDTH; cubeX += 0.07) {
            for (float cubeY = -CUBE_WIDTH; cubeY < CUBE_WIDTH; cubeY += 0.07) {
                // Model cube in 3D (6 faces)
                if (APPLY_LIGHTING) {
                    renderFace(cubeX, -CUBE_WIDTH, cubeY, topNormalVec);     // Top 
                    renderFace(cubeX, CUBE_WIDTH, cubeY, baseNormalVec);     // Base 

                    renderFace(cubeX, cubeY, -CUBE_WIDTH, frontNormalVec);   // Front
                    renderFace(cubeX, cubeY, CUBE_WIDTH, backNormalVec);     // Back
                
                    renderFace(CUBE_WIDTH, cubeX, cubeY, rightNormalVec);    // Right side
                    renderFace(-CUBE_WIDTH, cubeX, cubeY, leftNormalVec);    // Left side
                } 
                else {
                    renderFace(cubeX, -CUBE_WIDTH, cubeY, ';');     // Top 
                    renderFace(cubeX, CUBE_WIDTH, cubeY, '+');      // Base 

                    renderFace(cubeX, cubeY, -CUBE_WIDTH, '@');     // Front
                    renderFace(cubeX, cubeY, CUBE_WIDTH, '#');      // Back
                
                    renderFace(CUBE_WIDTH, cubeX, cubeY, '$');      // Right side
                    renderFace(-CUBE_WIDTH, cubeX, cubeY, '~');     // Left side
                }
            }
        }
        
        // Render cube
        cout << "\x1b[H";
        for (int i = 0; i < screenArea; i++) {
            cout << ((i % SCREEN_WIDTH) ? output[i] : '\n');
        }
        A += INCREMENT_ANGLE;
        B += INCREMENT_ANGLE;
        C += INCREMENT_ANGLE;

        // // Control frame rate
        std::chrono::milliseconds timespan(ANIMATION_DELAY); // or whatever
        std::this_thread::sleep_for(timespan);
    }
    return 0;
}