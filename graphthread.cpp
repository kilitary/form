// ---------------------------------------------------------------------------

#include <vcl.h>
#include "mainform.h"
#pragma hdrstop

#include "../mye/mye.h"
#include "graphthread.h"
#include "functions.h"
#include "pluginsForm.h"
#include <windows.h>
#include <pngimage.hpp>
#include <stdio.h>
#include "reqthread.h"
#include <Psapi.h>
#include <vector>

#include "reqthread.h"

#include <mmsystem.h>
#pragma package(smart_init)

using namespace std;
char *bgbuf;
Graphics::TBitmap *wbmp = NULL;
TCanvas *cnv = NULL;
Graphics::TBitmap *bmp1 = NULL, *bmp2 = NULL;
DWORD updates = 0;
int drawbg = 1;
extern TForm6 *Form6;
extern HINSTANCE hInstance;
extern mye_EnumerateVisibleWindows_api enm_windows_func;
extern TCriticalSection *back_c;
extern char ststring[1024];
extern int num_windows;
extern bool vdebug;
extern mye_EnumerateIcons_api enm_func;
extern long mouse_clicks;
extern int num_icons;
extern DWORD mmoves;
Graphics::TBitmap *back = NULL;
DWORD ststime;
extern mye_SetVal_api set_val;
extern mye_GetVal_api get_val;
extern ReqThread *rth;
extern char hint[1024];
extern HDC temphdc;
extern CRITICAL_SECTION wnd_s;
extern HWND temphwnd;
LOGFONT logFont;
extern TCriticalSection *vsws;
int numwin;
int prevnumwin;
extern TForm7 *Form7;
extern wnd_buttons_v wnd_buttons;
TEvent *graphics_event = NULL;
extern TCriticalSection *bgupd;
extern wnd_v wnd;
extern TCriticalSection *wcs;
extern int height, width;
extern TCriticalSection *bmp2s;
long graphics_delay = 30;
HFONT hFont = NULL, hFontOld = NULL;
DWORD time_prof = 0, max_time_prof = 0;
Graphics::TBitmap *tmph = NULL;
extern CRITICAL_SECTION wnd_buttons_s;
POINT pt;
bool drawstats = 0;
HWND BackgroundWindow = NULL;
extern ReqThread *rth;
extern DrawThread *thr;
HDC bdc = NULL;
extern wnd_buttons_v vwnd_buttons;
Graphics::TBitmap *bbmp = NULL;
Graphics::TBitmap *BackgroundBitmap = NULL;
std::vector<WINDOWS_D>windows;
TCriticalSection *windows_s;
DWORD graphic_waiting = false;
extern TEvent *bg_change;

#pragma pack(1)
BITMAPINFO bgbmi;
Graphics::TBitmap *bgbmp = NULL;
DWORD stfps, fps = 0, curfps = 0;
TEvent *bgraphics_event = NULL;
unsigned long di = 0;

TCriticalSection *ics = NULL;
vector<ICONS_D>wicons, icons;

__fastcall DrawThread::DrawThread(bool CreateSuspended):TThread(CreateSuspended)
{
    vsws = new TCriticalSection();

    bgraphics_event = new TEvent(false);

}

unsigned long breakdraw(void)
{
    unsigned long ret = 0;

    if ((bgraphics_event->WaitFor(0) == wrSignaled))
        di = (unsigned long)GetTickCount() - (unsigned long)graphic_waiting;

    if (di)
    {
        if (di > 10)
        {
            bgraphics_event->ResetEvent();
            return 0;
        }

        deb("breaking draw %u", ret);
        bgraphics_event->ResetEvent();

        graphics_event->SetEvent();
        graphic_waiting = 1;
    }

    return di;
}

// ---------------------------------------------------------------------------
void __fastcall DrawThread::Execute()
{
    NameThreadForDebugging("Graphics thread");

    graphics_event->SetEvent();

    graphic_waiting = 1;
    bgraphics_event->ResetEvent();

    while (1)
    {

        int ret;

        ret = graphics_event->WaitFor(35);

        if (ret == wrSignaled)
        {
            di = (GetTickCount() - graphic_waiting);

        }
        else
        {
            continue;
        }

        graphics_event->ResetEvent();
        bgraphics_event->ResetEvent();

        // RemoveQueuedEvents(DrawScreen);
      //   DrawScreen();
        Synchronize(DrawScreen);

        // deb("updates %d ", updates); // , bmp2->Width,bmp2->Height);
        graphic_waiting = GetTickCount();

    }
}

