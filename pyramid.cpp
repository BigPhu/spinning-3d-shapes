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
const float PYRAMID_HEIGHT = 1.5;
const float BASE_WIDTH = 1.5;
const float HORIZONTAL_SCALE = 30;
const float VERTICAL_SCALE = 15;
const bool APPLY_LIGHTING = true;

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
const int ANIMATION_DELAY = 10;      // In miliseconds
// ============================================================

float A = 0.0;
float B = 0.0;
float C = 0.0;
float sinA, cosA;
float sinB, cosB;
float sinC, cosC;
const int screenArea = SCREEN_WIDTH*SCREEN_HEIGHT;
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

void renderFace(float baseX, float baseY, float cubeZ, char faceTexture) {
    float x = calcX(baseX, baseY, cubeZ);
    float y = calcY(baseX, baseY, cubeZ);
    float z = calcZ(baseX, baseY, cubeZ);
    
    float ooz = 1/(z + DISTANCE_FROM_CAM);

    // Project donut to 2D screen
    int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz*2));
    int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz*2));
    int idx = xp + yp*SCREEN_WIDTH;
                
    if (idx >= 0 && idx < screenArea && ooz > zBuffer[idx]) {
        // Add texture
        zBuffer[idx] = ooz;

        if (APPLY_LIGHTING) {
            float vecLength = sqrt(x*x + y*y + z*z);
            float L = (x*NORMAL_LIGHT_X + y*NORMAL_LIGHT_Y + z*NORMAL_LIGHT_Z)/vecLength;
            if (L > 0) {
                int lightLevel = min(11.0f, L*LIGHT_INTENSITY);
                output[idx] = ".,-~:;=!*#$@"[lightLevel];
            }
        }
        else {
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

void showCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    // Get current cursor information
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    
    // Set the cursor visibility to true
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

int main() {
    hideCursor();
    system(COLOR);

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
        for (float baseX = -BASE_WIDTH; baseX < BASE_WIDTH; baseX += 0.07) {
            for (float baseY = -BASE_WIDTH; baseY < BASE_WIDTH; baseY += 0.07) {
                renderFace(baseX, PYRAMID_HEIGHT, baseY, '+');      
            }
        }
        // Render pyramid side faces
        for (float sideY = -PYRAMID_HEIGHT; sideY < PYRAMID_HEIGHT; sideY += 0.07) {
            float sideWidth = -(BASE_WIDTH/(2*PYRAMID_HEIGHT))*(sideY - PYRAMID_HEIGHT);
            float sideZ = -((BASE_WIDTH/(2*PYRAMID_HEIGHT))*sideY) + (BASE_WIDTH/2);

            for (float sideX = -sideWidth; sideX < sideWidth; sideX += 0.07) {
                renderFace(sideX, -sideY, -sideZ, '@');     // Front
                renderFace(sideX, -sideY, sideZ, '#');      // Back

                renderFace(sideZ, -sideY, sideX, ';');      // Right side
                renderFace(-sideZ, -sideY, sideX, '~');     // Left side
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