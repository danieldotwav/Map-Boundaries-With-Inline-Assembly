#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

const int WIDTH = 1024;
const int HEIGHT = 1024;
int bitmap[WIDTH][HEIGHT] = { 0 };

// Function to draw a line segment from (x1, y1) to (x2, y2)
void lineTo(int x1, int y1, int x2, int y2) {
    // Inline assembly for drawing a line using Bresenham's algorithm
    int* pixel_address;

    __asm {
        // Store the starting coordinates
        mov eax, x1
        mov ebx, y1
        mov ecx, x2
        mov edx, y2
        // Calculate the change in x and y
        sub ecx, eax // dx = x2 - x1
        sub edx, ebx // dy = y2 - y1
        // Initialize decision parameter
        mov esi, 0     // decision parameter (P)
        // Calculate decision parameter for slope <= 1
        cmp ecx, edx  // if |dx| >= |dy|
        jge L1         // then slope <= 1
        mov esi, 1     // decision parameter (P) for slope > 1
        // Swap x1, y1 and x2, y2 if necessary
        mov eax, x1
        mov ebx, y1
        mov ecx, x2
        mov edx, y2
        // Calculate the change in x and y after swapping
        sub ecx, eax // dx = x2 - x1
        sub edx, ebx // dy = y2 - y1
        L1 :
        // Initialize coordinates and increment direction

        shl edx, 1     // 2 * edx -> equivalent to multiplying by 2
            mov esi, edx
            mov edi, 0
            mov eax, x1
            mov ebx, y1
            mov ecx, x2
            mov edx, y2

            // Draw the starting pixel
            mov eax, ebx          // eax = y1
            imul eax, WIDTH       // eax = y1 * WIDTH
            add eax, x1           // eax = y1 * WIDTH + x1
            add eax, bitmap       // Add the base address of bitmap
            mov pixel_address, eax // Store the address into the pixel_address variable

            mov edi, pixel_address // Load the address into edi
            mov byte ptr[edi], 1   // Set the pixel value to 1 (or any desired value)

            mov eax, 1
            mov ebx, 1
            mov ecx, esi

            shl ecx, 1     // 2 * ecx
            mov esi, ecx

            shl edx, 1     // 2 * edx
            mov edi, edx

            L2 :
        // Draw the line segment
        add eax, ebx
            cmp eax, esi
            jl L3
            mov eax, 0
            add ebx, 2
            L3:
        add edx, ebx
            cmp edx, edi
            jl L4
            mov edx, 0
            add ebx, 2
            L4 :
            add eax, ecx
            add edx, edi
            add ebx, 2

            // Calculate the next pixel address using C++
            mov eax, ebx          // eax = y1
            imul eax, WIDTH       // eax = y1 * WIDTH
            add eax, x1           // eax = y1 * WIDTH + x1
            add eax, bitmap       // Add the base address of bitmap
            mov pixel_address, eax // Store the address into the pixel_address variable

            mov edi, pixel_address // Load the address into edi
            mov byte ptr[edi], 1   // Set the next pixel value to 1

            // Check if the end point is reached
            mov eax, x1
            mov ebx, y1
            mov ecx, x2
            mov edx, y2
            cmp eax, ecx
            jne L5
            cmp ebx, edx
            je L6
            L5 :
        // Update coordinates
        inc eax // Increment x
            mov ebx, y1
            mov edx, y2
            sub edx, ebx // Calculate dy
            jge L7
            dec ebx // Decrement y
            L7 :
        mov[x1], eax // Store the new x coordinate
            mov[y1], ebx // Store the new y coordinate
            jmp L2 // Continue drawing
            L6 :
    }
}

int main() {
    // Open input file
    ifstream inputFile("DCBoundaryFile.txt");
    if (!inputFile) {
        cerr << "Error opening file." << endl;
        return 1;
    }
    
    // Read and process input
    double latitude, longitude;
    inputFile >> latitude >> longitude;
    double firstLatitude = latitude;
    double firstLongitude = longitude;

    // Check if first latitude and longitude are within range
    if (latitude < 38.791513 || latitude > 38.995110) {
        cout << "Value " << latitude << " out of range, ending." << endl;
        return 3;
    }
    if (longitude < -77.119900 || longitude > -76.909395) {
        cout << "Value " << longitude << " out of range, ending." << endl;
        return 3;
    }

    // Loop through input file
    while (inputFile >> latitude >> longitude) {
        // Check if latitude and longitude are within range
        if (latitude < 38.791513 || latitude > 38.995110) {
            cout << "Value " << latitude << " out of range, ending." << endl;
            return 3;
        }
        if (longitude < -77.119900 || longitude > -76.909395) {
            cout << "Value " << longitude << " out of range, ending." << endl;
            return 3;
        }

        // Calculate pixel positions
        int x1 = static_cast<int>((longitude - (-76.909395)) / 0.00019883);
        int y1 = static_cast<int>((latitude - 38.791513) / 0.00019883);
        int x2, y2;

        // Read next point
        inputFile >> latitude >> longitude;

        // Calculate pixel positions for next point
        x2 = static_cast<int>((longitude - (-76.909395)) / 0.00019883);
        y2 = static_cast<int>((latitude - 38.791513) / 0.00019883);

        // Draw line segment
        lineTo(x1, y1, x2, y2);
    }

    // Compare last point to first point
    if (latitude == firstLatitude && longitude == firstLongitude) {
        // Write output bitmap
        // Invoke Paint to display bitmap
        cout << "Pierce College CS230 Spring 2024 Lab Assignment 3 - Your Name" << endl;
    }

    return 0;
}