//---------------------------------------------------------------------------

#ifndef TimedThreadH
#define TimedThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class TimedThread : public TThread
{
protected:
    void __fastcall Execute();
public:
    __fastcall TimedThread(bool CreateSuspended);
    void __fastcall Paint(void);
                                void __fastcall Do(void);
};
//---------------------------------------------------------------------------
#endif
