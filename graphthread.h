// ---------------------------------------------------------------------------

#ifndef graphthreadH
#define graphthreadH
// ---------------------------------------------------------------------------
#include <Classes.hpp>

typedef struct
{
    HWND hwnd;
    char title[128];
    TIcon *icon;
    int x;
    int y;
    TRect rect;
}WINDOWS_D;

typedef struct
{
    Graphics::TBitmap *ico;
    int idx;
    char tip[128];
    UINT id;
    TRect rect;
    HICON orighicon;
    Graphics::TBitmap *icon;
    NOTIFYICONDATAW pn;
    bool found;
}ICONS_D;

// ---------------------------------------------------------------------------
class DrawThread:public TThread
{
protected:
    void __fastcall Execute();

public:
    __fastcall DrawThread(bool CreateSuspended);
    void __fastcall DrawScreen(void);
    void __fastcall Paint(void);
    void __fastcall DrawBackground(TCanvas *cnv1);
    void __fastcall DrawStats(TCanvas *cnv);
    void __fastcall DrawWindows(TCanvas *cnv);          void __fastcall DrawVerbose(TCanvas *cnv);
    void __fastcall DrawIcons(TCanvas *cnv);
    void __fastcall DrawHint(TCanvas *cnv);
    void __fastcall DrawLines(TCanvas *cnv);
    void __fastcall DrawDockedDC(TCanvas *cnv);
    void __fastcall DrawButtons(TCanvas *cnv);
};

// ---------------------------------------------------------------------------
int WndWidth(void);

#endif
