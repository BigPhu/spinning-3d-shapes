#include <iostream>
#include <string>
#include <math.h>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

const float PI = 3.14;
// ========================  SETTINGS ========================
// Pyramid settings
const float PYRAMID_HEIGHT = 2.5;
const float BASE_WIDTH = 4;
const float HORIZONTAL_SCALE = 50;
const float VERTICAL_SCALE = 20;
const bool APPLY_LIGHTING = true;
const char* GRADIENT = ".,-~:;=!*#$@";

// Environment settings 
int SCREEN_WIDTH = 80;
int SCREEN_HEIGHT = 22;
char BACKGROUND = ' ';
const char* COLOR = "Color  0A";    // https://www.geeksforgeeks.org/how-to-print-COLORed-text-in-c/
const float DISTANCE_FROM_CAM = 8;
const float LIGHT_X = 0;
const float LIGHT_Y = 0;
const float LIGHT_Z = -1;
const unsigned int LIGHT_INTENSITY = 12;

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
    float y = -calcY(baseX, baseY, cubeZ);
    float z = calcZ(baseX, baseY, cubeZ);
    
    float ooz = 1/(z + DISTANCE_FROM_CAM);

    // Project donut to 2D screen
    int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz));
    int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz));
    int idx = xp + yp*SCREEN_WIDTH;
                
    if (idx >= 0 && idx < screenArea) {
        // Calculate lighting
        float normalX = calcX(normalVec[0], normalVec[1], normalVec[2]);
        float normalY = -calcY(normalVec[0], normalVec[1], normalVec[2]);
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
    float y = -calcY(baseX, baseY, cubeZ);
    float z = calcZ(baseX, baseY, cubeZ);
    
    float ooz = 1/(z + DISTANCE_FROM_CAM);

    // Project donut to 2D screen
    int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz));
    int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz));
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
    float denominator = BASE_WIDTH*BASE_WIDTH + 4*PYRAMID_HEIGHT*PYRAMID_HEIGHT;
    float normX = (2*PYRAMID_HEIGHT*PYRAMID_HEIGHT*BASE_WIDTH)/(denominator);
    float normY = (BASE_WIDTH*BASE_WIDTH*PYRAMID_HEIGHT)/(denominator) - PYRAMID_HEIGHT/2;

    float baseNormalVec[3] = {0, -1, 0};
    float frontNormalVec[3] = {0, normY, -normX};
    float backNormalVec[3] = {0, normY, normX};
    float rightNormalVec[3] = {normX, normY, 0};
    float leftNormalVec[3] = {-normX, normY, 0};

    normalize(frontNormalVec);
    normalize(backNormalVec);
    normalize(rightNormalVec);
    normalize(leftNormalVec);

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
        
        // Render pyramid base
        for (float baseX = -BASE_WIDTH/2; baseX < BASE_WIDTH/2; baseX += 0.07) {
            for (float baseY = -BASE_WIDTH/2; baseY < BASE_WIDTH/2; baseY += 0.07) {
                if (APPLY_LIGHTING) {
                    renderFace(baseX, -PYRAMID_HEIGHT/2, baseY, baseNormalVec);      
                }
                else {
                    renderFace(baseX, -PYRAMID_HEIGHT/2, baseY, '+'); 
                }
            }
        }
        // Render pyramid side faces
        for (float sideY = -PYRAMID_HEIGHT/2; sideY < PYRAMID_HEIGHT/2; sideY += 0.07) {
            float sideWidth = -(BASE_WIDTH/(2*PYRAMID_HEIGHT))*(sideY - PYRAMID_HEIGHT/2);
            float sideZ = sideWidth;

            for (float sideX = -sideWidth; sideX < sideWidth; sideX += 0.07) {
                if (APPLY_LIGHTING) {
                    renderFace(sideX, sideY, -sideZ, frontNormalVec);      // Front
                    renderFace(sideX, sideY, sideZ, backNormalVec);        // Back

                    renderFace(sideZ, sideY, sideX, rightNormalVec);       // Right side
                    renderFace(-sideZ, sideY, sideX, leftNormalVec);       // Left side
                }
                else {
                    renderFace(sideX, sideY, -sideZ, '@');     // Front
                    renderFace(sideX, sideY, sideZ, '#');      // Back

                    renderFace(sideZ, sideY, sideX, ';');      // Right side
                    renderFace(-sideZ, sideY, sideX, '~');     // Left side
                }
            }
        }
        
        // Render pyramid
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