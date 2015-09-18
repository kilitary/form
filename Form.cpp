//---------------------------------------------------------------------------
#define STRSAFE_NO_DEPRECATE
#include <vcl.h>
#pragma hdrstop
#pragma link "madExcept"
#pragma link "madLinkDisAsm"
#include <tchar.h>
HINSTANCE hInstance;
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", Form6);
//---------------------------------------------------------------------------
WINAPI _tWinMain(HINSTANCE hi, HINSTANCE, LPTSTR, int)
{
    hInstance=hi;
    try
    {
         Application->Initialize();
         Application->MainFormOnTaskBar = true;
         Application->CreateForm(__classid(TForm6), &Form6);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
