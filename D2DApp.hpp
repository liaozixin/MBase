#pragma once

#include <d2d1.h>
#include <d2d1helper.h>
#include <cstring>
#include <tuple>
#include <wrl.h>
#include <string>

class D2DApp
{
public:
    D2DApp (int width, int height, std::string title);
    ~D2DApp();
    D2DApp(const D2DApp&) = delete;
    D2DApp& operator=(const D2DApp&) = delete;

    HRESULT Initialize();
    void DealMessage();
    void DrawBmp(byte* bitmap);
    std::tuple<int,int> MousePos() const;
    void LoadBg(std::string path);

public:
    static bool IF_EXIT;
    static bool IF_DRAW;

private:
    HRESULT CreateDeviceIndependentResource();
    HRESULT CreateDeviceResource();
    void DiscarDeviceResource();
    HRESULT OnRender();
    void OnResize(unsigned int width, unsigned int height);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                    WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd;
    Microsoft::WRL::ComPtr<ID2D1Factory> m_pDirect2dFactory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_pBitmap;
    unsigned char* m_frame;
    unsigned char* m_bg;
    int m_width;
    int m_height;
    std::tuple<int, int> m_mousePos;
    std::string m_title;
};