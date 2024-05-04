#include <iostream>
#include <fstream>
#include "windows.h"
using namespace std;

#define IMAGE_SIZE 1024
#define DEGREES_PER_PIXEL 0.000208

void lineTo(char bits[IMAGE_SIZE][IMAGE_SIZE], int x0, int y0, int x1, int y1);

static char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };


int main(int argc, char* argv[]) {
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;

    char colorTable[1024];

    int i, j, k, l;
    char* fontPtr;
    char* bmpPtr;
    ofstream bmpOut("foo.bmp", ios::out | ios::binary);

    if (!bmpOut) {
        cout << "...could not open file, ending.";
        return -1;
    }
    bmfh.bfType = 0x4d42;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
    bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);

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

    ifstream inputFile("DCBoundaryFile.txt");
    if (!inputFile) {
        cout << "Error: Unable to open input file." << endl;
        return -1;
    }

    double firstLongitude, firstLatitude;
    inputFile >> firstLongitude >> firstLatitude;

    int xPrev = static_cast<int>((firstLongitude - (-76.909395)) / DEGREES_PER_PIXEL);
    int yPrev = static_cast<int>((38.995110 - firstLatitude) / DEGREES_PER_PIXEL);

    double longitude, latitude;
    while (inputFile >> longitude >> latitude) {
        if (longitude < -77.119900 || longitude > -76.909395 || latitude < 38.791513 || latitude > 38.995110) {
            cout << "Error: Latitude or longitude out of range." << endl;
            return -1;
        }

        int x = static_cast<int>((longitude - (-76.909395)) / DEGREES_PER_PIXEL);
        int y = static_cast<int>((38.995110 - latitude) / DEGREES_PER_PIXEL);

        lineTo(bits, xPrev, yPrev, x, y);

        xPrev = x;
        yPrev = y;
    }

    for (i = 0; i < 256; i++) {
        j = i * 4;
        colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
    }

    // Write out to the bitmap
    char* workPtr;
    workPtr = (char*)&bmfh;
    bmpOut.write(workPtr, 14);
    workPtr = (char*)&bmih;
    bmpOut.write(workPtr, 40);
    workPtr = &colorTable[0];
    bmpOut.write(workPtr, sizeof(colorTable));
    workPtr = &bits[0][0];
    bmpOut.write(workPtr, IMAGE_SIZE * IMAGE_SIZE);

    bmpOut.close();

    system("mspaint foo.bmp");
    return 0;
}

/*Convert lineTo into inline assembly*/
void lineTo(char bits[IMAGE_SIZE][IMAGE_SIZE], int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        bits[y0][x0] = 150;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}