// ---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH
// ---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <pngimage.hpp>

#include <vector>

// ---------------------------------------------------------------------------
class TForm6:public TForm
{
__published: // IDE-managed Components
    void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta,
          TPoint &MousePos, bool &Handled);
    void __fastcall FormKeyPress(TObject *Sender, wchar_t &Key);
    void __fastcall handleCustomMessage(TMessage &Message);


private: // User declarations
public: // User declarations
    __fastcall TForm6(TComponent *Owner);
    void __fastcall DrawScreen(void);

    BEGIN_MESSAGE_MAP
   	MESSAGE_HANDLER(WM_ERASEBKGND, TMessage, handleCustomMessage)
   // MESSAGE_HANDLER(WM_PAINT, TMessage, handleCustomMessage)

 END_MESSAGE_MAP(TForm)
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm6 *Form6;
void EnumerateIcons(void);
int WINAPI add_callback(LPVOID p, unsigned long size);
int WINAPI mod_callback(LPVOID p, unsigned long size);
int WINAPI del_callback(LPVOID p, unsigned long size);
void DrawStats(TCanvas *cnv);
// ---------------------------------------------------------------------------

typedef struct
{
    HANDLE wnd;
    HWND hwnd;
    HDC hdc;
    TCanvas *cnv;
    RECT rect;
    LPVOID onpaint;
    char dllpipe[128];
    int where;
    int width;
    int height;
    int left;
    int top;

}WND;

typedef std::vector<WND*>wnd_v;

typedef struct
{
    char cmd;
    LPVOID param;
    LPVOID param2;
    LPVOID addr;
    char dllpipe[128];
    RECT rect;
    Graphics::TBitmap *onhover;
    Graphics::TBitmap *pic;
    HANDLE id;
    HANDLE hproc;
    TColor transparentColor;
    char todelete;
    char visible;
    int idx;
    Graphics::TBitmap *ibmp;
    RECT irect;
    HBITMAP hbmp;
    char reg_wnd_done;
    int opacity;
}WNDBUTTONS;

typedef struct
{
    char dllpipe[128];
    char vkey;
    LPVOID addr;
} KEYS;

typedef std::vector<WNDBUTTONS*>wnd_buttons_v;
typedef std::vector<KEYS*>keys_v;
#endif
