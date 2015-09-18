// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <dirent.h>
#include "AutorunThread.h"
#include "functions.h"
#include <Psapi.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma package(smart_init)

char ststring[1024];
// ---------------------------------------------------------------------------

// Important: Methods and properties of objects in VCL can only be
// used in a method called using Synchronize, for example:
//
// Synchronize(&UpdateCaption);
//
// where UpdateCaption could look like:
//
// void __fastcall AutorunThread::UpdateCaption()
// {
// Form1->Caption = "Updated in a thread";
// }
// ---------------------------------------------------------------------------

__fastcall AutorunThread::AutorunThread(bool CreateSuspended):TThread(CreateSuspended)
{
}

// ---------------------------------------------------------------------------
void __fastcall AutorunThread::Execute()
{
    NameThreadForDebugging("AutorunThread");
    // ---- Place thread code here ----

    char curdir[MAX_PATH];
    char dir[MAX_PATH];

    stdeb("Loading autorun entries ...");

    GetProcessImageFileName(GetCurrentProcess(), dir, sizeof(dir));
    /* for(int i=strlen(dir); i;i--)
    {
    if(dir[i] == '/' || dir[i] == '\\') {
    dir[i] = 0x0;
    break;
    }
    } */
    PathRemoveFileSpec(dir);
    //deb("base: %s", dir);
    int ret = SetCurrentDirectory(dir);
    if (ret != S_OK)
    {
        deb("failed change dir");
    }

    SetCurrentDirectory("autorun");

    GetCurrentDirectory(sizeof(curdir), curdir);
    //deb("running in %s", curdir);
    snprintf(dir, sizeof(dir), "%s", curdir);

    DIR *dr;

    if (!(dr = opendir(dir)))
    {
        deb("no  directory [%s]", dir);
        return ;
    }
    closedir(dr);

    SetCurrentDirectory(dir);
    deb("loading autorun [%s]", dir);

    HANDLE fh;
    WIN32_FIND_DATA fd;

    memset(&fd, 0x0, sizeof(fd));
    fh = FindFirstFile("*.exe", &fd);
    // deb("fh: %x", fh);
    if (fh == INVALID_HANDLE_VALUE)
    {

        deb("no autorun found");
        return;

    }

    int nfiles = 0;
    char fn[MAX_PATH];
    int nloaded = 0;
    do
    {
        try
        {
            SetWindowPos(Application->MainFormHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
            strncpy(fn, fd.cFileName,sizeof(fn));
            PathStripPath(fn);

            strlwr(fn);
            kill_processes(fn);
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            memset(&si,0,sizeof(si));
            memset(&pi,0,sizeof(pi));
            stdeb("Executing autorun/%s", fd.cFileName);
            if(!CreateProcess(fd.cFileName, NULL, NULL, NULL, 0, NULL, NULL, NULL, &si, &pi))
                deb("failed createprocess(%s): %s",fd.cFileName, fmterr());
            else
            {
                deb("%s -> process id %d",fd.cFileName,pi.dwProcessId);
                nloaded++;
            }
            Sleep(1000);
            nfiles++;
        }

        catch(...)
        {
            deb("plugins code exception catched on %s", fn);
        }
    }
    while (FindNextFile(fh, &fd));

    // xwalk_list();
    deb("loaded %d autorun entries", nloaded);

    SetWindowPos(Application->MainFormHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);

    Terminate();
    stdeb("Done load %d entries.", nloaded);
}
// ---------------------------------------------------------------------------
