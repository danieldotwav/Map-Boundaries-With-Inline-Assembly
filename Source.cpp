/*
Name: Daniel Rivas
Course: CS230
Assignment: Lab 3 - Map Boundaries
Date Modified: 05/07/2024
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "windows.h"
using namespace std;

#define IMAGE_SIZE 1024
const double MAX_LATITUDE = 38.995110;
const double MIN_LONGITUDE = -77.119900;
const double MIN_LATITUDE = 38.791513;
const double MAX_LONGITUDE = -76.909395;
const double DEGREES_PER_PIXEL = 0.00019883;

const double scaleLat = IMAGE_SIZE / (MAX_LATITUDE - MIN_LATITUDE);
const double scaleLong = IMAGE_SIZE / (MAX_LONGITUDE - MIN_LONGITUDE);

// Calculate x and y coordinates from geographical coordinates
int longitudeToImageX(double longitude) {
    return ((longitude - MIN_LONGITUDE) / (MAX_LONGITUDE - MIN_LONGITUDE)) * IMAGE_SIZE;
}

int latitudeToImageY(double latitude) {
    return IMAGE_SIZE - 1 - static_cast<int>((latitude - MIN_LATITUDE) / (MAX_LATITUDE - MIN_LATITUDE) * IMAGE_SIZE);
}

bool coordinateIsInRange(double longitude, double latitude) {
    return (longitude >= MIN_LONGITUDE && longitude <= MAX_LONGITUDE &&
        latitude >= MIN_LATITUDE && latitude <= MAX_LATITUDE);
}

// Bresenham's Line Algorithm
void lineTo(int x1, int y1, int x2, int y2, char bits[IMAGE_SIZE][IMAGE_SIZE]) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (true) {
        if (x1 >= 0 && x1 < IMAGE_SIZE && y1 >= 0 && y1 < IMAGE_SIZE) {
            bits[y1][x1] = 255;
        }

        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}


int main(int argc, char* argv[]) {
    // Open the input file containing boundary data
    ifstream inputFile("DCBoundaryFile.txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open input file. Terminating program..." << endl;
        return 1;
    }

    // Bitmap file and info headers
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    char colorTable[1024]; // Color table for an 8-bit grayscale image
    char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };

    // Open the output bitmap file for binary writing
    ofstream bmpOut("boundary_img.bmp", ios::out + ios::binary);
    if (!bmpOut) {
        cout << "Error: Unable to open file";
        return 1;
    }

    // Initialize the bitmap file header with static values
    bmfh.bfType = 0x4d42;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
    bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);

    // Initialize the bitmap information header with static values
    bmih.biSize = 40;
    bmih.biWidth = IMAGE_SIZE;
    bmih.biHeight = IMAGE_SIZE;
    bmih.biPlanes = 1;
    bmih.biBitCount = 8;
    bmih.biCompression = 0;
    bmih.biSizeImage = IMAGE_SIZE * IMAGE_SIZE;
    bmih.biXPelsPerMeter = 2835;
    bmih.biYPelsPerMeter = 2835;
    bmih.biClrUsed = 256;
    bmih.biClrImportant = 0;

    // Initialize the greyscale color table
    for (int i = 0; i < 256; i++) {
        int j = i * 4;
        // Fill each color with the same grayscale value (R, G, B, reserved)
        colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
    }

    // Clear the bitmap to all black
    for (int i = 0; i < IMAGE_SIZE; i++) {
        for (int j = 0; j < IMAGE_SIZE; j++) {
            bits[i][j] = 0;
        }
    }

    // Discard first line (header) in the input file
    string line;
    getline(inputFile, line);

    // Read the first pair of geographical coordinates
    double prevLongitude, prevLatitude;
    inputFile >> prevLongitude >> prevLatitude;

    // Convert the first geographical coordinates to image coordinates
    double longitude, latitude;
    double x1, y1;

    // Inline assembly for converting the first geographical coordinates to image coordinates
    _asm {
        // Calculate the pixel y-coordinate for the first point
        movsd   xmm0, qword ptr[prevLatitude]; // Load prevLatitude
        subsd   xmm0, qword ptr[MIN_LATITUDE]; // Subtract MIN_LATITUDE
        mulsd   xmm0, qword ptr[scaleLat];     // Multiply by scaleLat
        movsd   qword ptr[y1], xmm0;           // Store y1

        // Calculate the pixel x-coordinate for the first point
        movsd   xmm0, qword ptr[prevLongitude]; // Load prevLongitude
        subsd   xmm0, qword ptr[MIN_LONGITUDE]; // Subtract MIN_LONGITUDE
        mulsd   xmm0, qword ptr[scaleLong];     // Multiply by scaleLong
        movsd   qword ptr[x1], xmm0;            // Store x1
    }

    // Loop to draw lines from the previous to the current points
    while (inputFile >> longitude >> latitude) {
        if (coordinateIsInRange(longitude, latitude)) {
            // Rinse and repeat
            double x2, y2;

            _asm {
                // Calculate the pixel y-coordinate for the next point
                movsd   xmm0, qword ptr[latitude]; // Load latitude
                subsd   xmm0, qword ptr[MIN_LATITUDE]; // Subtract MIN_LATITUDE
                mulsd   xmm0, qword ptr[scaleLat];     // Multiply by scaleLat
                movsd   qword ptr[y2], xmm0;           // Store y2

                // Calculate the pixel x-coordinate for the next point
                movsd   xmm0, qword ptr[longitude]; // Load longitude
                subsd   xmm0, qword ptr[MIN_LONGITUDE]; // Subtract MIN_LONGITUDE
                mulsd   xmm0, qword ptr[scaleLong];     // Multiply by scaleLong
                movsd   qword ptr[x2], xmm0;            // Store x2
            }

            // Draw a line between the two points on the bitmap
            lineTo(x1, y1, x2, y2, bits);

            // Update previous coordinates with new coordinates
            _asm {
                movsd   xmm0, qword ptr[x2]; // Load x2
                movsd   qword ptr[x1], xmm0; // Update x1 with x2

                movsd   xmm0, qword ptr[y2]; // Load y2
                movsd   qword ptr[y1], xmm0; // Update y1 with y2
            }
        }
    }

    inputFile.close();

    // Write out to the bitmap
    char* workPtr;
    workPtr = reinterpret_cast<char*>(&bmfh);
    bmpOut.write(workPtr, 14);
    workPtr = reinterpret_cast<char*>(&bmih);
    bmpOut.write(workPtr, 40);
    workPtr = &colorTable[0];
    bmpOut.write(workPtr, sizeof(colorTable));
    workPtr = &bits[0][0];
    bmpOut.write(workPtr, IMAGE_SIZE * IMAGE_SIZE);
    bmpOut.close();

    // Pray there is a God
    system("mspaint boundary_img.bmp");

    return 0;
}