void __fastcall DrawThread::DrawBackground(TCanvas *cnv1)
{
    back_c->Enter();
    if (!back)
    {

        back = new Graphics::TBitmap();
        back->Dormant();
        back->LoadFromResourceName((unsigned int)hInstance, "bg1");

        bgbmp->SetSize(back->Width, back->Height);
        bgbmp->Canvas->CopyRect(TRect(0, 0, back->Width, back->Height), back->Canvas,
            TRect(0, 0, back->Width, back->Height));
    }

    if (!wbmp)
    {
        wbmp = new Graphics::TBitmap();
        wbmp->Dormant();
        if (!back)
            MessageBox(NULL, "wbmp shit", "err", MB_OK);
        wbmp->SetSize(back->Width, back->Height);
        // wbmp->Assign(back);
        wbmp->Canvas->CopyRect(TRect(0, 0, back->Width, back->Height), back->Canvas,
            TRect(0, 0, back->Width, back->Height));

        deb("wbmp %p %d:%d", wbmp, wbmp->Width, wbmp->Height);
    }
    back_c->Leave();
    if (BackgroundBitmap)
    {

        if (bgbmp&& bg_change->WaitFor(0) == wrSignaled)
        {

            bgupd->Enter();

            HDC hdc = CreateCompatibleDC(0);
            if (!SetDIBits(hdc, bgbmp->Handle, 0, bgbmi.bmiHeader.biHeight, bgbuf, &bgbmi, DIB_PAL_COLORS))
            {

                deb("bgupd setdibits(h=%d,%d): %s", bgbmi.bmiHeader.biHeight, bgbmi.bmiHeader.biSizeImage, fmterr());
            }

            // bgbmp->Canvas->Unlock();
            DeleteDC(hdc);
            bgupd->Leave();
            bg_change->ResetEvent();
        }

        // BackgroundBitmap->Canvas->Lock();

        // // bgbmp->Assign(BackgroundBitmap);
        // if (!BackgroundBitmap->Width || !BackgroundBitmap->Height)
        // MessageBox(Application->MainFormHandle, "bgbmp no w/h", "err", MB_OK);
        bgupd->Enter();
        // bgbmp->Canvas->Lock();
        if (!bgbmp->Empty)
        {
            try
            {

                cnv1->Draw(0, 0, bgbmp);

            }
            catch(Exception& e)
            {
                char sz1[128];
                deb("exception while drawing bgbmp on cnv1: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
            }
            bgupd->Leave();
            return;
        }
        bgupd->Leave();
    }
    // bgbmp->Canvas->Unlock();

    back_c->Enter();

    try
    {
        cnv1->Draw(0, 0, back);
    }
    catch(Exception& e)
    {
        char sz1[128];
        deb("exception while drawing back on cnv1: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
    }
    back_c->Leave();

}

int iFunGetTime(FILETIME ftTime)
{
    SYSTEMTIME stTime;
    int iTime;
    FileTimeToSystemTime(&ftTime, &stTime);
    iTime = stTime.wSecond * 1000;
    iTime += stTime.wMilliseconds;
    return iTime;
}

void __fastcall DrawThread::DrawDockedDC(TCanvas *cnv1)
{

    static long id = 0;

    for (wnd_v::iterator it = wnd.begin();it!=wnd.end();it++)
    {
        if (!(*it)->onpaint)
            continue;

        // if (!(*it)->hdc)

        // SetWindowPos((*it)->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOZORDER);
        (*it)->hdc = GetWindowDC((*it)->hwnd);

        // deb("%d:%d",(*it)->rect.left,(*it)->rect.top);
        HDC hndl;

        // BackgroundBitmap->Canvas->Lock();

        // if (bg_change->WaitFor(0) == wrSignaled)
        // {
        // if (BackgroundBitmap && bgbmp && bgupd->TryEnter())
        //
        // {
        // bgbmp->Canvas->Lock();
        // if (!bgbmp->Empty && bgbmp->Width && bgbmp->Height)
        // {
        //
        // // wbmp->Assign(bgbmp);
        // wbmp->Canvas->Lock();
        // wbmp->SetSize(bgbmp->Width, bgbmp->Height);
        // wbmp->Canvas->CopyRect(TRect(0, 0, bgbmp->Width, bgbmp->Height), bgbmp->Canvas,
        // TRect(0, 0, bgbmp->Width, bgbmp->Height));
        // wbmp->Canvas->Unlock();
        //
        // }
        // bgbmp->Canvas->Unlock();
        // bgupd->Release();
        // }
        // }

        // deb("hndl %x", wbmp->Handle);

        // wbmp->Canvas->Lock();

        bgupd->Enter();
        if (BackgroundBitmap)
            hndl = bgbmp->Canvas->Handle;
        else
            hndl = cnv1->Handle;
        hndl = tmph->Canvas->Handle;

        int ret = BitBlt((*it)->hdc, 0, 0, (*it)->width, (*it)->height, hndl, (*it)->rect.left, (*it)->rect.top,
            SRCCOPY);
        bgupd->Leave();

        // wbmp->Canvas->Unlock();

        if (!ret)
            deb("BitBlt for docked dc %x: %s", (*it)->hdc, fmterr());

        // BackgroundBitmap->Canvas->Unlock();

        EVENT e;

        e.msg = LO_ONPAINT;
        e.addr = (*it)->onpaint;
        DWORD dwRead;
        // MessageBoxA(NULL, "calling  onpaint()",NULL,MB_OK);
        unsigned long ret2 = CallNamedPipeA((*it)->dllpipe, &e, sizeof(e), &e, sizeof(e), &dwRead, 1000);

        // ret2 = BitBlt(cnv->Handle, (*it)->rect.left, (*it)->rect.top, (*it)->width, (*it)->height, (*it)->hdc, 0, 0,
        // SRCCOPY);
        // if (!ret2)
        // deb("BitBlt for docked dc %x: %s", (*it)->hdc, fmterr());
        ReleaseDC((*it)->hwnd, (*it)->hdc);
    }
}

void __fastcall DrawThread::DrawButtons(TCanvas *cnv1)
{
    POINT pos;
    DWORD ex;

    GetCursorPos(&pos);

    //
    // if(!wcs->TryEnter())
    // return;
    static wnd_buttons_v wwnd_buttons;
    if (wcs->TryEnter())
    {
        wwnd_buttons = wnd_buttons;
        wcs->Release();
    }

    for (wnd_buttons_v::iterator it = wwnd_buttons.begin();it!=wwnd_buttons.end();it++)
    {

        if (!(*it)->visible||(*it)->rect.top < 0 || (*it)->rect.bottom > height || (*it)->rect.left < 0 || (*it)
            ->rect.right > width)
            continue;

        if ((*it)->transparentColor)
        {
            (*it)->pic->Transparent = true;
            (*it)->pic->TransparentColor = (*it)->transparentColor;
            (*it)->onhover->Transparent = true;
            (*it)->onhover->TransparentColor = (*it)->transparentColor;
        }

        if (PtInRect((*it)->rect, pos) && (*it)->onhover)
        {
            // deb(" mouse in window %p rect @ %d:%d",(*it),(*it)->rect.left,(*it)->rect.top);
            cnv1->Draw((*it)->rect.left, (*it)->rect.top, (*it)->onhover, (*it)->opacity);
        }
        else
            cnv1->Draw((*it)->rect.left, (*it)->rect.top, (*it)->pic, (*it)->opacity);
    }

}

void __fastcall DrawThread::DrawStats(TCanvas *cnv1)
{
    HDC savedc = cnv1->Handle;
    static int saveddc = 0;

    if (!drawstats)
    {
        static char supd[128] = "";

        static DWORD sttick, curtick;
        curtick = GetTickCount();

        curfps = updates;
        if (curtick >= sttick+500)
        {

            fps = (curfps-stfps)*2;
            stfps = updates;
            sttick = curtick;
            set_val("form.stats.curfps", &fps, sizeof(fps), "frames per second");

        }
        sprintf(supd, "%u FPS", fps);

        cnv1->Brush->Style = bsClear;
        cnv1->Font->Name = "consolas";
        cnv1->Font->Color = clWhite;

        cnv1->TextOutA(width - cnv1->TextWidth(supd), 1, supd);
        return;
    }

    // cnv1->Font->Handle = hFont;
    cnv1->Font->Size = 8;
    cnv1->Brush->Style = bsClear;

    DWORD gdi;
    gdi = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    DWORD user;
    user = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
    DWORD handles;
    GetProcessHandleCount(GetCurrentProcess(), &handles);
    char sgdi[128];
    sprintf(sgdi, "GDI:                      %-10d", gdi);

    char suser[128];
    sprintf(suser, "USER:                     %-10d", user);

    char shandles[128];
    sprintf(shandles, "Handles:                  %-10d", handles);

    PROCESS_MEMORY_COUNTERS_EX pmce;
    GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS) &pmce, sizeof(pmce));
    char spagefault[128];
    sprintf(spagefault, "Page faults:              %-10d", pmce.PageFaultCount);
    char swork[129];
    float mb = pmce.WorkingSetSize, mb2 = 1024/1024;
    mb = mb/1024/1024;
    sprintf(swork, "Working set size:         %-8d (%6.2f MB)", pmce.WorkingSetSize, mb);
    SIZE_T wMin, wMax;
    GetProcessWorkingSetSize(GetCurrentProcess(), &wMin, &wMax);
    char swmin_wmax[128];
    sprintf(swmin_wmax, "Working set min/max:      %-8d / %8d", wMin, wMax);

    char spriv[128];

    mb2 = pmce.PrivateUsage;
    mb = mb2/1024/1024;
    sprintf(spriv, "Private usage:            %-8d (%6.2f MB)", pmce.PrivateUsage, mb);

    PROCESS_HEAP_ENTRY Entry;
    Entry.lpData = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD dwMem = 0, blocks = 0;
    while (HeapWalk(hHeap, &Entry) != FALSE)
    {
        dwMem += Entry.cbData;
        blocks++;
    }
    char smem[128];
    mb2 = dwMem;
    mb = mb2/1024/1024;
    sprintf(smem, "Heap used:                %-8d (%6.2f MB in %3d blocks)", dwMem, mb, blocks);

    char smov[128];
    POINT pt;
    GetCursorPos(&pt);
    COLORREF cl = GetPixel(cnv1->Handle, pt.x, pt.y);
    int r = GetRValue(cl);
    int g = GetGValue(cl);
    int b = GetBValue(cl);
    sprintf(smov, "Mouse movements:          %-8d @ %4d:%-4d  RGB(%2x,%2x,%2x)", mmoves, pt.x, pt.y, r, g, b);

    static char supd[128] = "";

    static DWORD sttick, curtick;
    curtick = GetTickCount();
    static DWORD stupdates, fps, curfps;

    if (curtick >= sttick+500)
    {

        fps = (updates-stupdates)*2;

        stupdates = updates;
        sttick = curtick;
        if (fps>0)
        {
            if (fps > 25)
                graphics_delay++;
            else
                if (graphics_delay)
                    graphics_delay--;
        }
        set_val("form.stats.curfps", &fps, sizeof(fps), "frames per second");
    }

    sprintf(supd, "Graphics updates:         %-8u (%d FPS, %2d ms delay)", updates, fps, graphics_delay);

    IO_COUNTERS ic;
    GetProcessIoCounters(GetCurrentProcess(), &ic);
    char sread[128];
    char swrite[128];
    char sother[128];
    sprintf(sread, "Read operations:          %-8d %8u bytes", ic.ReadOperationCount, ic.ReadTransferCount);
    sprintf(swrite, "Write operations:         %-8d %8u bytes", ic.WriteOperationCount, ic.WriteTransferCount);
    sprintf(sother, "Other operations:         %-8d %8u bytes", ic.OtherOperationCount, ic.OtherTransferCount);

    // calculate cpu usage
    static DWORD dwOldTime, dwNewTime, dwTime;
    static char scpu[128] = "*";
    static FILETIME kt, ut, kt2, ut2, ct, ct2, et, et2;
    double kt3, ut3, kt4, kt5;
    static DWORD stcputick = 0;
    static double lOldUser, lNewUser, lOldKernel, lNewKernel, iProcUsage, lUser, lKernel;

    GetProcessTimes(GetCurrentProcess(), &ct2, &et2, &kt2, &ut2);

    curtick = GetTickCount();
    if (curtick >= stcputick+250)
    {
        stcputick = curtick;
        // Sleep(1000);

        kt3 = (double)((double)kt2.dwLowDateTime - (double)kt.dwLowDateTime);
        ut3 = (double)((double)ut2.dwLowDateTime - (double)ut.dwLowDateTime);
        kt5 = 0.16;
        kt4 = (kt3 + ut3)/kt5;

        dwNewTime = timeGetTime();
        lNewUser = iFunGetTime(ut2);
        lNewKernel = iFunGetTime(kt2);

        lKernel = lNewKernel - lOldKernel;
        lUser = lNewUser - lOldUser;
        dwTime = dwNewTime-dwOldTime;
        iProcUsage = (((lKernel+lUser)*25.0) / dwTime);

        sprintf(scpu, "CPU usage:                %.2f%%", iProcUsage);
        char cpu = (char)iProcUsage;
        set_val("form.stats.cpuusage", &cpu, sizeof(cpu), "cpu usage");

        GetProcessTimes(GetCurrentProcess(), &ct, &et, &kt, &ut);

        lOldUser = iFunGetTime(ut);
        lOldKernel = iFunGetTime(kt);
        dwOldTime = timeGetTime();
    }

    char smclk[128];
    sprintf(smclk, "Mouse clicks:             %-8d", mouse_clicks);

    char scp[129];
    sprintf(scp, "Time profile:             %-8d msecs (max %d)", time_prof, max_time_prof);

    char swb[128];
    DWORD mem = sizeof(WNDBUTTONS) * wnd_buttons.size()+sizeof(WNDBUTTONS*);
    sprintf(swb, "Windows:                  %-8d (mem %d KB)", wnd_buttons.size(), mem/1024);

    // draw data
    int x = width-cnv1->TextWidth(smov);

    cnv1->TextOutA(x, 50, sgdi);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1), suser);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*2, shandles);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*3, spagefault);

    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*4, spriv);

    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*5, smem);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*6, swork);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*7, swmin_wmax);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*8, supd);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*9, smov);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*10, sread);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*11, swrite);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*12, sother);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*13, scpu);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*14, smclk);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*15, scp);
    cnv1->TextOutA(x, 50+(cnv1->TextHeight(sgdi)+1)*16, swb);

    cnv1->Brush->Color = RGB(r, g, b);
    TRect rect(width-90, 50+(cnv1->TextHeight(sgdi)+1)*9+4, width-85, 50+(cnv1->TextHeight(sgdi)+1)*9 + 9);
    cnv1->FillRect(rect);
    cnv1->Brush->Style = bsClear;

    // cnv->Handle=savedc;
}

