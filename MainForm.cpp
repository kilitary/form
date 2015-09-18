// ---------------------------------------------------------------------------
#define STRSAFE_NO_DEPRECATE
#include <vcl.h>
#pragma hdrstop

#include "MainForm.h"
#include "functions.h"
#include "../mye/mye.h"
#include "psapi.h"
#include "AutorunThread.h"
#include "graphthread.h"
#include "TimedThread.h"
#include "ShmThread.h"
#include "ReqThread.h"
#include "cleanupThread.h"
#include <vector>

using namespace std;
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm6 *Form6;
mye_EnumerateIcons_api enm_func;
mye_TrayAddCallback_api add_func;
mye_TrayModifyCallback_api mod_func;
mye_TrayDeleteCallback_api del_func;
mye_ClickIcon_api clk_func;
bool vdebug = false;
mye_ClickWindow_api clk_win_func;
mye_RightClickWindow_api rclk_win_func;
mye_DblClickIcon_api dck_func;
extern vector<ICONS_D>icons, wicons;
extern Graphics::TBitmap *bgbmp;
mye_RightClickIcon_api rck_func;
mye_EnumerateVisibleWindows_api enm_windows_func;
mye_SetVal_api set_val;
mye_GetVal_api get_val;
extern char *bgbuf;
mye_EnumerateVal_api enm_val=NULL;
HMODULE hLib;
TCriticalSection *back_c;
int num_icons, num_windows;
extern HINSTANCE hInstance;
TCriticalSection *bmp2s = NULL;
extern wnd_v wnd;
extern DWORD BackgroundProcessId;
extern TCriticalSection *ics;
extern CRITICAL_SECTION wnd_s;
extern bool drawstats;
extern keys_v keys;
extern wheel_events_v wheel_events;
extern wnd_buttons_v wnd_buttons;
extern RECT monitorRect;
int height, width;
bool valdebug;
extern HWND BackgroundWindow;
extern bool graphic_waiting;
extern TCriticalSection *rwcs;
extern wnd_buttons_v reg_wnd_buttons;

wnd_buttons_v wwnd_buttons;
TCriticalSection *wcs;
extern std::vector<WINDOWS_D>windows;

long mouse_clicks = 0;
extern TEvent *graphics_event;
TForm *hfrm = NULL;
extern TEvent *shme;
POINT pt;
TMutex *shm;
char hint[1024];
HANDLE hMapShmBuf;
ReqThread *rth = NULL;
DrawThread *thr = NULL;
char *shmbuf;
extern Graphics::TBitmap *bmp2;
extern TCriticalSection *windows_s;
extern TEvent *breg_wnd_buttons_e;
extern TCriticalSection *wec;
extern int h, w;
extern TCriticalSection *bgupd;
extern Graphics::TBitmap *bmp1;
extern Graphics::TBitmap *BackgroundBitmap;

TMutex *shmb;

DWORD mmoves = 0;
CRITICAL_SECTION wnd_buttons_s;
int monitors = 0;

BOOL CALLBACK EnumMonitors(HMONITOR hm, HDC dcm, LPRECT rect, LPARAM p)
{
    monitors++;
}

void __fastcall onidle(void)
{
    // Application->ProcessMessages();
}

