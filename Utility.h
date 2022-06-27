//
// Created by 15711 on 2022.05.19.
//

#ifndef TEST_PY_UTILITY_H
#define TEST_PY_UTILITY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

template<typename T>
void WriteToCSV(const std::string& fileName, const T* data, int width, int height)
{
    std::ofstream file;
    int size = width * height;
    file.open(fileName, std::ios::out);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int index = i * width + j;
            assert(index < size);
            T value = data[index];
            file << std::setiosflags(std::ios::fixed) << std::setprecision(10) << value;
            if (j == width - 1)
                file << "\n";
            else
                file << ",";
        }
    }
    file << std::endl;
    file.close();
}

#endif //TEST_PY_UTILITY_H
