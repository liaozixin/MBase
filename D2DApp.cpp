#include "D2DApp.hpp"


#include <3rdparty/stb-master/stb_image.h>


#include <3rdparty/stb-master/stb_image_resize.h>

#include <cmath>
#include <cstdio>
#include <windowsx.h>
#include <chrono>


#ifndef HINST_THISCOMPONENT
    EXTERN_C IMAGE_DOS_HEADER __ImageBase;
    #define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

bool D2DApp::IF_EXIT = false;
bool D2DApp::IF_DRAW = false;
D2DApp::D2DApp(int width, int height, std::string title)
            :m_hwnd(nullptr),
            m_pRenderTarget(nullptr),
            m_pDirect2dFactory(nullptr),
            m_pBitmap(nullptr),
            m_width(static_cast<float>(width)),
            m_height(static_cast<float>(height)),
            m_frame(nullptr),
            m_mousePos(-100,-100),
            m_title(title)
{
    int lenght = width * height * 4;
    m_bg = new unsigned char[lenght];
    std::memset(m_bg, 255, lenght);
}

D2DApp::~D2DApp()
{
    delete[] m_bg;
}

void D2DApp::DealMessage()
{
    MSG msg;
    if (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT D2DApp::Initialize()
{
    HRESULT hr;
    hr = CreateDeviceIndependentResource();

    if(SUCCEEDED(hr))
    {
        WNDCLASSEX wcex = {sizeof(WNDCLASSEX)};
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = D2DApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = TEXT("D2DApp");
        
        RegisterClassEx(&wcex);
        FLOAT dpiX, dpiY;
        m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            "D2DApp",
            m_title.c_str(),
            WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                static_cast<UINT>(std::ceil(m_width * dpiX / 96.f)),
                static_cast<UINT>(std::ceil(m_height * dpiY / 96.f)),
                nullptr,
                nullptr,
                HINST_THISCOMPONENT,
                this
        );

        RECT winRec;
        GetWindowRect(m_hwnd, &winRec);
        RECT cliRect;
        GetClientRect(m_hwnd, &cliRect);
        int offsetWidth = (winRec.right - winRec.left) - (cliRect.right - cliRect.left);
        int offsetHeight = (winRec.bottom - winRec.top) - (cliRect.bottom - cliRect.top);
        int newWidth = m_width + offsetWidth;
        int newHeight = m_height + offsetHeight;
        int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
        int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;

        MoveWindow(
                m_hwnd,
                centerX - newWidth / 2,
                centerY - newHeight / 2,
                newWidth,
                newHeight,
                false
                );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }
    return hr;
}

HRESULT D2DApp::CreateDeviceIndependentResource()
{
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_pDirect2dFactory.GetAddressOf());
    return hr;
}

HRESULT D2DApp::CreateDeviceResource()
{
    HRESULT hr = S_OK;
    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(
                rc.right - rc.left,
                rc.bottom - rc.top
                );
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(m_hwnd, size),
                &m_pRenderTarget
                );


        hr = m_pRenderTarget->CreateBitmap(
                size,
                D2D1::BitmapProperties(
                        D2D1::PixelFormat(
                                DXGI_FORMAT_B8G8R8A8_UNORM,
                                D2D1_ALPHA_MODE_IGNORE
                        )
                ),
                &m_pBitmap
        );

    }
    return hr;
}

void D2DApp::DiscarDeviceResource()
{
    m_pDirect2dFactory = nullptr;
    m_pRenderTarget = nullptr;
    m_pBitmap = nullptr;
}