// ---------------------------------------------------------------------------
__fastcall TForm6::TForm6(TComponent *Owner):TForm(Owner)
{
    height = GetSystemMetrics(SM_CYSCREEN);
    width = GetSystemMetrics(SM_CXSCREEN);

    h = GetSystemMetrics(SM_CYSCREEN);
    w = GetSystemMetrics(SM_CXSCREEN);

    bmp2 = new Graphics::TBitmap();
    bmp1 = new Graphics::TBitmap();
    if (!bmp1)
        MessageBox(0, "new tbitmap failed", "", MB_OK);
    // bmp1->Dormant();

    bmp1->SetSize(width, height);

    // bmp2->Dormant();
    bmp2->SetSize(width, height);

    extern TCanvas *cnv;
    cnv = bmp2->Canvas;

    // ReportMemoryLeaksOnShutdown=true;
    IsMultiThread = true;
    bgbuf = new char[10*1024*1024];
    DoubleBuffered = false;

    rwcs = new TCriticalSection();
    breg_wnd_buttons_e = new TEvent(false);

    bmp2s = new TCriticalSection();
    bgupd = new TCriticalSection();

    graphics_event = new TEvent(false);
    windows_s = new TCriticalSection();

    back_c = new TCriticalSection();
    ics = new TCriticalSection();

    monitorRect.right = width;
    monitorRect.bottom = height;
    Form6->Height = height;
    Form6->Width = width;
    deb("Form: startup. screen size %d X %d", width, height);
    Form6->Left = 0;
    Form6->Top = 0;
    Form6->Color = RGB(0, 0x80, 0xff);

    hLib = LoadLibrary("mye.dll");
    deb("mye.dll hLib %x", hLib);
    enm_func = (mye_EnumerateIcons_api)GetProcAddress(hLib, "_mye_EnumerateIcons@8");
    add_func = (mye_TrayAddCallback_api)GetProcAddress(hLib, "_mye_TrayAddCallback@4");
    mod_func = (mye_TrayAddCallback_api)GetProcAddress(hLib, "_mye_TrayModifyCallback@4");
    del_func = (mye_TrayAddCallback_api)GetProcAddress(hLib, "_mye_TrayDeleteCallback@4");
    clk_func = (mye_ClickIcon_api)GetProcAddress(hLib, "_mye_ClickIcon@4");
    clk_win_func = (mye_ClickWindow_api)GetProcAddress(hLib, "_mye_ClickWindow@4");
    rclk_win_func = (mye_RightClickWindow_api)GetProcAddress(hLib, "_mye_RightClickWindow@4");
    dck_func = (mye_DblClickIcon_api)GetProcAddress(hLib, "_mye_DblClickIcon@4");
    rck_func = (mye_RightClickIcon_api)GetProcAddress(hLib, "_mye_RightClickIcon@4");
    enm_windows_func = (mye_EnumerateVisibleWindows_api)GetProcAddress(hLib, "_mye_EnumerateVisibleWindows@4");
    set_val = (mye_SetVal_api)GetProcAddress(hLib, "_mye_SetVal@16");
    get_val = (mye_GetVal_api)GetProcAddress(hLib, "_mye_GetVal@12");
    enm_val = (mye_EnumerateVal_api)GetProcAddress(hLib, "_mye_EnumerateVal@16");

    get_val("form.stats.draw", &drawstats, 1);
    get_val("form.debug.vdebug", &vdebug, 1);
    get_val("form.debug.valdebug", &valdebug, 1);
    InitializeCriticalSection(&wnd_s);
    InitializeCriticalSection(&wnd_buttons_s);
    wcs = new TCriticalSection();

    // wnd_buttons.resize (1000);

    hMapShmBuf = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 9*1024*1024, "Global\\FormShmBuf");
    if (!hMapShmBuf)
    {
        deb("Createfilemapping: %s", fmterr());
        MessageBox(Application->MainFormHandle, "error no shmbuf", "err", MB_OK);
        ExitProcess(0);
    }
    shmbuf = (char*)MapViewOfFile(hMapShmBuf, FILE_MAP_WRITE, 0, 0, 0);
    if (!shmbuf)
    {
        deb("MapViewOfFile: %s", fmterr());
        MessageBox(Application->MainFormHandle, "error no shmbuf", "err", MB_OK);
        ExitProcess(0);
    }
    shm = new TMutex((_SECURITY_ATTRIBUTES*)NULL, false, "FormReqMem", false);
    shmb = new TMutex((unsigned int)MUTEX_ALL_ACCESS, true, "FormShmMem", false);
    shme = new TEvent(NULL, false, false, "FormShmBufEvent", false);
    if (!shme || !shmb || !shm)
    {
        deb("shme: %x shmb: %x", shmb, shme);
        MessageBox(Application->MainFormHandle, "error no mutex/events", "err", MB_OK);
        ExitProcess(0);
    }

    EnumDisplayMonitors(GetWindowDC(0), NULL, EnumMonitors, NULL);
    deb("monitors: %d", monitors);

    thr = new DrawThread(1);
    // thr->Priority = tpIdle; // tpHighest;
    thr->Start();

    // CleanupThread *ctr = new CleanupThread(1);
    // ctr->Priority = tpLowest;
    // ctr->Start();
    rth = new ReqThread(1);
    // rth->Priority = tpHigher;
    rth->Start();

    TimedThread *th = new TimedThread(1);
    // th->Priority = tpIdle;
    th->Start();

    ShmThread *st = new ShmThread(1);
    // st->Priority = tpHighest;
    st->Start();

    // add_func(add_callback);
    // mod_func(mod_callback);
    // del_func(del_callback);
    // EnumerateIcons();
    int idx = 0;
    char name[129];
    // char data[1024];
    char data[4096];

    while (enm_val(NULL, idx, name, (unsigned char*) &data) != MYE_NO_MORE_ITEMS)
    {

        deb(" ctl #%02d: %25s = %d", idx, name, (long) *data);
        idx++;
    }

    // Left = width;
    Top = 0;

    HWND hwnde;
    hwnde = FindWindow("Shell_TrayWnd", NULL);
    deb("hwnde: %x", hwnde);
    SetWindowPos(hwnde, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

    AutorunThread *at = new AutorunThread(1);
    at->Priority = tpIdle;
    at->Start();
    // Application->OnIdle = onidle;
}

