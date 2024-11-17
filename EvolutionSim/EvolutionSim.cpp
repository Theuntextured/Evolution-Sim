#include "Common.h"
#include "Engine.h"

#ifdef _DEBUG
int main()
#else
int WinMain()
#endif
{
    auto engine = new Engine;
    while(true)
    {
        if(!engine->tick())
            break;
    }
    delete engine;
    return 0;
}
