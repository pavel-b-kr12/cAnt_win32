#include <windows.h>
#include <winuser.h>

//=====================================
#include <cmath>
using namespace std;
#define PI 3.14

#include <chrono>  // for high_resolution_clock
#include <iostream>
#include <fstream>

class Ant
{
public:

    ~Ant();
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t dir = 0;
    uint32_t step_i = 0;
    Ant(uint16_t x, uint16_t y, uint16_t xM, uint16_t yM, uint16_t dir)
    {
        //std::cout << x << y << xM << yM  << " \n";
        this->x = x;
        this->y = y;
        this->xM = xM;
        this->yM = yM;
        this->dir = dir;
    }

    uint16_t Run(uint16_t c)
    {
        //std::cout << '\r' << x << y;
        //std::cout  << x << " " << y << " "<< c << " " << dir << " \n";

        switch (c)
        {
        default:
        case 0: c = 255;  if (dir == 3) dir = 0; else dir++; break;
        case 255: c = 0;  if (dir == 0) dir = 3; else dir--; break;
        }

        switch (dir) //step
        {
        case 4: dir = 0;
        case 0:  if (y == yM) y = 0; else y++; break;
        case 1:  if (x == xM) x = 0; else x++; break;
        case 2:  if (y == 0) y = yM; else y--; break;
        case 3:  if (x == 0) x = xM; else x--; break;
        }
        step_i++;
        return c;
    }

private:
    uint16_t xM = 0;
    uint16_t yM = 0;


};

Ant::~Ant()
{
}


const auto fieldSize = 1024;
const auto M = fieldSize;
const auto N = fieldSize;

// allocate (no initializatoin)
auto field = new double[M][N]();
auto arrHist = new double[M][N]();
auto arrHistVisits = new double[M][N]();
Ant a = Ant(fieldSize / 2, fieldSize / 2, fieldSize - 1, fieldSize - 1, 0);

//=====================================
typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        rgb.r = hsv.v; rgb.g = t; rgb.b = p;
        break;
    case 1:
        rgb.r = q; rgb.g = hsv.v; rgb.b = p;
        break;
    case 2:
        rgb.r = p; rgb.g = hsv.v; rgb.b = t;
        break;
    case 3:
        rgb.r = p; rgb.g = q; rgb.b = hsv.v;
        break;
    case 4:
        rgb.r = t; rgb.g = p; rgb.b = hsv.v;
        break;
    default:
        rgb.r = hsv.v; rgb.g = p; rgb.b = q;
        break;
    }

    return rgb;
}
//=================================

const int winx = 1200;
const int winy = 1080;
const int winbpp = 3;

BITMAPINFO m_bi;
char* buffer = 0;

int il = 0;
void setbuffer()
{
    auto displayType = GetKeyState(VK_CONTROL); //VK_LEFT

    m_bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_bi.bmiHeader.biWidth = winx;
    m_bi.bmiHeader.biHeight = winy;
    m_bi.bmiHeader.biPlanes = 1;
    m_bi.bmiHeader.biBitCount = 24;
    m_bi.bmiHeader.biCompression = BI_RGB;
    m_bi.bmiHeader.biSizeImage = 0;
    m_bi.bmiHeader.biXPelsPerMeter = 100;
    m_bi.bmiHeader.biYPelsPerMeter = 100;
    m_bi.bmiHeader.biClrUsed = 0;
    m_bi.bmiHeader.biClrImportant = 0;

    size_t paddedWidth = (winx * 3 + 3) & ~3;
    buffer = new char[paddedWidth * winy * winbpp];

    for (int y = 0; y < winy; ++y)
    {
        for (int x = 0; x < winx; ++x)
        {
            for (int z = 0; z < 3; ++z)
            {
                if (x < fieldSize && y < fieldSize)
                {
                    //HsvColor c = HsvColor();
                    //c.h = (char)arrHist[x][y] / 16;
                    //HsvToRgb(c).r

                    auto v = (int)(displayType == 0 ? field[x][y] : (int)(arrHist[x][y] / 64+z*60+il)%256);
                    
                    buffer[y * paddedWidth + x * winbpp + z] = v % 256;   //z * x+il;
                }

            }
        }
    }

    il++;
}


//========================

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

#define IDT_TIMER1 1001



VOID CALLBACK MyTimerProc(
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{
    

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) //10Mops = 122ms
    {
        uint16_t x = a.x;
        uint16_t y = a.y;
        //if (x > 1023 || y > 1023) std::cout << a.x << a.y << " \n";

        field[x][y] = a.Run(field[x][y]);
        arrHist[x][y] = a.step_i;
        arrHistVisits[x][y] ++;

        //std::cout << arrHistVisits[a.x][a.y] << " \n";
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count()*1000 << " ms\n";

    setbuffer();
    InvalidateRect(hwnd, NULL, FALSE);

}

#undef _main
#undef main

int WINAPI WinMain(HINSTANCE hThisInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszArgument,
    int nCmdShow)
{

    std::ofstream file;
    file.open("cout.txt");
    std::streambuf* sbuf = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf());
    //cout is now pointing to a file

    //==================


    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = L"CodeBlocksWindowsApp";
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

    if (!RegisterClassEx(&wincl))
        return 0;

    hwnd = CreateWindowEx(
        0,                   /* Extended possibilites for variation */
        L"CodeBlocksWindowsApp",         /* Classname */
        L" project 1 ",       /* Title Text */
        WS_OVERLAPPEDWINDOW, /* default window */
        CW_USEDEFAULT,       /* Windows decides the position */
        CW_USEDEFAULT,       /* where the window ends up on the screen */
        winx,                 /* The programs width */
        winy,                 /* and height in pixels */
        HWND_DESKTOP,        /* The window is a child-window to desktop */
        NULL,                /* No menu */
        hThisInstance,       /* Program Instance handler */
        NULL                 /* No Window Creation data */
    );

    ShowWindow(hwnd, nCmdShow);

    SetTimer(hwnd,             // handle to main window 
        IDT_TIMER1,            // timer identifier 
        16,
        (TIMERPROC)MyTimerProc);     // no timer callback 

    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    RECT client;
    DWORD result;

    switch (message)                  /* handle the messages */
    {
    case WM_CREATE:
        setbuffer();
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    case WM_TIMER: //^ never called
        setbuffer();
        InvalidateRect(hwnd, NULL, FALSE);
        //UpdateWindow(hwnd);
        break;

    case WM_PAINT:
        
        hDC = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &client);
        result = StretchDIBits(hDC,
            0, 0,
            client.right, client.bottom,
            0, 0,
            winx, winy,
            buffer, &m_bi, DIB_RGB_COLORS, SRCCOPY);
        if (result != winy)
        {
            //Drawing failed
            DebugBreak();
        }
        EndPaint(hwnd, &ps);


        break;

    case WM_DESTROY:
        delete buffer;
        PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