// ---------------------------------------------------------------------------

int WINAPI add_callback(LPVOID p, unsigned long size)
{
    static unsigned long msgid = 0;

    deb("--> add_callback(0x%08x, %d) id %d", p, size, msgid++);

    EnumerateIcons();
}

int WINAPI mod_callback(LPVOID p, unsigned long size)
{

    static unsigned long msgid = 0;
    // deb("--> mod_callback(0x%08x, %d) id %d ", p, size, msgid++);

    EnumerateIcons();
}

int WINAPI del_callback(LPVOID p, unsigned long size)
{

    static unsigned long msgid = 0;
    deb("--> del_callback(0x%08x, %d) id %d", p, size, msgid++);

    EnumerateIcons();
}

void EnumerateIcons(void)
{

    // deb("ret: %d", ret);
}

void __fastcall TForm6::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{

    RECT icRect;
    POINT pt;
    GetCursorPos(&pt);

    for (int i = 0;i<num_icons;i++)
    {
        icRect.left = ((5 + (42*i)));
        icRect.right = icRect.left + 37;
        icRect.top = Form6->Height - 36;
        icRect.bottom = icRect.top + 32;

        if (PtInRect(&icRect, pt))
        {
            deb("click on icon %d", i);
            if (Button == mbRight)
                rck_func(i?i:0);
            else
                clk_func(i?i:0);

        }
    }

    for (int i = 0;i<num_windows;i++)
    {
        icRect.left = (5 + (37*i));
        icRect.right = icRect.left + 37;
        icRect.top = 20;
        icRect.bottom = icRect.top + 36;

        if (PtInRect(&icRect, pt))
        {

            HWND hwnd = enm_windows_func(i);
            if (Button == mbRight)
                rclk_win_func(i);
            else
                clk_win_func(i);
            // SetForegroundWindow(hwnd);

            // SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
            // ShowWindow(hwnd, SW_SHOWNORMAL);
        }
    }

    icRect.left = Width-15;
    icRect.right = Width;
    icRect.top = 0;
    icRect.bottom = 5;
    if (PtInRect(&icRect, pt))
    {
        // SetWindowPos(Application->MainFormHandle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOMOVE);
        ShowWindow(Application->MainFormHandle, SW_SHOWMINIMIZED);
    }

    EVENT e;
    char dllpipe[128];
    e.addr = NULL;
    // EnterCriticalSection(&wnd_buttons_s);

    // wnd_buttons_v wnd2 = wnd_buttons;

    wcs->Acquire();
    for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
    {
        if ((*it)->todelete || !(*it)->visible)
            continue;

        if ((*it)->addr && PtInRect((*it)->rect, pt))
        {

            e.msg = LO_ONBCLICK;
            e.addr = (*it)->addr;
            e.param = (*it)->param;
            e.button = Button;
            strcpy(dllpipe, (*it)->dllpipe);
        }
    }
    wcs->Release();

    if (e.addr)
    {
        DWORD dwRead;
        // MessageBoxA(NULL, "calling  onpaint()",NULL,MB_OK);

        unsigned long ret2 = CallNamedPipeA(dllpipe, &e, sizeof(e), &e, sizeof(e), &dwRead, 1000);
    }

    mouse_clicks++;
}
// ---------------------------------------------------------------------------

