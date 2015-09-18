// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TimedThread.h"
#include "mainform.h"
#include "functions.h"
#include "graphthread.h"
#include "../mye/mye.h"
#include <Psapi.h>
#include <vector>
#include <algorithm>

using namespace std;

#pragma package(smart_init)
int h, w;
extern TForm6 *Form6;
extern CRITICAL_SECTION wnd_s;
extern mye_SetVal_api set_val;
extern mye_GetVal_api get_val;
extern mye_GetBackgroundRect_api gbg;
extern TEvent *graphics_event;
extern wnd_v wnd;
extern HINSTANCE hInstance;
extern TCriticalSection *windows_s;
extern mye_EnumerateIcons_api enm_func;
extern mye_EnumerateVisibleWindows_api enm_windows_func;
extern int height, width;
extern DWORD updates;
extern TCriticalSection *wcs;
extern wnd_buttons_v wnd_buttons;
extern std::vector<WINDOWS_D>windows;
extern Graphics::TBitmap *tmph;
extern bool graphic_waiting;
TCriticalSection *vsws;
extern TEvent *breg_wnd_buttons_e;
wnd_buttons_v vwnd_buttons;
extern TEvent *efl_clearwindows;
TEvent *te;
extern vector<ICONS_D>icons;

extern TCriticalSection *rwcs;
extern wnd_buttons_v reg_wnd_buttons;
// ---------------------------------------------------------------------------

__fastcall TimedThread::TimedThread(bool CreateSuspended):TThread(CreateSuspended)
{
    te = new TEvent(false);
}

// ---------------------------------------------------------------------------
BOOL CALLBACK enumFunc(HWND hwnd, LPARAM lParam)
{
    char sztext[1024];

    GetWindowText(hwnd, sztext, sizeof(sztext));
    if (!sztext[0] || !IsWindowVisible(hwnd))
        return true;

    RECT rect;
    GetWindowRect(hwnd, &rect);

    int newh = 0, neww = 0;
    if (rect.bottom >= h - 32*2-20)
        newh = (h - 32*2-20) - (rect.top);

    if (newh)
    {
        // SetWindowPos(hwnd, HWND_TOP, 0, 0, rect.right-rect.left, newh, SWP_NOMOVE|SWP_NOZORDER);
    }

    return true;
}

void __fastcall TimedThread::Execute()
{
    NameThreadForDebugging("TimedThread");

    DWORD tid = GetCurrentThreadId();
    if (!set_val)
        deb("no set_Val!");

    set_val("form.timethread.threadid", &tid, sizeof(tid));

    while (1)
    {
        te->ResetEvent();
        te->WaitFor(20);

        // RemoveQueuedEvents(Do);
        Synchronize(Do);

      //   Do();
        // Sleep(50);

    }
}

bool sorticons(ICONS_D ic, ICONS_D ic2)
{
    return ic.pn.hWnd < ic2.pn.hWnd;
}

bool setfound(ICONS_D ic)
{
    ic.found = false;
  //  deb("set ic %x false", ic.pn.hWnd);
}

