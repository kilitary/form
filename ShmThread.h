//---------------------------------------------------------------------------

#ifndef ShmThreadH
#define ShmThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class ShmThread : public TThread
{
protected:
    void __fastcall Execute();
public:
    __fastcall ShmThread(bool CreateSuspended);
    void __fastcall Do(void);
};
//---------------------------------------------------------------------------
#endif