void __fastcall TForm6::FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{

    mmoves++;
    RECT icRect;

    POINT pt;
    static POINT lpt;

    // GetCursorPos(&pt);
    pt.x = X;
    pt.y = Y;

    if (lpt.x == pt.x&& lpt.y == pt.y)
        return;

    lpt = pt;

    bool found = false;

    // deb("mouse move");
    int i = 0;
    ics->Enter();
    for (vector<ICONS_D>::iterator it = icons.begin();it!=icons.end();it++)
    {
        icRect.left = (((42*i)));
        icRect.right = icRect.left + 32;
        icRect.top = Form6->Height - 36;
        icRect.bottom = icRect.top + 32;

        if (PtInRect(&icRect, pt))
        {
            static int previc = 0;
            found = true;
            // NOTIFYICONDATAW pn;

            // enm_func(i, &pn);

            // char str[128];
            // deunicode(pn.szTip, str, sizeof(str));
            // deb("hint for icon %d: %s", i, );
            // strcpy(str, (*it).tip);

            // if (!(*it).tip[0])
            // continue;

            strncpy(hint, (*it).tip, sizeof(hint));
            if (graphic_waiting)
                graphics_event->SetEvent();

            ics->Leave();
            return;
        }
        i++;
    }
    ics->Leave();

    for (int i = 0;i<num_windows;i++)
    {
        icRect.left = ((42*i));
        icRect.right = icRect.left + 32;
        icRect.top = 20;
        icRect.bottom = icRect.top + 22;

        if (PtInRect(&icRect, pt))
        {
            found = true;
            static int prevwin = 0;

            HWND hwnd = enm_windows_func(i);
            if (hwnd == (HWND)MYE_NO_MORE_ITEMS)
                break;

            char str[128];
            GetWindowText(hwnd, str, 128);
            // deb("hint for window %d: %s", i, deunicode(szTip, str, sizeof(str)));

            // if (!str[0])
            // continue;

            strncpy(hint, str, sizeof(hint));
            if (graphic_waiting)
                graphics_event->SetEvent();

            return;
        }

    }

    if (pt.x < width && pt.x > 0 && pt.y > 0 && pt.y < height) // && wcs->TryEnter()
    {

        if (wcs->TryEnter())
        {
            wwnd_buttons = wnd_buttons;
            wcs->Leave();
        }
        // wcs->Enter();
        for (wnd_buttons_v::iterator it = wwnd_buttons.begin();it!=wwnd_buttons.end();it++)
        {
            // deb("size %d | it %p - %d", wwnd_buttons.size(), (*it), it);
            if (!(*it)->visible || !(*it)->onhover)
                continue;

            if (PtInRect((*it)->rect, pt))
            {
                if (graphic_waiting)
                    graphics_event->SetEvent();
                return;
            }
        }

    }
    if (hint[0] && !found)
    {
        hint[0] = 0;
        graphics_event->SetEvent();
    }
    else
        if (!found)
            hint[0] = 0;

}