//
void __fastcall TimedThread::Do(void)
{
    Application->ProcessMessages();
    // EnumWindows(enumFunc, NULL);

    EnterCriticalSection(&wnd_s);
    for (wnd_v::iterator it = wnd.begin();it!=wnd.end();it++)
    {
        HWND hwnd = (*it)->hwnd;
        DWORD trId, prId;
        trId = GetWindowThreadProcessId(hwnd, &prId);
        if (!trId || !prId)
        {
            it = wnd.erase(it);
            if (it == wnd.end())
                break;
        }

    }
    LeaveCriticalSection(&wnd_s);

    int x = 0, y = 0;
    for (int i = 0;;i++)
    {
        HWND hwnd = enm_windows_func(i);
        if (hwnd == (HWND)MYE_NO_MORE_ITEMS)
            break;

        bool found;
        found = false;
        for (std::vector<WINDOWS_D>::iterator it = windows.begin();it!=windows.end();it++)
        {
            if ((*it).hwnd == hwnd)
            {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        char title[128];
        GetWindowText(hwnd, title, sizeof(title));
        // deb("window #%d: %x '%s'", i, hwnd, title);

        // windows_s->Acquire();
        // windows_s wnd2 = windows;

        // windows_s->Release();

        WINDOWS_D wd;

        wd.hwnd = hwnd;
        strcpy(wd.title, title);

        DWORD dwProcId;
        GetWindowThreadProcessId(hwnd, &dwProcId);
        HANDLE hProc;
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, 0, dwProcId);
        char path[MAX_PATH];
        GetModuleFileNameEx(hProc, 0, path, MAX_PATH);
        CloseHandle(hProc);
        // deb("  %s", path);
        WORD wIcon;
        HICON hIcon;
        wIcon = 0;
        // SHFILEINFO psfi;
        // SHGetFileInfo(path, 0, &psfi, sizeof(psfi), SHGFI_ICON);
        hIcon = ExtractAssociatedIcon(hInstance, path, &wIcon);
        if (!hIcon)
            MessageBox(0, "extractasocpm failed!", "", MB_OK);

        // deb("  #%d", wIcon);

        x = 2+(10 * i) + (32 * i);
        y = 10;

        wd.rect = TRect(x, y, x+32, y+32);

        TIcon *tic = NULL;

        tic = new TIcon();
        tic->Handle = hIcon;

        wd.icon = tic;
        wd.x = x;
        wd.y = y;

        windows_s->Acquire();
        windows.push_back(wd);
        windows_s->Release();

    }

    for (std::vector<WINDOWS_D>::iterator it = windows.begin();it!=windows.end();it++)
    {
        HWND hwnd = (*it).hwnd;
        DWORD trId, prId;
        trId = GetWindowThreadProcessId(hwnd, &prId);
        if (!trId || !prId)
        {
            delete(*it).icon;

            windows_s->Acquire();
            it = windows.erase(it);
            windows_s->Release();
            if (it == windows.end())

                break;
        }
    }

    // Application->ProcessMessages();
    // SpinWait(15000);
    // SwitchToThread();

    if (reg_wnd_buttons.size() && breg_wnd_buttons_e->WaitFor(0) != wrSignaled && rwcs->TryEnter())
    {
        deb("reg_wnd_buttons %d", reg_wnd_buttons.size());

        for (wnd_buttons_v::iterator it = reg_wnd_buttons.begin();it!=reg_wnd_buttons.end();it++)
        {
            if (breg_wnd_buttons_e->WaitFor(0) == wrSignaled)
            {
                breg_wnd_buttons_e->ResetEvent();

                break;
            }

            if (!(*it)->reg_wnd_done && wcs->TryEnter())
            {
                if (!(*it)->reg_wnd_done)
                {
                    wnd_buttons.push_back((*it));
                    (*it)->reg_wnd_done = true;
                }

                wcs->Leave();
                // rwcs->Enter();
                it = reg_wnd_buttons.erase(it);
                if (it == reg_wnd_buttons.end())
                {
                    // rwcs->Leave();
                    break;
                }
                // rwcs->Leave();
            }
        }
        rwcs->Leave();
    }

    long ret;
    int pos;
    pos = 0;
    ICONS_D wd;

    memset(&wd, 0, sizeof(wd));
    int idx;
    idx = 0;
    int cell_idx;
    cell_idx = 0;
    NOTIFYICONDATAW pn;
    extern TCriticalSection *ics;
    extern Graphics::TBitmap *bmp1, *back;
    extern Graphics::TBitmap *bgbmp;

    extern TCriticalSection *bgupd;
    char sz1[128];
    bool found;
    Graphics::TBitmap *icobmp, *bmp;
    Graphics::TBitmap *tmp, *tmp2;
    DWORD id;
    TIcon *tic;
    tic = NULL;
    idx = 0;
    int skipped = 0, added = 0, changed = 0;

    ics->Enter();
    for_each(icons.begin(), icons.end(), setfound);
    ics->Leave();

    while ((ret = enm_func(idx, &pn)) == MYE_SUCCESS)
    {
        idx++;

        if (!bmp1&&!back)
            break;

        id = (UINT)pn.uID + (UINT)pn.hWnd + (UINT)pn.uCallbackMessage;

        found = false;
        for (vector<ICONS_D>::iterator it = icons.begin();it!=icons.end();it++)
        {
            if ((*it).id != id)
            {
                skipped++;
                // deb("skip: id: %x id it: %x", id, (*it).id);
                continue;
                // break;
            }
            found = true;
            // if ((*it).orighicon == pn.hIcon)
            // continue;

            changed++;

            deunicode(pn.szTip, (*it).tip, sizeof((*it).tip));
            // strncpy(, , 128);

            if (!pn.hIcon || !(pn.uFlags & NIF_ICON))
                break;

            bmp = new Graphics::TBitmap();
            tic = new TIcon();
            icobmp = new Graphics::TBitmap();
            //

            // if (!tic)

            //
            tic->ReleaseHandle();
            tic->Handle = pn.hIcon;
            // tic->SetSize(32,32);
            //
            if (tic->Width > width || tic->Width < 0 || tic->Height > height || tic->Height < 0)
                deb("tic %d:%d", tic->Width, tic->Height);
            try
            {
                icobmp->Assign(tic);
            }

            catch(Exception& e)
            {
                char sz1[128];
                deb("exception while assigning icobmp(%x): %s", pn.hIcon,
                    deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
            }

            TRect rect(0, 0, 32, 32);
            bmp->SetSize(rect.right-rect.left, rect.bottom-rect.top);

            // tmp = new Graphics::TBitmap();

            x = (2 + idx*10+(32*idx) + WndWidth());

            y = height - 36;

            RECT rect2 =
            {
                x, y, x+32, y+32
            };

            (*it).rect = rect2;

            bgupd->Enter();
            extern TCriticalSection *back_c;
            back_c->Enter();
            if (bgbmp&&!bgbmp->Empty)
            {
                tmp2 = bgbmp;
            }
            else
                tmp2 = back;

            // tmp->Canvas->Brush->Style = bsClear;

            // tmp->SetSize(rect.right-rect.left, rect.Bottom-rect.top);

            bmp->Canvas->CopyRect(TRect(rect), tmph->Canvas, TRect(rect2));
            back_c->Leave();
            bgupd->Leave();

            // tmp->Canvas->Draw(0, 0, tic);
            bmp->Canvas->Brush->Style = bsClear;
            // bmp->Canvas->Draw(0, 0, tmp);

            bmp->Canvas->StretchDraw(rect, icobmp);

            Graphics::TBitmap *icon = new Graphics::TBitmap();
            icon->Assign(bmp);
            // delete tmp;
            // bmp->Canvas->Draw( icobmp);
            delete icobmp;
            //
            delete tic;

            ics->Enter();
            delete(*it).ico;
            delete(*it).icon;
            (*it).orighicon = pn.hIcon;
            (*it).ico = bmp;
            (*it).icon = icon;
            (*it).found = true;
            memcpy(&(*it).pn, &pn, sizeof(pn));

            (*it).idx = idx;
            // (*it).id = id;
            ics->Leave();
            // if (found)

            break;
        }

        if (found)
            continue;

        // deb("will add: id: %x ", id);

        wd.id = id;
        wd.idx = idx;
        icobmp = new Graphics::TBitmap();
        bmp = new Graphics::TBitmap();
        tic = new TIcon();
        pos++;

        tic->Handle = pn.hIcon;
        //
        if (tic->Width > width || tic->Width < 0 || tic->Height > height || tic->Height < 0)
            deb("tic %d:%d", tic->Width, tic->Height);
        try
        {
            icobmp->Assign(tic);
        }
        catch(Exception& e)
        {
            char sz1[128];
            deb("exception while assigning icobmp(%x): %s", pn.hIcon, deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
        }

        TRect rect(0, 0, 32, 32);
        bmp->SetSize(32, 32);

        x = width - (32 + 10 + idx*10+(32*idx) + WndWidth());

        y = height - 36;

        RECT rect2 =
        {
            x, y, x+32, y+32
        };

        wd.rect = TRect(rect2);

        tmp2 = bgbmp ? bgbmp:back;

        if (!tmp2)
            break;

        bmp->Canvas->Brush->Style = bsClear;
        bgupd->Enter();
        extern TCriticalSection *back_c;
        back_c->Enter();
        bmp->Canvas->CopyRect(TRect(rect), tmph->Canvas, TRect(rect2));
        back_c->Leave();
        bgupd->Leave();

        bmp->Canvas->StretchDraw(rect, icobmp);

        Graphics::TBitmap *icon = new Graphics::TBitmap();

        icon->SetSize(32, 32);
        icon->Canvas->StretchDraw(TRect(0, 0, 32, 32), icobmp);
        //

        delete icobmp;
        delete tic;

        wd.ico = bmp;
        wd.orighicon = pn.hIcon;
        wd.idx = idx;
        wd.icon = icon;
        wd.found = true;
        memcpy(&wd.pn, &pn, sizeof(pn));

        deunicode(pn.szTip, wd.tip, sizeof(wd.tip));

        ics->Enter();
        icons.push_back(wd);
        ics->Leave();
        // deb("added: hwnd: %x uid: %x uclbk: %x id: %x", pn.hWnd, pn.uID, pn.uCallbackMessage, id);
        added++;

        cell_idx++;
        idx++;
    }

    ics->Enter();
    // vector<ICONS_D>::iterator it=remove_if(icons.begin(), icons.end(), bool remnotfound(ICONS_D ic) {return !ic.found});
    // icons.erase(it);
    for (vector<ICONS_D>::iterator it = icons.begin();it!=icons.end();it++)
    {
        if (!(*it).found)
        {
            deb("del icon %d",it);
            it = icons.erase(it);
            if (it == icons.end())
                break;
        }
    }

    sort(icons.begin(), icons.end(), sorticons);
    ics->Leave();

    if (added||changed)
    {

        graphics_event->SetEvent();
    }
   //  deb("icons: %d idx: %d skipped: %d added: %d changed: %d", icons.size(), idx, skipped, added,changed);

    if (efl_clearwindows->WaitFor(0) == wrSignaled)
    {
        wcs->Acquire();
        for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
        {

            CloseHandle((*it)->hproc);
            // deb("  + freeing pic      %p", wnd_buttons[i]->pic);
            delete(*it)->pic;
            // deb("  + freeing onhover  %p", wnd_buttons[i]->onhover);
            if ((*it)->onhover)
                delete(*it)->onhover;

            delete[](*it);

        }

        extern wnd_buttons_v wwnd_buttons;
        wwnd_buttons.clear();
        wnd_buttons.clear();
        wcs->Release();

        efl_clearwindows->ResetEvent();
    }

    extern TEvent *bg_change;
    extern Graphics::TBitmap *bg2bmp;
    extern char bgbuf[5*1024*1024];
    extern BITMAPINFO bgbmi;
    extern TCriticalSection *bmp2s;

    // if (bg_change->WaitFor(0) == wrSignaled && bgupd->TryEnter())
    // {
    //
    // // bmp2s->Enter();
    //
    // // if (bg2bmp)
    // // delete bg2bmp;
    //
    // // bg2bmp = new Graphics::TBitmap();
    // //if (!bg2bmp)
    // //  bg2bmp = new Graphics::TBitmap();
    //
    // // bg2bmp->Canvas->Lock();
    // //bg2bmp->Dormant();
    //
    // //bg2bmp->SetSize(width, height);
    //
    // HDC hdc;
    // hdc = CreateCompatibleDC(0);
    // if (!SetDIBits(hdc, bgbmp->Handle, 0, bgbmi.bmiHeader.biHeight, bgbuf, &bgbmi, DIB_PAL_COLORS))
    // {
    //
    // deb("bgupd setdibits(h=%d): %s", bgbmi.bmiHeader.biHeight, fmterr());
    // }
    //
    // // bg2bmp->Canvas->Unlock();
    // DeleteDC(hdc);
    //
    // // bmp2s->Leave();
    // bgupd->Leave();
    //
    // bg_change->ResetEvent();
    // }

    // bgraphics_event->SetEvent();
    // graphics_event->SetEvent();

}

// }

void __fastcall TimedThread::Paint(void)
{
    // Form6->Repaint();
}
// ---------------------------------------------------------------------------
