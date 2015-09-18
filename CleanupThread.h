//---------------------------------------------------------------------------

#ifndef CleanupThreadH
#define CleanupThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class CleanupThread : public TThread
{
protected:
    void __fastcall Execute();
public:
    __fastcall CleanupThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