LRESULT CALLBACK D2DApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;
        D2DApp *pD2DApp = (D2DApp *) pcs->lpCreateParams;

        SetWindowLongPtrW(
                hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pD2DApp)
        );
        result = 1;
    }
    else
    {
        D2DApp* pD2DApp = reinterpret_cast<D2DApp*>(
                static_cast<LONG_PTR>(
                    GetWindowLongPtrW(
                            hWnd, GWLP_USERDATA
                            )
                ));
        bool wasHandled = false;
        if (pD2DApp)
        {
            switch (message)
            {
                case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = LOWORD(lParam);
                    pD2DApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hWnd, NULL, false);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_PAINT:
                {
                    pD2DApp->OnRender();
                    //ValidateRect(hWnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_MOUSEMOVE:
                    if(wParam & MK_LBUTTON)
                    {
                        std::get<0>(pD2DApp->m_mousePos) = GET_X_LPARAM(lParam);
                        std::get<1>(pD2DApp->m_mousePos) = GET_Y_LPARAM(lParam);
                        IF_DRAW = true;
                    }
                break;

                case WM_LBUTTONDOWN:
                    std::get<0>(pD2DApp->m_mousePos) = GET_X_LPARAM(lParam);
                    std::get<1>(pD2DApp->m_mousePos) = GET_Y_LPARAM(lParam);
                    IF_DRAW = true;
                break;

                case WM_LBUTTONUP:
                    IF_DRAW = false;
                break;

                case WM_DESTROY:
                {
                    IF_EXIT = true;
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }
        if (!wasHandled)
            result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    return result;
}

HRESULT D2DApp::OnRender()
{
    static auto timeBegin = std::chrono::steady_clock::now();
    static int frameCount{0};
    static int fps{0};

    HRESULT hr = S_OK;
    hr = CreateDeviceResource();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        if (!m_frame)
            m_frame = m_bg;
        hr = m_pBitmap->CopyFromMemory(
                nullptr,
                m_frame,
                m_width * 4
        );

        m_pRenderTarget->DrawBitmap(
                m_pBitmap.Get(),
                D2D1::RectF(0.0f, 0.0f, m_width, m_height),
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                D2D1::RectF(0.0f, 0.0f, m_width, m_height)
                );
        hr = m_pRenderTarget->EndDraw();
    }
    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscarDeviceResource();
    }
    ++frameCount;
    auto timeEnd = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::milli> elapsed = timeEnd - timeBegin;
    if (elapsed.count() > 1000.0)
    {
        fps = frameCount;
        std::string fpsInformation(
                "    Fps: " +
                std::to_string(fps)
        );
        SetWindowTextA(m_hwnd, (m_title + fpsInformation).c_str());
        timeBegin = timeEnd;
        frameCount = 0;
    }

    return hr;
}

void D2DApp::OnResize(unsigned int width, unsigned int height)
{
    if (m_pRenderTarget)
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
}

void D2DApp::DrawBmp(unsigned char* bitmap)
{
    m_frame = bitmap;
    for (int i = 0; i < m_width * m_height * 4; i+=4) {
        float alpha = (static_cast<float>(m_frame[i + 3]) / 255.f);
        m_frame[i + 0] = static_cast<unsigned char>((alpha * (float)m_frame[i + 0] + (1.f - alpha) * (float)m_bg[i + 0]));
        m_frame[i + 1] = static_cast<unsigned char>((alpha * (float)m_frame[i + 1] + (1.f - alpha) * (float)m_bg[i + 1]));
        m_frame[i + 2] = static_cast<unsigned char>((alpha * (float)m_frame[i + 2] + (1.f - alpha) * (float)m_bg[i + 2]));
        m_frame[i + 3] = static_cast<unsigned char>(255);
    }
    InvalidateRect(m_hwnd, nullptr, true);
    UpdateWindow(m_hwnd);
}

std::tuple<int, int> D2DApp::MousePos() const
{
    return m_mousePos;
}

void D2DApp::LoadBg(std::string path)
{
    int w, h, c;
    unsigned char* data = stbi_load(path.c_str(),
                                    &w, &h, &c,
                                    0);

    unsigned char* dataResize = new unsigned char[m_width * m_height * 4];

    stbir_resize(data, w, h, 0,
                 dataResize, m_width, m_height, 0,
                 STBIR_TYPE_UINT8, c, STBIR_ALPHA_CHANNEL_NONE, 0,
                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                 STBIR_COLORSPACE_SRGB, nullptr);

    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            m_bg[i * m_width * 4 + j * 4 + 2] = dataResize[i * m_width * c + j * c + 0];
            m_bg[i * m_width * 4 + j * 4 + 1] = dataResize[i * m_width * c + j * c + 1];
            m_bg[i * m_width * 4 + j * 4 + 0] = dataResize[i * m_width * c + j * c + 2];
            m_bg[i * m_width * 4 + j * 4 + 3] = static_cast<unsigned char>(255);
        }
    }


    delete[] dataResize;
    stbi_image_free(data);
}