int WndWidth(void)
{
    int totwidth = 0;

    if (!wnd.size())
        return 0;

    // deb("windows plugins: %d",wnd.size());
    // MessageBox(NULL,"read deb",NULL,MB_OK);
    int pos = 0;
    // EnterCriticalSection(&wnd_s);
    wnd_v wnd2 = wnd;
    for (wnd_v::iterator it = wnd2.begin();it!=wnd2.end();it++, pos++)
    {

        // deb(" #%d %d + %d = %d", pos, width, (*it)->width, width+(*it)->width);
        totwidth += (*it)->width;
        // MessageBox(NULL,"next",NULL,MB_OK);
    }
    // LeaveCriticalSection(&wnd_s);

    return totwidth;
}

void __fastcall DrawThread::Paint(void)
{
    // deb("repaint");
    Form6->Repaint();
    // SendMessage(Application->MainFormHandle, WM_PAINT, 0, 0);
    // Form6->Perform(WM_PAINT,0,0);
    // deb("rep done");
}

void __fastcall DrawThread::DrawWindows(TCanvas *cnv1)
{

    // for (numwin = 0;;numwin++)
    // {
    // HWND hwnd = enm_windows_func(numwin);
    // if (hwnd == (HWND)MYE_NO_MORE_ITEMS)
    // break;
    // }
    // num_windows = numwin;
    // int pos;
    static std::vector<WINDOWS_D>wv;
    if (windows_s->TryEnter())
    {
        wv = windows;
        windows_s->Leave();
    }

    for (std::vector<WINDOWS_D>::iterator it = wv.begin();it!=wv.end();it++)
    {
        // try
        // {
        int opa;
        opa = 130;

        RECT rect =
        {
            (*it).rect.left, (*it).rect.top, (*it).rect.right, (*it).rect.bottom
        };

        if (PtInRect(&rect, pt))
        {
            opa = 255;
            // deb("wind %s",(*it).title);

            Graphics::TBitmap *tmp = new Graphics::TBitmap(), *tmp2 = new Graphics::TBitmap();
            tmp->Dormant();
            tmp2->Dormant();
            tmp->SetSize(32, 32);
            tmp2->SetSize(40, 40);
            // tmp->Canvas->Brush->Style = bsClear;
            bgupd->Enter();
            tmp->Canvas->CopyRect(TRect(0, 0, 40, 40), bgbmp->Canvas,
                TRect((*it).x-1, (*it).y-1, (*it).x+39, (*it).y+39));
            bgupd->Leave();

            tmp->Canvas->Draw(0, 0, (*it).icon); // , 150);
            tmp2->Canvas->StretchDraw(TRect(0, 0, 40, 40), tmp);

            cnv1->Draw((*it).x-4, (*it).y-4, tmp2, opa);

            delete tmp;
            delete tmp2;

        }
        else
        {
            Graphics::TBitmap *tmp = new Graphics::TBitmap(), *tmp2 = new Graphics::TBitmap();
            tmp->Dormant();
            tmp2->Dormant();

            tmp->SetSize(32, 32);
            tmp2->SetSize(32, 32);
            tmp->Canvas->Brush->Style = bsClear;
            bgupd->Enter();
            tmp->Canvas->CopyRect(TRect(0, 0, 32, 32), bgbmp->Canvas, TRect((*it).x, (*it).y, (*it).x+32, (*it).y+32));
            bgupd->Leave();
            tmp->Canvas->Draw(0, 0, (*it).icon, opa);
            // tmp2->Assign(tmp);
            tmp2->Canvas->Draw(0, 0, tmp);
            tmp->Canvas->Draw(0, 0, tmp2, opa);
            cnv1->Draw((*it).x, (*it).y, tmp, opa);

            delete tmp;
            delete tmp2;

        }
        // }
        // catch(Exception& e)
        // {
        // char sz1[128];
        // deb("exception while drawing icon on windows_d: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
        // }
    }

    num_windows = wv.size();
}

