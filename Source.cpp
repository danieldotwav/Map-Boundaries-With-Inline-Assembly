/*
Name: Daniel Rivas
Course: CS230
Lab 3 - Map Boundaries

*/

#include <iostream>
#include <fstream>
#include <cmath>
#include "windows.h"
using namespace std;

#define IMAGE_SIZE 1024 //changed from 256
const double MAX_LATITUDE = 38.995110;
const double MIN_LONGITUDE = -77.119900;
const double MIN_LATITUDE = 38.791513;
const double MAX_LONGITUDE = -76.909395;
const double DEGREES_PER_PIXEL = 0.00019883;

// Calculate x and y coordinates from geographical coordinates
int longitudeToImageX(double longitude) {
    return static_cast<int>((longitude - MIN_LONGITUDE) / (MAX_LONGITUDE - MIN_LONGITUDE) * IMAGE_SIZE);
}

int latitudeToImageY(double latitude) {
    return static_cast<int>((MAX_LATITUDE - latitude) / (MAX_LATITUDE - MIN_LATITUDE) * IMAGE_SIZE);
}

bool isPointInRange(double longitude, double latitude) {
    return(longitude >= MIN_LONGITUDE && longitude <= MAX_LONGITUDE &&
        latitude >= MIN_LATITUDE && latitude <= MAX_LATITUDE);
}

void drawLine(int x1, int y1, int x2, int y2, char bits[IMAGE_SIZE][IMAGE_SIZE]) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (true) {
        if (x1 >= 0 && x1 < IMAGE_SIZE && y1 >= 0 && y1 < IMAGE_SIZE) {
            // Reflect across the y-axis
            bits[x1][IMAGE_SIZE - 1 - y1] = 255;
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}


int main(int argc, char* argv[]) {
    ifstream inputFile("DCBoundaryFile.txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }

    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    char colorTable[1024];
    char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };

    // Define and open the output file
    ofstream bmpOut("gradient_with_line.bmp", ios::out + ios::binary);
    if (!bmpOut) {
        cout << "Could not open file, ending.";
        return -1;
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
    bmih.biXPelsPerMeter = 2835; // magic number, see Wikipedia entry
    bmih.biYPelsPerMeter = 2835;
    bmih.biClrUsed = 256;
    bmih.biClrImportant = 0;

    // Build color table
    for (int i = 0; i < 256; i++) {
        int j = i * 4;
        colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
    }

    // Build grayscale array of bits in image, with gradient
    for (int i = 0; i < IMAGE_SIZE; i++) {
        for (int j = 0; j < IMAGE_SIZE; j++) {
            bits[i][j] = 0; //black table
        }
    }

    double prevLongitude, prevLatitude;
    inputFile >> prevLongitude >> prevLatitude;

    // Convert geographical coordinates to image coordinates
    int x1 = static_cast<int>(longitudeToImageX(prevLongitude));
    int y1 = static_cast<int>(latitudeToImageY(prevLatitude));
    double longitude, latitude;

    // Loop to draw lines from the previous to the current points
    while (inputFile >> longitude >> latitude) {
        if (isPointInRange(longitude, latitude)) {
            int x2 = static_cast<int>(longitudeToImageX(longitude));
            int y2 = static_cast<int>(latitudeToImageY(latitude));

            // Draw a line between the two points
            drawLine(x1, y1, x2, y2, bits);

            // Update to the next point
            x1 = x2;
            y1 = y2;
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

    // Now let's look at our creation
    system("mspaint gradient_with_line.bmp");

    return 0;
}

