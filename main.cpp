#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>
#include <string>
#include "picosha2.h"

const int WIDTH = 1000, HEIGHT = 1000;
const int NUM_PIXELS = WIDTH * HEIGHT;
const int NUM_ROWS = 500, NUM_COLS = 500;
const int NUM_CELLS = NUM_ROWS * NUM_COLS;
int cellData[NUM_CELLS];
sf::Uint8 pixelData[NUM_PIXELS * 4]; // RGBA

inline int pixelToIdx(int pixel)
{
    int px = pixel % WIDTH;
    int py = pixel / WIDTH;
    int c = px * NUM_COLS / WIDTH;
    int r = py * NUM_ROWS / HEIGHT;
    return r * NUM_COLS + c;
}

void updatePixels()
{
    for (int i = 0; i < NUM_PIXELS; i++) {
        int cellIdx = pixelToIdx(i);
        int dataIdx = i * 4;
        auto data = (sf::Uint32*)(&pixelData[dataIdx]);
        if (cellData[cellIdx] == 1) {
            *data = 0xFFFFFFFF; // Live cells are white
        }
        else {
            *data = 0; // Dead cells are black
        }
    }
}

void initCells()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> dist(0, 20);
    for (auto& cell : cellData) {
        cell = (dist(eng) == 0) ? 1 : 0;
    }
    updatePixels();
}

void countNeighbors(int* neighborCount)
{
    for (int r = 0; r < NUM_ROWS; r++) {
        for (int c = 0; c < NUM_COLS; c++) {
            int idx = r * NUM_COLS + c;
            int numNeighbors = 0;
            for (int i = r - 1; i <= r + 1; i++) {
                for (int j = c - 1; j <= c + 1; j++) {
                    if (i < 0 || i >= NUM_ROWS) continue;
                    if (j < 0 || j >= NUM_COLS) continue;
                    if (i == r && j == c) continue;
                    int adjIdx = i * NUM_COLS + j;
                    if (cellData[adjIdx] == 1) {
                        numNeighbors++;
                    }
                }
            }
            neighborCount[idx] = numNeighbors;
        }
    }
}

void updateCells()
{
    int neighborCount[NUM_CELLS];
    countNeighbors(neighborCount);
    for (int i = 0; i < NUM_CELLS; i++) {
        int numNeighbors = neighborCount[i];
        if (cellData[i] && numNeighbors < 2) {
            cellData[i] = 0;
        }
        else if (cellData[i] && numNeighbors > 3) {
            cellData[i] = 0;
        }
        else if (!cellData[i] && numNeighbors == 3) {
            cellData[i] = 1;
        }
    }
    updatePixels();
}

std::string getHash()
{
    static const char* base64LUT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string base64Str;
    for (int i = 0; i < NUM_CELLS; i += 6) {
        int num = 0;
        for (int j = i; j < i + 6 && j < NUM_CELLS; j++) {
            num = (num | (int)cellData[j]) << 1;
        }
        num = num >> 1;
        base64Str.push_back(base64LUT[num]);
    }
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(base64Str.begin(), base64Str.end(), hash.begin(), hash.end());
    std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
    return hex_str;
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(WIDTH, HEIGHT),
        "Game of Life",
        sf::Style::Titlebar | sf::Style::Close
    );

    const int HASH_LIST_LEN = 512;
    int lastHashPtr = 0;
    std::string hashList[512];
    for (int i = 0; i < HASH_LIST_LEN; i++) {
        hashList[i] = "something random to start off with, it doesn't matter what";
    }
    int equilibriumCount = 0;

    initCells();
    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    texture.update(pixelData);
    sf::Sprite sprite(texture);

    bool paused = false;
    bool ended = false;

    auto tPrev = std::chrono::high_resolution_clock::now();

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
        }

        auto tNow = std::chrono::high_resolution_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::microseconds>(tNow-tPrev);
        if (dt.count()/1000.0 > 10) // Fix at 100 FPS
        {
            tPrev = tNow;

            if (!paused) {
                updateCells();
                texture.update(pixelData);
            }

            std::string hashStr = getHash();
            for (int i = 0; i < HASH_LIST_LEN; i++) {
                if (hashStr == hashList[i]) {
                    equilibriumCount++;
                }
            }
            hashList[lastHashPtr] = hashStr;
            lastHashPtr = (lastHashPtr + 1) % HASH_LIST_LEN;

            if (equilibriumCount > 50) {
                paused = true;
                if (!ended) {
                    std::cout << "Equilibrium Reached!" << std::endl;
                    ended = true;
                }
            }
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }
}