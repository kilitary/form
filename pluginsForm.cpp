//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "pluginsForm.h"
#include "functions.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm7 *Form7;
//---------------------------------------------------------------------------
__fastcall TForm7::TForm7(TComponent* Owner)
    : TForm(Owner)
{
    deb("plugins form hwnd: %x", Handle);
}
//---------------------------------------------------------------------------
