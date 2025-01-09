#include <iostream>
#include <string>
#include <math.h>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

const float PI = 3.14;
// ========================  SETTINGS ========================
// Donut settings
float THICKNESS = 1.5;
float HOLE_SIZE = 3;    // The younger the soul, the tighter the hole
float HORIZONTAL_SCALE = 30;
float VERTICAL_SCALE = 15;

// Display settings 
int SCREEN_WIDTH = 80;
int SCREEN_HEIGHT = 22;
char BACKGROUND = ' ';
const char* COLOR = "Color  03";    // https://www.geeksforgeeks.org/how-to-print-COLORed-text-in-c/
const float DISTANCE_FROM_CAM = 8;

// Animation settings
float INCREMENT_ANGLE = 0.04;
int ANIMATION_DELAY = 10;      // In miliseconds
// ============================================================

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
    
    int screenArea = SCREEN_WIDTH*SCREEN_HEIGHT;
    // Initialize output frame
    char* output = new char[screenArea];
    // Initialize z-buffer
    float* zBuffer = new float[screenArea];

    system(COLOR);

    float A = 0;
    float B = 0;

    // Main render loop
    while (true) {
        float sinA = sin(A), cosA = cos(A);
        float sinB = sin(B), cosB = cos(B);

        // Reset output & z-buffer
        for (int i = 0; i < screenArea; i++) {
            output[i] = BACKGROUND;
            zBuffer[i] = 0;
        }

        // Render donut
        for (float theta = 0; theta < 2*PI; theta += 0.07) {
            float sinTheta = sin(theta), cosTheta = cos(theta);

            for (float phi = 0; phi < 2*PI; phi += 0.07) {
                float sinPhi = sin(phi), cosPhi = cos(phi);

                float circleX = (HOLE_SIZE + THICKNESS*cosTheta);
                float circleY = THICKNESS*sinTheta;

                // Model donut in 3D space
                float x = circleX*(cosB*cosPhi + sinA*sinB*sinPhi) - circleY*cosA*sinB;
                float y = circleX*(cosPhi*sinB - cosB*sinA*sinPhi) + circleY*cosA*cosB;
                float z = DISTANCE_FROM_CAM + cosA*circleX*sinPhi + circleY*sinA;
                float ooz = 1/z;

                // Project donut to 2D screen
                int xp = (int) ((SCREEN_WIDTH/2) + (HORIZONTAL_SCALE*x*ooz));
                int yp = (int) ((SCREEN_HEIGHT/2) + (VERTICAL_SCALE*y*ooz));
                int idx = xp + yp*SCREEN_WIDTH;
                
                if (idx >= 0 && idx < screenArea) {
                    // Add lighting
                    float L = cosPhi*cosTheta*sinB - cosA*cosTheta*sinPhi - sinA*sinTheta + cosB*(cosA*sinTheta - cosTheta*sinA*sinPhi);
                    
                    if (ooz > zBuffer[idx] && L > 0) {
                        zBuffer[idx] = ooz;
                        int lightLevel = L*8;

                        output[idx] = ".,-~:;=!*#$@"[lightLevel];
                    }
                }
            }
        }

        // Render donut
        cout << "\x1b[H";
        for (int i = 0; i < screenArea; i++) {
            cout << ((i % SCREEN_WIDTH) ? output[i] : '\n');
        }

        A += INCREMENT_ANGLE;
        B += INCREMENT_ANGLE;

        // Control frame rate
        std::chrono::milliseconds timespan(ANIMATION_DELAY); // or whatever
        std::this_thread::sleep_for(timespan);
    }

    return 0;
}