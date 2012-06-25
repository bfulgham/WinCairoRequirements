// cairotest.cpp 
//
// Simple test harness used to confirm Cairo function under Windows.
//
// * Uses code from Andrew Lim (danteshamest@gmail.com) from his example program
//   (see windrealm.com) for the GDI routines.
//
// * Uses code and suggestions from 
//   http://www.nullterminator.net/opengl32.html for OpenGL/WGL stuff.
//
// * Also uses the gtkmm cairo clock example code.
//

#include "stdafx.h"

#include <windows.h>

#include <commctrl.h>
#include <commdlg.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string>
#include <wininet.h>

#include <windows.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "cairo/cairo-gl.h"
#include "cairo/cairo-win32.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;                                // current instance
HWND hMainWnd;
WNDPROC DefEditProc = 0;
TCHAR szWindowClass[MAX_LOADSTRING] =  _T ("WIN_CAIRO_TEST");            // the main window class name
HDC dc;
HGLRC rc;
int LastUpdate = 0;
int Frames = 0;
int width = 400;
int height = 400;
cairo_t* cr = 0;
cairo_surface_t* surface = 0;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void SetClientSize(HWND hwnd, int clientWidth, int clientHeight);

void render();

#if USE_CONSOLE_ENTRY_POINT
int _tmain(int argc, _TCHAR* argv[])
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpstrCmdLine, int nCmdShow)
#endif
{
     // TODO: Place code here.
    MSG msg = {0};

    INITCOMMONCONTROLSEX InitCtrlEx;

    InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrlEx.dwICC  = 0x00004000; //ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&InitCtrlEx);

    MyRegisterClass(hInst);

    // Init COM
    OleInitialize(NULL);

    hMainWnd = CreateWindow(szWindowClass, _T ("Cairo Test Harness"), WS_OVERLAPPEDWINDOW,
                   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0, 0, hInst, 0);

    if (!hMainWnd)
        return FALSE;

    SetClientSize(hMainWnd, height, width);

    dc = ::GetDC (hMainWnd);

    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;
 
    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof (pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, iFormat, &pfd);
 
    rc = wglCreateContext(dc);
 
    cairo_device_t* device = cairo_wgl_device_create(rc);
 
    if (cairo_device_status(device) != CAIRO_STATUS_SUCCESS)
       printf("cairo device failed with %s\n", cairo_status_to_string(cairo_device_status(device)));
 
    surface = cairo_gl_surface_create_for_dc(device, dc, width, height);
 
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        printf("cairo surface failed with %s\n", cairo_status_to_string(cairo_surface_status(surface)));
 
    cairo_device_destroy(device);

    cr = cairo_create (surface);
 
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
       printf("cairo failed with %s\n", cairo_status_to_string(cairo_status(cr)));
 
    ShowWindow(hMainWnd, nCmdShow);

    bool running = true;
    LastUpdate = GetTickCount();

    wglMakeCurrent(dc, rc);

    while (running)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
            }
            else
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
        else
        {
            render();
            SwapBuffers(dc);
            ::Sleep (100);
        }
    }

#ifdef _CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif

    // Shut down COM.
    OleUninitialize();
    
    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINLAUNCHER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = 0;
    wcex.lpszMenuName   = 0; //MAKEINTRESOURCE(IDC_WINLAUNCHER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = 0; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

static const int dragBarHeight = 30;

LRESULT OnClose(HWND, WPARAM, LPARAM)
{
  PostQuitMessage(0);
  return 0;
}

/**
  Gradient demonstration.
  Taken from http://cairographics.org/samples/
*/
void gradientExample(cairo_t* cr)
{
  cairo_pattern_t *pat;

  pat = cairo_pattern_create_linear (0.0, 0.0,  0.0, 256.0);
  cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
  cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
  cairo_rectangle (cr, 0, 0, 256, 256);
  cairo_set_source (cr, pat);
  cairo_fill (cr);
  cairo_pattern_destroy (pat);

  pat = cairo_pattern_create_radial (115.2, 102.4, 25.6,
                                     102.4,  102.4, 128.0);
  cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
  cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
  cairo_set_source (cr, pat);
  cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * M_PI);
  cairo_fill (cr);
  cairo_pattern_destroy (pat);
}

/**
  Changes the dimensions of a window's client area.
*/
void SetClientSize(HWND hwnd, int clientWidth, int clientHeight)
{
  if (!IsWindow (hwnd))
      return;

  DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
  DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  HMENU menu = GetMenu(hwnd);
  RECT rc = { 0, 0, clientWidth, clientHeight } ;
  AdjustWindowRectEx( &rc, dwStyle, menu ? TRUE : FALSE, dwExStyle );
  SetWindowPos (hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                  SWP_NOZORDER | SWP_NOMOVE ) ;
}