void __fastcall TForm6::handleCustomMessage(TMessage &Message)
{
    Message.Result = true;
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::FormClose(TObject *Sender, TCloseAction &Action)
{
    EnterCriticalSection(&wnd_s);
    for (wnd_v::iterator it = wnd.begin();it!=wnd.end();it++)
    {
        HWND hwnd = (*it)->hwnd;
        DWORD prId;
        GetWindowThreadProcessId(hwnd, &prId);

        CloseWindow(hwnd);
        Sleep(10);

        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, 0, prId);
        if (!hProc)
            deb("openProcess to terminate: %s", fmterr());
        // deb("prId %x hProc %x",prId, hProc);
        TerminateProcess(hProc, 0);
        CloseHandle(hProc);
    }
    LeaveCriticalSection(&wnd_s);

    if (BackgroundWindow)
    {
        HWND hwnd = BackgroundWindow;
        DWORD prId;
        GetWindowThreadProcessId(hwnd, &prId);

        CloseWindow(hwnd);
        Sleep(10);

        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, 0, prId);
        if (!hProc)
            deb("openProcess to terminate: %s", fmterr());
        // deb("prId %x hProc %x",prId, hProc);
        TerminateProcess(hProc, 0);
        CloseHandle(hProc);
    }

    if (BackgroundProcessId)
    {
        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, 0, BackgroundProcessId);
        if (!hProc)
            deb("BackgroundProcessId openProcess to terminate: %s", fmterr());
        // deb("prId %x hProc %x",prId, hProc);
        TerminateProcess(hProc, 0);
        CloseHandle(hProc);
    }

    for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
    {

        TerminateProcess((*it)->hproc, 0);
        CloseHandle((*it)->hproc);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::FormPaint(TObject *Sender)
{
    // deb("formpaint");
    // graphics_event->SetEvent();

    // deb("onpaint()");

    // if (bmp2s->TryEnter())
    // {
    // bmp2->Assign(bmp1);
    // bmp2s->Release();
    // }

    // bmp1->Canvas->FillRect(TRect(0, 0, bmp1->Width, bmp1->Height));

    // bmp2->Canvas->CopyRect(TRect(0, 0, bmp1->Width, bmp1->Height), bmp1->Canvas,
    // TRect(0, 0, bmp1->Width, bmp1->Height));

    bmp2s->Acquire();
    // HRGN rgn = CreateRectRgn(30, 30, 650, 1200);
    // SelectClipRgn(Form6->Canvas->Handle, rgn);

    BitBlt(Canvas->Handle, 0, 0, width, height, bmp1->Canvas->Handle, 0, 0, SRCCOPY);

    // Canvas->Draw(0, 0, bmp1);
    bmp2s->Release();
    // deb("onpaint done");
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos,
    bool &Handled)
{
    // static DWORD sttick,curtick;
    //
    // curtick=GetTickCount();
    //
    //
    // if(curtick - sttick < 50)
    // {
    // Handled=true;
    // return;
    // }
    // sttick=curtick;

    wec->Acquire();
    for (wheel_events_v::iterator it = wheel_events.begin();it!=wheel_events.end();it++)
    {
        EVENT e;

        e.msg = LO_ONWHEELEVENT;
        e.addr = (*it).addr;
        e.shift = Shift;
        e.wheeldelta = WheelDelta;
        e.mousepos = MousePos;
        // memcpy(&e.mousepos, &MousePos, sizeof(MousePos));

        DWORD dwRead;
        // MessageBoxA(NULL, "calling  onpaint()",NULL,MB_OK);

        unsigned long ret2 = CallNamedPipeA((*it).dllpipe, &e, sizeof(e), &dwRead, sizeof(dwRead), &dwRead,
            NMPWAIT_WAIT_FOREVER);
        if (!ret2)
            deb("FormMouseWheel CallNamedPipeA: %s", fmterr());

    }
    wec->Release();

    Handled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TForm6::FormKeyPress(TObject *Sender, wchar_t &Key)
{
    char str[128];
    deunicode(&Key, str, 1);
    deb("key down form6: %x '%c'", str[0], str[0]);

    stdeb("Key: %c (0x02X)", str[0], str[0]);

    if (str[0] == 0x64)
    {
        vdebug = !vdebug;
        set_val("form.debug.vdebug", &vdebug, 1);
        stdeb("vdebug: %s", vdebug?"on":"off");
    }

    if (str[0] == 'v')
    {
        valdebug = !valdebug;
        set_val("form.debug.valdebug", &valdebug, 1);
        stdeb("valdebug: %s", valdebug?"on":"off");
    }

    EVENT e;
    for (keys_v::iterator it = keys.begin();it!=keys.end();it++)
    {
        if ((*it)->vkey == str[0])
        {
            e.msg = LO_ONKEY;
            e.addr = (*it)->addr;
            e.param = (LPVOID)(*it)->vkey;

            DWORD dwRead;
            // MessageBoxA(NULL, "calling  onpaint()",NULL,MB_OK);

            unsigned long ret2 = CallNamedPipeA((*it)->dllpipe, &e, sizeof(e), &e, sizeof(e), &dwRead, 1000);
        }
    }
    if (str[0] == VK_ESCAPE)
        Close();
    if (str[0] == 's')
    {
        drawstats = !drawstats;
        set_val("form.stats.draw", &drawstats, 1);
    }
}
// ---------------------------------------------------------------------------