void __fastcall DrawThread::DrawIcons(TCanvas *cnv1)
{
    cnv->Pen->Style = psClear;

    cnv->Brush->Style = bsSolid;
    cnv->Brush->Color = RGB(0, 0x80, 0xff);
    int idx = 0, cell_idx = 0, pos = 0;
    NOTIFYICONDATAW pn;

    ics->Enter();
    for (vector<ICONS_D>::iterator it = icons.begin();it!=icons.end();it++)
    {
        // deb(" %d %s", pos, (*it).tip);
        // static Graphics::TBitmap *bmp = NULL;

        // int x = width - (32 + 10 + idx*10+(32*idx) + WndWidth());
        // int y = height - 36;
        int x = (5+(42*idx));
        int y = height - 36;
        try
        {
            int opa;
            opa = 130;

            // RECT rect =
            // {
            // (*it).rect.left, (*it).rect.top, (*it).rect.right, (*it).rect.bottom
            // };

            RECT rect =
            {
                x, y, x+32, y+32
            };

            if (PtInRect(&rect, pt))
            {
                opa = 255;

                if (vdebug)
                {
                    char txt[1024];
                    char sz1[128];
                    char hstr[129];
                    GetWindowTextA((*it).pn.hWnd, hstr, sizeof(hstr));
                    DWORD id = (*it).pn.uID+(DWORD)(*it).pn.hWnd+(*it).pn.uCallbackMessage;
                    sprintf(txt,
                        "icon #%2d\r\nid: %X\r\------------\r\nuid: %X\r\nucallback: %X\r\nhwnd: %X (%s)\r\nhicon: %x\r\ntip: %s\r\n"
                        , idx, id, (*it).pn.uID, (*it).pn.uCallbackMessage, (*it).pn.hWnd, hstr, (*it).pn.hIcon,
                        deunicode((*it).pn.szTip, sz1, sizeof(sz1)));
                    RECT rect =
                    {
                        4, 60, 1500, 1000
                    };
                    cnv1->Font->Size = 8;
                    DrawTextA(cnv->Handle, txt, strlen(txt), &rect, 0);
                }
            }

            cnv1->Draw(x, y, (*it).icon, opa);
        }
        catch(Exception& e)
        {
            char sz1[128];
            deb("exception while Draw ico(%x): %s", (*it).ico, deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
        }

        idx++;
    }
    ics->Leave();

    num_icons = idx;
}

void __fastcall DrawThread::DrawLines(TCanvas *cnv1)
{
    cnv1->Pen->Color = clDkGray;
    cnv1->Pen->Style = psSolid;
    cnv1->MoveTo(0, 48);
    cnv1->LineTo(width, 48);

    cnv1->MoveTo(0, height-44);
    cnv1->LineTo(width, height-44);
}

void __fastcall DrawThread::DrawHint(TCanvas *cnv1)
{
    if (hint[0])
    {
        Graphics::TBitmap *hbmp;
        hbmp = new Graphics::TBitmap();

        int x, y;
        // deb("x:y %d:%d pt.x %d pt.y %d",x,y,pt.x,pt.y);
        AnsiString as;
        as = hint;
        as = as.Trim();
        strcpy(hint, as.c_str());

        TCanvas *hcnv;

        hcnv = hbmp->Canvas;

        hcnv->Font->Name = "tahoma";
        hcnv->Font->Size = 12;
        hcnv->Font->Style = (TFontStyles)0;
        hcnv->Font->Color = clBlack;
        LOGFONT logFont;
        memset(&logFont, 0, sizeof(logFont));
        TFontStyles st;
        logFont.lfHeight = -(0.5 + 1.0 * hcnv->Font->Size * 83 / 80);
        logFont.lfWidth = 6;
        logFont.lfEscapement = 0;
        logFont.lfOrientation = 0;
        logFont.lfCharSet = DEFAULT_CHARSET;

        logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
        logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logFont.lfQuality = PROOF_QUALITY; // 1
        logFont.lfPitchAndFamily = DEFAULT_PITCH;
        char str[128];
        strncpy(logFont.lfFaceName, deunicode(hcnv->Font->Name.c_str(), str, sizeof(str)), sizeof(logFont.lfFaceName));
        HFONT hFont2 = NULL, hFontOld = NULL;
        hFont2 = CreateFontIndirect(&logFont);
        hcnv->Font->Handle = hFont2;

        x = (pt.x)+20;
        y = pt.y+20;

        if (x <= 10)
            x = 5;
        if (y <= 10)
            y = 5;

        if (y+hcnv->TextHeight(hint)>height-6)
            y = height-hcnv->TextHeight(hint)-6;

        int hi = hcnv->TextHeight(hint)+4;
        int wi = hcnv->TextWidth(hint)+10;

        hbmp->SetSize(wi, hi);

        hcnv->Brush->Style = bsClear;
        hcnv->Brush->Color = clWebGhostWhite; // RGB(0x00,0xff,0xce);//RGB(0xff, 0xff, 0xa0);

        hcnv->FillRect(TRect(0, 0, wi, hi));

        Graphics::TBitmap *hhbmp = new Graphics::TBitmap();
        hhbmp->SetSize(wi, hi);

        hcnv->Pen->Style = psSolid;
        hcnv->Pen->Color = clWebDarkSlategray;
        hcnv->Pen->Width = 1;
        hcnv->Rectangle(1, 1, wi, hi);
        hcnv->Pen->Style = psClear;

        hcnv->TextOutA(4, 2, hint);
        DeleteObject(hFont2);
        cnv1->Draw(x, y, hbmp); // x+hcnv->TextWidth(hint)+2, y+hcnv->TextHeight(hint)+2);
        delete hhbmp;
        delete hbmp;
        // TextOut(cnv->Handle, (int)x, (int)y, hint, strlen(hint));
    }
}

void __fastcall DrawThread::DrawVerbose(TCanvas *cnv1)
{
    if (ststring[0]!=0x20)
    {
        if ((GetTickCount() - ststime) > 4000)
        {
            ststring[0] = 0;

        }
        else
        {

            Graphics::TBitmap *stsbmp = new Graphics::TBitmap();
            stsbmp->Canvas->Font->Handle = hFont;
            stsbmp->Canvas->Font->Size = 12;
            stsbmp->Canvas->Font->Style = TFontStyles()<< fsBold;
            stsbmp->Canvas->Font->Color = clSilver;
            stsbmp->Canvas->Brush->Style = bsClear;

            stsbmp->SetSize(stsbmp->Canvas->TextWidth(ststring), stsbmp->Canvas->TextHeight(ststring));
            stsbmp->Canvas->CopyRect(TRect(0, 0, stsbmp->Width, stsbmp->Height), tmph->Canvas,
                TRect(width-stsbmp->Width - 10, 15, width-stsbmp->Width - 10+stsbmp->Canvas->TextWidth(ststring),
                    15 + stsbmp->Canvas->TextHeight(ststring)));

            RECT rect =
            {
                0, 0, icons.size()*42+5+stsbmp->Canvas->TextWidth(ststring), height-33
            };
            DrawTextA(stsbmp->Canvas->Handle, ststring, strlen(ststring), &rect, 0);

            cnv1->Draw(width-stsbmp->Width - 10, 15, stsbmp);
            delete stsbmp;
        }
    }
    extern bool valdebug;
    extern mye_EnumerateVal_api enm_val;

    if (valdebug && enm_val)
    {
        int idx = 0;
        char name[129];
        // char data[1024];
        char data[4096];
        int x = 10, y = 55;

        while (enm_val(NULL, idx, name, (unsigned char*)data) != MYE_NO_MORE_ITEMS)
        {

            char sz3[1024];
            char sadd[1281];
            bool ascii = true;
            for (int u = 0;u<3;u++)
                if (!isprint(data[u]))
                    ascii = false;
            if (ascii)
                snprintf(sadd, 155, "%s", data);
            else
                sadd[0] = 0x0;

            sprintf(sz3, "#%02d: %25s = %-8d 0x%08X %s", idx, name, (long) *data, (DWORD) *data, sadd);
            idx++;

            Graphics::TBitmap *stsbmp = new Graphics::TBitmap();
            stsbmp->Canvas->Font->Handle = hFont;
            stsbmp->Canvas->Font->Size = 8;
            stsbmp->Canvas->Font->Name = "consolas";
            // stsbmp->Canvas->Font->Style = TFontStyles()<< fsBold;
            stsbmp->Canvas->Font->Color = clWhite;
            stsbmp->Canvas->Brush->Style = bsClear;

            stsbmp->SetSize(stsbmp->Canvas->TextWidth(sz3), stsbmp->Canvas->TextHeight(sz3)-3);
            stsbmp->Canvas->CopyRect(TRect(0, 0, stsbmp->Width, stsbmp->Height), bgbmp->Canvas,
                TRect(x, 15, x - 10+stsbmp->Canvas->TextWidth(sz3), 15 + stsbmp->Canvas->TextHeight(sz3)-3));

            RECT rect =
            {
                0, 0, icons.size()*42+5+stsbmp->Canvas->TextWidth(sz3), height-33-3
            };
            DrawTextA(stsbmp->Canvas->Handle, sz3, strlen(sz3), &rect, 0);

            cnv1->Draw(x, y, stsbmp);
            y+=stsbmp->Canvas->TextHeight(sz3)-3;
            delete stsbmp;

        }
    }
}

void __fastcall DrawThread::DrawScreen(void)
{
    DWORD tick = 0;

    GetCursorPos(&pt);

    // bmp2s->Enter();
    // graphic_waiting = false;

    ptick(0);

    // cnv->Lock();

    try
    {
        TRect rect(0, 0, width, height);
        cnv->Brush->Color = clBlue;
        cnv->FillRect(rect);
    }
    catch(Exception& e)
    {
        char sz1[128];
        deb("exception while filling cnv bmp1 rect: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
    }

    cnv->Brush->Style = bsClear;
    cnv->Font->Name = "consolas";
    cnv->Font->Color = clWhite;

    if (!hFont)
    {
        TFontStyles st;
        logFont.lfHeight = -(0.5 + 1.0 * cnv->Font->Size * 96 / 80);
        logFont.lfWidth = 6;
        logFont.lfEscapement = 0;
        logFont.lfOrientation = 0;
        logFont.lfWeight = cnv->Font->Style.Contains(fsBold) ? 700:1950;
        logFont.lfItalic = cnv->Font->Style.Contains(fsItalic) ? TRUE:FALSE;
        logFont.lfUnderline = cnv->Font->Style.Contains(fsUnderline) ? TRUE:FALSE;
        logFont.lfStrikeOut = cnv->Font->Style.Contains(fsStrikeOut) ? TRUE:FALSE;
        logFont.lfCharSet = DEFAULT_CHARSET;
        logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
        logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logFont.lfQuality = PROOF_QUALITY; // 1
        logFont.lfPitchAndFamily = DEFAULT_PITCH;
        char str[128];
        strncpy(logFont.lfFaceName, deunicode(cnv->Font->Name.c_str(), str, sizeof(str)), sizeof(logFont.lfFaceName));
        hFont = CreateFontIndirect(&logFont);
    }

    if (drawbg)
        DrawBackground(cnv);
    else
    {
        cnv->Pen->Color = RGB(0, 0x80, 0xff);
        cnv->Brush->Color = RGB(0, 0x80, 0xff);
        cnv->Brush->Style = bsSolid;
        TRect rect(0, 0, width, height);
        cnv->FillRect(rect);
    }

    if (!tmph)
    {
        tmph = new Graphics::TBitmap();
        tmph->SetSize(width, height);
    }
    tmph->Canvas->Brush->Color = clBlack;
    tmph->Canvas->FillRect(TRect(0, 0, width, height));
    tmph->Canvas->Draw(0, 0, bgbmp, 200);
    cnv->CopyRect(TRect(0, 0, width, 48), tmph->Canvas, TRect(0, 0, width, 48));

    cnv->CopyRect(TRect(0, height-44, width, height), tmph->Canvas, TRect(0, height-44, width, height));

    DrawButtons(cnv);

    DrawWindows(cnv);

    DrawIcons(cnv);

    DrawLines(cnv);

    DrawHint(cnv);

    DrawVerbose(cnv);

    static TPngImage *down = NULL;
    if (!down)
    {
        down = new TPngImage();
        // down->LoadFromResourceName((unsigned int)hInstance, "down");
    }
    TRect rect1(width - down->Width, 0, width, down->Height);
    // cnv->StretchDraw(rect1, down);

    DrawStats(cnv);
    // cnv->Unlock();

    DrawDockedDC(cnv);

    bmp2s->Enter();
    // bmp1->SetSize(bmp2->Width, bmp2->Height);
    BitBlt(bmp1->Canvas->Handle, 0, 0, bmp2->Width, bmp2->Height, bmp2->Canvas->Handle, 0, 0, SRCCOPY);
    bmp2s->Leave();

    Paint();
    // Paint();

    ptick(1);
    static DWORD tot = 0;
    tot+=time_prof;
    if (time_prof>150)
        deb("drawscreen took %d ms, %d", time_prof, tot);

    // bgraphics_event->ResetEvent();
    updates++;

    // Form6->Perform(WM_PAINT,0,0);
    // MessageBoxA(NULL, "check",NULL,MB_OK);

    // Form6->Canvas->Unlock();

    // delete bmp1;
}
// ---------------------------------------------------------------------------
