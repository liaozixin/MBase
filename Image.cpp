//
// Created by 15711 on 2022.06.26.
//
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <3rdparty/stb-master/stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <3rdparty/stb-master/stb_image_resize.h>
#include <cstdio>
#include <cassert>

Image::Image(int width, int height, int channels) : m_width(width),
                                             m_height(height),
                                             m_channels(channels)
{
    m_data = new unsigned char[width * height * channels];
    memset(m_data, 255, sizeof(unsigned char) * width * height * channels);

    try
    {
        if (width < 0 || height < 0 || channels < 0 || channels > 4)
            throw "Image init error!";
    }
    catch (char* e)
    {
        printf("%s\n", e);
    }
    m_imgData = std::make_shared<std::vector<unsigned char> >(Size(),
                                                              static_cast<unsigned char>(255));
};

Image::Image(std::string path)
{
    auto tem = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
    m_data = new unsigned char[m_width * m_height * m_channels];
    for (int i = 0; i < m_height * m_width * m_channels; ++i)
        m_data[i] = static_cast<unsigned char>(tem[i]);

    stbi_image_free(tem);
};

Image::~Image()
{
    delete[] m_data;
    m_data = nullptr;
};

unsigned char *Image::Data()
{
    return this->m_data;
}

void Image::Resize(int width, int height)
{
    unsigned char* tem = new unsigned char[width * height * m_channels];

    stbir_resize(m_data, m_width, m_height, 0,
                 tem, width, height, 0,
                 STBIR_TYPE_UINT8, m_channels, STBIR_ALPHA_CHANNEL_NONE, 0,
                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                 STBIR_COLORSPACE_SRGB, nullptr);

    m_width = width;
    m_height = height;
    delete[] m_data;
    m_data = tem;
}

int Image::Size() {
    return (*m_imgData).size();
}

int Image::Width() {
    return m_width;
}

int Image::Height() {
    return m_height;
}

int Image::Channels() {
    return m_channels;
}

int Image::Steps() {
    return m_channels;
}

void Image::AddAlpha(int aplha) {
    assert(m_channels != 3);
    int size = 4 * m_width * m_height;

    auto tem = new unsigned char[size];

    for (int i = 0; i < size; i+=4) {
        tem[i] = m_data[i];
        tem[i + 1] = m_data[i + 1];
        tem[i + 2] = m_data[i + 2];
        tem[i + 3] = static_cast<unsigned char>(aplha);
    }

    delete[] m_data;
    m_data = tem;
    m_channels = 4;
}
