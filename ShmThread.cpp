// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ShmThread.h"

#include "CleanupThread.h"
#include "functions.h"
#include "reqthread.h"
#include "graphthread.h"
#include "../mye/mye.h"
#include "mainform.h"

extern HANDLE hMapShmBuf;
extern char *shmbuf;
extern TCriticalSection *wcs;
extern wnd_buttons_v wnd_buttons;
extern TEvent *graphics_event;
extern TMutex *shmb;
TEvent *shme;
#pragma package(smart_init)
char *cmd;
char *data;
char *status;

DWORD *procId;
extern TCriticalSection *bgupd;
extern DWORD BackgroundProcessId;
extern BITMAPINFO bgbmi;
extern TEvent *bg_change;
extern char *bgbuf;

__fastcall ShmThread::ShmThread(bool CreateSuspended):TThread(CreateSuspended)
{
}

// ---------------------------------------------------------------------------
void __fastcall ShmThread::Execute()
{
    NameThreadForDebugging("ShmThread");

    status = shmbuf;
    cmd = shmbuf +sizeof(char);
    data = shmbuf +sizeof(char)+sizeof(char);
    shmbuf[0] = SHM_READY;
    shme->ResetEvent();

    while (1)
    {
        shme->WaitFor(INFINITE);

        if (status[0] != SHM_RREADY)
        {
            deb("shm signaled");
            shme->ResetEvent();
            continue;
        }

        // deb("shm cmd %d", cmd[0]);

        shme->ResetEvent();
        // Synchronize(Do);
        // shme->SetEvent();
        // }
        // }
        //
        // void __fastcall ShmThread::Do(void)
        // {

        if (!shmbuf)
        {
            MessageBox(Application->MainFormHandle, "error no shmbuf", "err", MB_OK);
            ExitProcess(0);
        }

        switch(cmd[0])
        {
            case LO_UPDWNDPIC:
            // deb("L:O_UPDWNDPIC");
            // ReadFile(hPipe, &hid, sizeof(hid), &dwRead, false);
            HANDLE hid;
            LPVOID p;
            HDC hdc;
            BITMAPINFO *pbmi;
            LPVOID dbuf;
            Graphics::TBitmap *ub;

            memcpy(&hid, data, sizeof(hid));
            // data += sizeof(HANDLE);

            bool fail;
            fail = false;
            wcs->Acquire();
            bool found;
            found = false;
            WNDBUTTONS *pwnd;
            pwnd = NULL;

            try
            {
                pwnd = (WNDBUTTONS*)hid;
                for (wnd_buttons_v::iterator it = wnd_buttons.begin();it!=wnd_buttons.end();it++)
                {
                    // deb("%x > %x",(*it)->id,hid);

                    if ((*it)->id == hid)
                    {
                        // deb("found %x",hid);
                        found = true;
                        break;
                    }
                }

                if (!found || IsBadReadPtr(pwnd, sizeof(WNDBUTTONS)))
                {
                    fail = true;
                }
                else
                {

                    // deb(" LO_UPDWNDPIC => %x ", pwnd);
                    // deb("%d:%d", pwnd->rect.left, pwnd->rect.top);

                    if (pwnd->pic)
                    {
                        // deb(" SHM:LO_UPDWNDPIC @ %p deleting %p", pwnd, pwnd->pic);
                        delete pwnd->pic;

                    }

                    ub = new Graphics::TBitmap();

                    pbmi = (BITMAPINFO*)(data +sizeof(HANDLE));
                    dbuf = data +sizeof(BITMAPINFO)+sizeof(HANDLE);

                    hdc = CreateCompatibleDC(0);

                    ub->SetSize(pwnd->rect.right - pwnd->rect.left, pwnd->rect.bottom - pwnd->rect.top);

                    if (!SetDIBits(hdc, ub->Handle, 0, pbmi->bmiHeader.biHeight, dbuf, pbmi, DIB_PAL_COLORS))
                    {
                        deb("bmi->h: %d bits %d", pbmi->bmiHeader.biHeight, pbmi->bmiHeader.biBitCount);
                        deb("bmihdr sz %d", pbmi->bmiHeader.biSize);
                        deb("updwndpic setdibits: %s", fmterr());
                    }
                    DeleteDC(hdc);

                    pwnd->pic = ub;

                }
            }
            catch(...)
            {
                char sz1[1024];
                // deb("exception in shmthread: %s", deunicode(e.Message.c_str(), sz1, sizeof(sz1)));
                deb("shmthread exception");
                fail = true;
                data[0] = SHM_FAIL;
            }

            if (!fail && found)
            {
                if (pwnd->visible)
                {
                    extern TEvent *bgraphics_event;

                    bgraphics_event->SetEvent();
                    graphics_event->SetEvent();
                }

                // deb("success update %x %d:%d v:%d", hid, pwnd->rect.left, pwnd->rect.top, pwnd->visible);
                data[0] = SHM_READY;
            }
            else
            {
                // deb("unsuccessfull update of %x", hid);
                data[0] = SHM_FAIL;
            }
            wcs->Release();
            break;

            /////////////////            /////////////////            /////////////////            /////////////////            /////////////////

            case LO_UPDATEBGBITMAP:
            //shmb->Acquire();
            // deb("case LO_UPDATEBGBITMAP:");
            extern Graphics::TBitmap *BackgroundBitmap;
            BackgroundBitmap = (Graphics::TBitmap*)1;
            procId = (DWORD*)data;

            BackgroundProcessId = (DWORD)*procId;

            bgupd->Enter();

            memcpy(&bgbmi, data+sizeof(DWORD) , sizeof(bgbmi));

            if (!bgbmi.bmiHeader.biSizeImage ||!bgbmi.bmiHeader.biHeight)
                deb("bmi bg height %d", bgbmi.bmiHeader.biHeight);
         //   deb("size %d", bgbmi.bmiHeader.biSizeImage);

            try
            {
                memcpy(bgbuf, data +sizeof(bgbmi)+sizeof(DWORD), bgbmi.bmiHeader.biSizeImage);
            }
            catch(...)
            {
                deb("exception while shmcopy bg");
            }
            //

            bgupd->Leave();

            bg_change->SetEvent();
         //   shmb->Release();

            graphics_event->SetEvent();

      //      deb("done bg upd");
            data[0] = SHM_READY;
            break;

            default:
            deb("unknwon shm cmd %d", *cmd);
            break;
        }

        *status = SHM_READY;

        // shmb->Release();

    }
}
// ---------------------------------------------------------------------------
