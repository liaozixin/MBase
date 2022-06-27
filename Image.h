//
// Created by 15711 on 2022.04.09.
//

#ifndef MO_XI_FRAMEBUFFER_H
#define MO_XI_FRAMEBUFFER_H


#include <string>
#include <memory>
#include <vector>

class Image
{
public:
    Image(int width, int height, int channels);
    Image(std::string path);
    ~Image();
    unsigned char* Data();
    void Resize(int width, int height);
    void AddAlpha(int alpha);
    int Size();
    int Width();
    int Height();
    int Channels();
    int Steps();
private:
    unsigned char* m_data{nullptr};
    std::shared_ptr<std::vector<unsigned char> > m_imgData;
    int m_width;
    int m_height;
    int m_channels;
};





#endif //MO_XI_FRAMEBUFFER_H
