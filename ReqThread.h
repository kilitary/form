//---------------------------------------------------------------------------

#ifndef ReqThreadH
#define ReqThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class ReqThread : public TThread
{
protected:
    void __fastcall Execute();
public:
    __fastcall ReqThread(bool CreateSuspended);
    void __fastcall Repaint(void);
    void __fastcall wndproc(Messages::TMessage & Message);
    void __fastcall Do(void);
};
//---------------------------------------------------------------------------
#endif
