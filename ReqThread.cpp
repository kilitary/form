// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ReqThread.h"
#include "graphThread.h"
#include "functions.h"
#include "mainform.h"
#include "pluginsForm.h"

#include "Graphics.hpp"
#include "../mye/mye.h"
// #include <Streams.h>
#pragma package(smart_init)

extern TForm6 *Form6;
CRITICAL_SECTION wnd_s;
extern mye_GetVal_api get_val;
extern mye_SetVal_api set_val;
HDC temphdc = 0;
HWND temphwnd = 0;
extern HWND BackgroundWindow;
bool delete_wnd_buttons_job = false;
wnd_v wnd;
RECT rect3, monitorRect;
extern HDC bdc;
extern Graphics::TBitmap *bbmp;
extern Graphics::TBitmap *BackgroundBitmap;
DWORD BackgroundProcessId;
TEvent *breg_wnd_buttons_e = NULL;
TCriticalSection *bgupd = NULL;
extern int height, width;
extern Graphics::TBitmap *back;
extern CRITICAL_SECTION wnd_buttons_s;
extern DWORD time_prof;
keys_v keys;
wheel_events_v wheel_events;
extern TCriticalSection *wcs;
extern TCriticalSection *back_c;
extern HINSTANCE hInstance;
TCriticalSection *wec = NULL;
extern TEvent *graphics_event;
extern int monitors;
extern ReqThread *rth;
extern DrawThread *thr;
TEvent *bg_change = NULL;
wnd_buttons_v wnd_buttons;
extern char *bgbuf;
extern TEvent *te;
extern BITMAPINFO bgbmi;
extern Graphics::TBitmap *bbmp;
extern bool graphic_waiting;
TEvent *efl_clearwindows = NULL;
char sz1[128];

TCriticalSection *rwcs = NULL;
wnd_buttons_v reg_wnd_buttons;
Graphics::TBitmap *bg2bmp = NULL;
// ---------------------------------------------------------------------------

// Important: Methods and properties of objects in VCL can only be
// used in a method called using Synchronize, for example:
//
// Synchronize(&UpdateCaption);
//
// where UpdateCaption could look like:
//
// void __fastcall ReqThread::UpdateCaption()
// {
// Form1->Caption = "Updated in a thread";
// }
// ---------------------------------------------------------------------------
HANDLE hPipe = NULL;
HANDLE hMapBuf;
char *pmbuf;
extern Graphics::TBitmap *bgbmp;

__fastcall ReqThread::ReqThread(bool CreateSuspended):TThread(CreateSuspended)
{
    bg_change = new TEvent(false);
    Priority = tpHighest;
    efl_clearwindows = new TEvent(false);

    bgbmp = new Graphics::TBitmap();
    bgbmp->SetSize(width, height);
}
// ---------------------------------------------------------------------------

void __fastcall ReqThread::wndproc(Messages::TMessage &Message)
{
    Message.Result = true;
}

void __fastcall ReqThread::Repaint(void)
{
    // Form6->Repaint();
    // Form6->Perform(WM_PAINT,0,0);
    // Application->ProcessMessages();
}

