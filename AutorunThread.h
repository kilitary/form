//---------------------------------------------------------------------------

#ifndef AutorunThreadH
#define AutorunThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class AutorunThread : public TThread
{
protected:
    void __fastcall Execute();
public:
    __fastcall AutorunThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