/**
  Handles WM_PAINT.
*/
LRESULT OnPaint(HWND hwnd, WPARAM, LPARAM)
{
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint (hwnd, &ps);

  // Create the cairo surface and context.
  cairo_surface_t *surface = cairo_win32_surface_create (hdc);
  cairo_t *cr = cairo_create (surface);
  
  // Draw on the cairo context.
  gradientExample (cr);

  // Cleanup.
  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  EndPaint (hwnd, &ps);
  return 0 ;
}

/**
  Handles our window's messages.
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    //case WM_PAINT: return OnPaint(hWnd, wParam, lParam);
    case WM_CLOSE: 
        return OnClose(hWnd, wParam, lParam);
    case WM_DESTROY:
        /*cairo_destroy(cr);
        cairo_surface_destroy(surface);
 
        wglMakeCurrent( NULL, NULL );
        wglDeleteContext( rc );*/
 
        ReleaseDC(hWnd, dc);
        PostQuitMessage (0);
        break;

    default:
        return DefWindowProc(hWnd,message,wParam,lParam);
  }

  return DefWindowProc(hWnd,message,wParam,lParam);
}

void render()
{
    if (GetTickCount() - LastUpdate > 1000)
    {
        //std::cout << "Fps: " << Frames / ((GetTickCount() - LastUpdate) / 1000.f) << "\n";

        LastUpdate = GetTickCount();
        Frames = 0;
    }

    cairo_identity_matrix(cr);

    double m_radius = 0.42;
    double m_line_width = 0.05;

    cairo_scale(cr, width, height);
    cairo_translate(cr, 0.5, 0.5);
    cairo_set_line_width(cr, m_line_width);

    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.337, 0.612, 0.117, 0.9);   // green
    cairo_paint(cr);
    cairo_restore(cr);
    cairo_arc(cr, 0, 0, m_radius, 0, 2 * M_PI);
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_fill_preserve(cr);
    cairo_restore(cr);
    cairo_stroke_preserve(cr);
    cairo_clip(cr);

    //clock ticks
    for (int i = 0; i < 12; i++)
    {
        double inset = 0.05;

        cairo_save(cr);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

        if(i % 3 != 0)
        {
            inset *= 0.8;
            cairo_set_line_width(cr, 0.03);
        }

        cairo_move_to(cr,
                (m_radius - inset) * cos (i * M_PI / 6),
                (m_radius - inset) * sin (i * M_PI / 6));
        cairo_line_to (cr,
                m_radius * cos (i * M_PI / 6),
                m_radius * sin (i * M_PI / 6));
        cairo_stroke(cr);
        cairo_restore(cr); /* stack-pen-size */
    }

    // store the current time
    SYSTEMTIME time;
    GetLocalTime(&time);

    // compute the angles of the indicators of our clock
    double minutes = time.wMinute * M_PI / 30;
    double hours = time.wHour * M_PI / 6;
    double seconds= ((double)time.wSecond + (double)time.wMilliseconds / 1000) * M_PI / 30;

    cairo_save(cr);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // draw the seconds hand
    cairo_save(cr);
    cairo_set_line_width(cr, m_line_width / 3);
    cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.8); // gray
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(seconds) * (m_radius * 0.9),
            -cos(seconds) * (m_radius * 0.9));
    cairo_stroke(cr);
    cairo_restore(cr);

    // draw the minutes hand
    cairo_set_source_rgba(cr, 0.117, 0.337, 0.612, 0.9);   // blue
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(minutes + seconds / 60) * (m_radius * 0.8),
            -cos(minutes + seconds / 60) * (m_radius * 0.8));
    cairo_stroke(cr);

    // draw the hours hand
    cairo_set_source_rgba(cr, 0.337, 0.612, 0.117, 0.9);   // green
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(hours + minutes / 12.0) * (m_radius * 0.5),
            -cos(hours + minutes / 12.0) * (m_radius * 0.5));
    cairo_stroke(cr);
    cairo_restore(cr);

    // draw a little dot in the middle
    cairo_arc(cr, 0, 0, m_line_width / 3.0, 0, 2 * M_PI);
    cairo_fill(cr);

    cairo_surface_flush(surface);

    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
       printf("render failed with %s\n", cairo_status_to_string(cairo_status(cr)));

    Frames++;
}