void __fastcall ReqThread::Execute()
{

    wec = new TCriticalSection();

    hPipe = CreateNamedPipe("\\\\.\\pipe\\ReqForm", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE, PIPE_UNLIMITED_INSTANCES,
        1*1024*1024, 1*1024*1024, 5500, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        deb("ReqThread CreateNamedPipe: %s", fmterr());
        ExitThread(0);
    }

    SIZE_T mapsize = (3*1024*1024);
    hMapBuf = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, mapsize, "Global\\FormMapBuf");
    // deb("hMapBuf server %x", hMapBuf);
    pmbuf = (char*)MapViewOfFile(hMapBuf, FILE_MAP_WRITE, 0, 2*1024*1024, 0);
    if (!pmbuf)
        deb("MapViuewOfFile %s", fmterr());

    NameThreadForDebugging("ReqThread");
    // ---- Place thread code here ----
    DWORD tid = GetCurrentThreadId();
    deb("ReqThread id %x", tid);
    set_val("form.requests.threadid", &tid, sizeof(tid));

    while (1)
    {

        bool ret = ConnectNamedPipe(hPipe, NULL);
        if (!ret)
        {
            DWORD err = GetLastError();
            deb("ReqThread ConnectNamedPipe: %s", fmterr(err));
            if (err != 535)
            {
                // SpinWait(5000);
                WaitNamedPipeA("\\\\.\\pipe\\ReqForm", 1);
                continue;
            }
        }

         Synchronize(Do);
         }

 }


         void __fastcall ReqThread::Do(void)
         {

        long *pbuf;

        unsigned long msgid = 0;
        DWORD dwRead;
        HDC hdc = 0;
        HWND hwnd = 0;
        int toclear = 0;
        char buf[1024];
        int style;
        WND *nwnd;
        DWORD freed = 0;
        LPVOID dbuf;
        BITMAPINFO *pbmi;
        EVENT e;
        HANDLE hh;
        RECT rect;

        char *data;
        long id;
        BITMAPINFO bmi;
        DWORD ntry = 0;
        Graphics::TBitmap *ub;

        long size = 0;
        DWORD *pdata;
        Graphics::TBitmap *rbmp;
        TRect tr1, tr2;
        int r2;
        HANDLE hid;

        // ptick(0);
        ReadFile(hPipe, buf, 1, &dwRead, NULL);

        // if (buf[0]!=7)
      //  deb("Req: cmd %d", buf[0]);

        switch(buf[0])
        {
            /////////////////////C            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_DOCKWINDOW:

            HWND shwnd;
            ReadFile(hPipe, &shwnd, sizeof(shwnd), &dwRead, NULL);
            // deb("docking hwnd %x (read %d)", shwnd, dwRead);
            LPVOID addr;
            ReadFile(hPipe, &addr, sizeof(addr), &dwRead, NULL);
            // deb("addr %p", addr);
            char dllpipe[128];
            ReadFile(hPipe, dllpipe, sizeof(dllpipe), &dwRead, NULL);
            // deb("dllpipe %s", dllpipe);
            nwnd = new WND;
            nwnd->hwnd = shwnd;
            nwnd->onpaint = addr;
            nwnd->hdc = 0;
            strncpy(nwnd->dllpipe, dllpipe, sizeof(nwnd->dllpipe));
            RECT rect;
            GetWindowRect(shwnd, &rect);
            nwnd->width = rect.right - rect.left;
            nwnd->height = rect.bottom - rect.top;
            // deb("width %d wndwidth: %d", nwnd->width, WndWidth());
            nwnd->left = Form6->Width - nwnd->width - WndWidth();
            nwnd->top = Form6->Height - 32;
            // deb("left %d top %d", nwnd->left, nwnd->top);
            rect.left = nwnd->left;
            rect.top = nwnd->top;
            rect.right = rect.left + nwnd->width;
            rect.bottom = rect.top + nwnd->height;
            nwnd->rect = rect;
            // SetWindowPos(shwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
            SetWindowPos(shwnd, HWND_TOPMOST, nwnd->left, nwnd->top, 0, 0, SWP_NOSIZE);
            // SetWindowLong(shwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

            EnterCriticalSection(&wnd_s);
            wnd.push_back(nwnd);
            LeaveCriticalSection(&wnd_s);

            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            // graphics_event->SetEvent();
            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_REGISTERKEY:
            KEYS *k;
            k = new KEYS;

            ReadFile(hPipe, dllpipe, 128, &dwRead, NULL);

            char key;
            ReadFile(hPipe, &key, sizeof(key), &dwRead, NULL);
            ReadFile(hPipe, &addr, sizeof(addr), &dwRead, NULL);
            k->vkey = key;
            k->addr = addr;
            strcpy(k->dllpipe, dllpipe);
            keys.push_back(k);
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            deb("registered key '%c' addr %x", key, addr);
            break;
            /////////////////////C            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_DEREGISTERWINDOW:
            ReadFile(hPipe, &hid, sizeof(hid), &dwRead, NULL);
            dwRead = 0;
            for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
            {
                dwRead++;
                if ((*it)->id == hid)
                {
                    deb("deregistering wnd # %3d @ 0x%08p", dwRead, *it);
                    CloseHandle((*it)->hproc);
                    deb("freeing pic     %p", (*it)->pic);
                    delete(*it)->pic;
                    deb("freeing onhover %p", (*it)->onhover);
                    delete(*it)->onhover;
                    it = wnd_buttons.erase(it);
                    if (it == wnd_buttons.end())
                        break;
                }
            }
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            // graphics_event->SetEvent();
            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_GETBKGNDRECT:
            // graphics_event->ResetEvent();
            // deb("LO_GETBKGNDRECT");

            ReadFile(hPipe, &rect, sizeof(rect), &dwRead, NULL);

            rbmp = new Graphics::TBitmap();
            rbmp->Dormant();

            int hi, wi;
            wi = rect.right-rect.left;
            hi = rect.bottom-rect.top;
            rbmp->SetSize(wi, hi);
            // deb("set size %d:%d",wi,hi);


            tr1 = Rect(0, 0, wi, hi);

            tr2 = Rect(rect.left, rect.top, rect.right, rect.bottom);

            extern TCriticalSection *bmp2s;
            // bmp2s->Enter();
            bgupd->Enter();
            if (BackgroundBitmap)
            {

                if (bgbmp->Empty)
                {

                    bgbmp->LoadFromResourceName((unsigned int)hInstance, "bg1");

                }

            }
            else
            {
                 back_c->Enter();
                bgbmp->SetSize(width,height);
                bgbmp->Canvas->CopyRect(TRect(0, 0, back->Width, back->Width), back->Canvas, TRect(0, 0, back->Width, back->Height));
                back_c->Leave();
            }

            // rbmp->PixelFormat = bgbmp->PixelFormat;
            extern TMutex *shm;
            shm->Acquire();

            BITMAPINFO *pbmi;
            pbmi = (BITMAPINFO*)pmbuf;
            memset(pbmi, 0, sizeof(BITMAPINFO));
            // deb("pbmi @ %p", pbmi);
            pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

            // bgbmp->Canvas->Lock();
            rbmp->Canvas->CopyRect(tr1, bgbmp->Canvas, tr2);

            // bgbmp->Canvas->Unlock();

            hdc = CreateCompatibleDC(0);
            r2 = GetDIBits(hdc, (HBITMAP)rbmp->Handle, 0, rbmp->Height, 0, pbmi, DIB_PAL_COLORS);
            if (!r2)
            {
                deb("LO_GETBKGNDRECT getdibits: %s", fmterr());
                deb("biSize %d  (%d X %d)", pbmi->bmiHeader.biSize, rbmp->Width, rbmp->Height);
                id = -1;
                WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
                bgupd->Release();
                // bmp2s->Leave();
                delete rbmp;
                // delete bgbmp;
                DisconnectNamedPipe(hPipe);
                DeleteDC(hdc);
                shm->Release();
                break;
            }

            pbmi->bmiHeader.biCompression = BI_RGB;
            // pbmi->bmiHeader.biPlanes = 1;
            // pbmi->bmiHeader.biBitCount = 32;
            char *pdata2;
            pdata2 = (char*)pmbuf +sizeof(BITMAPINFO);

            if (!GetDIBits(hdc, (HBITMAP)rbmp->Handle, 0, pbmi->bmiHeader.biHeight, pdata2, pbmi, DIB_PAL_COLORS))
            {
                deb("LO_GETBKGNDRECT getdibits: %s", fmterr());
                deb("bmi->biHeight: %d", pbmi->bmiHeader.biHeight);
                deb("erroneous flow.");
                id = -1;
                WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
                delete rbmp;
                bgupd->Release();
                // bmp2s->Leave();
                // delete bgbmp;
                shm->Release();
                DisconnectNamedPipe(hPipe);
                DeleteDC(hdc);
                break;
            }

            shm->Release();

            id = 0;
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            DisconnectNamedPipe(hPipe);

            if (!DeleteDC(hdc))
                deb("deletedc: %s", fmterr());

            delete rbmp;

            bgupd->Leave();

            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_DRAW:
            graphics_event->SetEvent();
            WriteFile(hPipe, &hwnd, sizeof(hwnd), &dwRead, NULL);
            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_REGWHEELEVENT:

            // ReadFile(hPipe, &e, sizeof(e), &dwRead, NULL);

            ReadFile(hPipe, &addr, sizeof(addr), &dwRead, NULL);
            ReadFile(hPipe, dllpipe, sizeof(dllpipe), &dwRead, NULL);
            e.addr = addr;
            strcpy(e.dllpipe, dllpipe);
            wec->Acquire();
            wheel_events.push_back(e);
            wec->Release();
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_UPDWNDRECT:

            bool fail;
            fail = false;
            ReadFile(hPipe, &hh, sizeof(hh), &dwRead, NULL);
            ReadFile(hPipe, &rect, sizeof(rect), &dwRead, NULL);
            deb(" LO_UPDWNDRECT for %x to %d:%d", hh, rect.left, rect.top);
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);
            DisconnectNamedPipe(hPipe);

            wcs->Enter();
            try
            {
                WNDBUTTONS *pwnd = (WNDBUTTONS*)hh;
                pwnd->rect = rect;
            }
            catch(Exception& e)
            {
                deb("exception while LO_UPDWNDRECT: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
                fail = true;
            }
            wcs->Leave();

            if (!fail && rect.left < width && rect.top < height)
            {
                // te->SetEvent();
                graphics_event->SetEvent();
            }
            // Form6->Repaint();

            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_SETWNDVISIBLE:
            // graphics_event->ResetEvent();
            ReadFile(hPipe, &hh, sizeof(hh), &dwRead, NULL);
            ReadFile(hPipe, buf, 1, &dwRead, NULL);
            WriteFile(hPipe, &id, sizeof(id), &dwRead, NULL);

            wcs->Acquire();

            bool vsbl;
            vsbl = true;
            fail = false;
            try
            {
                WNDBUTTONS *pwnd = (WNDBUTTONS*)hh;
                pwnd->visible = buf[0];
                if (pwnd->rect.left>width || pwnd->rect.top>height)
                    vsbl = false;
            }
            catch(Exception& e)
            {
                deb("exception while LO_SETWNDVIISIBLE: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
                fail = true;
            }

            wcs->Release();

            // te->SetEvent();

            if (vsbl && !fail)
                graphics_event->SetEvent();
            break;
            /////////////////////C            /////////////////////C            /////////////////////C
            case LO_REGISTERWINDOW:
            // graphics_event->ResetEvent();
            // Priority = tpHighest;
            // deb("LO_REGISTERWINDOW");
            // ptick(0);
#pragma pack(1)
            typedef struct
            {
                LPVOID param;
                LPVOID addr;
                char dllpipe[128];
                RECT rect;
                DWORD procId;
                TColor tclr;
                char visible;
                int opacity;
                // DWORD size;
                // BITMAPINFO bmi;
            }tmp3;
#pragma pop(1)
            tmp3 tmp;
            ReadFile(hPipe, &tmp, sizeof(tmp), &dwRead, NULL);

            WNDBUTTONS *pwnd;

            pwnd = new WNDBUTTONS;
            pwnd->todelete = false;
            pwnd->param = tmp.param;
            pwnd->hproc = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE, false, tmp.procId);
            pwnd->addr = tmp.addr;
            pwnd->rect = tmp.rect;
            pwnd->id = (HANDLE)pwnd;
            pwnd->visible = tmp.visible;
            pwnd->opacity = tmp.opacity;

            if (tmp.param == (LPVOID) -1)
                pwnd->param = pwnd->id;

            strcpy(pwnd->dllpipe, tmp.dllpipe);

            pwnd->pic = new Graphics::TBitmap();
            pwnd->onhover = NULL;

            // deb("pwnd->pic %p pwnd->onhover %p # %3d", pwnd->pic, pwnd->onhover, wnd_buttons.size());
            // read pic 1
            ReadFile(hPipe, &size, sizeof(size), &dwRead, NULL);

            ReadFile(hPipe, &bmi, sizeof(bmi), &dwRead, NULL);

            pbuf = new long[size];
            ReadFile(hPipe, pbuf, size, &dwRead, NULL);

            hdc = CreateCompatibleDC(0);

            pwnd->pic->SetSize(bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight);
            int ret;
            ret = SetDIBits(hdc, pwnd->pic->Handle, 0, bmi.bmiHeader.biHeight, pbuf, &bmi, DIB_PAL_COLORS);
            if (!ret)
                deb("setdibits: %s", fmterr());

            delete[]pbuf;

            // read pic 2
            ReadFile(hPipe, &size, sizeof(size), &dwRead, NULL);
            if (size)
            {
                pwnd->onhover = new Graphics::TBitmap();
                // deb("pic 2 size %d", size);
                memset((void*)&bmi, 0, sizeof(bmi));
                ReadFile(hPipe, &bmi, sizeof(bmi), &dwRead, NULL);

                pbuf = new long[size];
                ReadFile(hPipe, pbuf, size, &dwRead, NULL);

                // hdc = CreateCompatibleDC(0);

                pwnd->onhover->SetSize(bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight);
                ret = SetDIBits(hdc, pwnd->onhover->Handle, 0, bmi.bmiHeader.biHeight, pbuf, &bmi, DIB_PAL_COLORS);
                if (!ret)
                    deb("setdibits 2: %s", fmterr());

                delete[]pbuf;
            }
            else
            {
                delete pwnd->onhover;
                pwnd->onhover = NULL;
            }
            DeleteDC(hdc);
            // delete[]pbuf;

            pwnd->transparentColor = tmp.tclr;

            hid = pwnd->id;

            WriteFile(hPipe, &hid, sizeof(hid), &dwRead, NULL);

            if (wcs->TryEnter())
            {

                wnd_buttons.push_back(pwnd);
                wcs->Release();
                // graphics_event->ResetEvent();
            }
            else
            {

                if (!rwcs->TryEnter())
                {

                    breg_wnd_buttons_e->SetEvent();

                    rwcs->Enter();
                    pwnd->reg_wnd_done = false;
                    reg_wnd_buttons.push_back(pwnd);
                    rwcs->Leave();

                }
                else
                {

                    pwnd->reg_wnd_done = false;
                    reg_wnd_buttons.push_back(pwnd);
                    rwcs->Leave();
                    te->SetEvent();
                }

            }
            extern TEvent *bgraphics_event;

            graphics_event->SetEvent();
            break;
            /////////////////////C

            case LO_CLEARWINDOWS:
            // graphics_event->ResetEvent();
            // delete_wnd_buttons_job = true;
            WriteFile(hPipe, &freed, sizeof(freed), &dwRead, NULL);
            DisconnectNamedPipe(hPipe);
            freed = 0;
            if (wnd_buttons.size() && wcs->TryEnter())
            {

                for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
                {

                    CloseHandle((*it)->hproc);
                    // deb("  + freeing pic      %p", wnd_buttons[i]->pic);
                    delete(*it)->pic;
                    // deb("  + freeing onhover  %p", wnd_buttons[i]->onhover);
                    if ((*it)->onhover)
                        delete(*it)->onhover;

                    delete[](*it);

                    freed++;
                }
                freed = wnd_buttons.size();
                wnd_buttons.clear();
                // wnd_buttons.resize(5000);
                deb("clear windows cleared %d wins", freed);
                wcs->Release();

            }
            else
                if (wnd_buttons.size())
                {
                    efl_clearwindows->SetEvent();
                    te->SetEvent();
                }

            // graphics_event->SetEvent();

            break;
            /////////////////////C            /////////////////////C            /////////////////////C            /////////////////////C

        }
        // LeaveCriticalSection(&wnd_s);

        dwRead = 0;
        // WriteFile(hPipe, &hwnd, sizeof(hwnd), &dwRead, NULL);
        // deb("wrote %d bytes", dwRead);
        temphdc = hdc;
        temphwnd = hwnd;
        // FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);

     //   deb(" done Req: cmd %d", buf[0]);
        // Sleep(0);
        // SpinWait(5000);
        // Synchronize(Repaint);

}
// ---------------------------------------------------------------------------
