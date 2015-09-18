// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CleanupThread.h"
#include "functions.h"
#include "reqthread.h"
#include "graphthread.h"
#include "../mye/mye.h"
#include "mainform.h"

#pragma package(smart_init)

extern wnd_buttons_v wnd_buttons;
extern bool delete_wnd_buttons_job;

extern TCriticalSection *wcs;
// ---------------------------------------------------------------------------

// Important: Methods and properties of objects in VCL can only be
// used in a method called using Synchronize, for example:
//
// Synchronize(&UpdateCaption);
//
// where UpdateCaption could look like:
//
// void __fastcall CleanupThread::UpdateCaption()
// {
// Form1->Caption = "Updated in a thread";
// }
// ---------------------------------------------------------------------------

__fastcall CleanupThread::CleanupThread(bool CreateSuspended):TThread(CreateSuspended)
{
}

// ---------------------------------------------------------------------------
void __fastcall CleanupThread::Execute()
{
    NameThreadForDebugging("CleanupThread");

    DWORD freed = 0;
    while (1)
    {
        freed = 0;

        if (delete_wnd_buttons_job)
        {
            
            delete_wnd_buttons_job = false;
        }

        Sleep(1);
    }
}
// ---------------------------------------------------------------------------
