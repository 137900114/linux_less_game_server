
#include "Server.h"
#include "Timer.h"

Timer gTimer;

int main(){
    Server server;
    gTimer.Initialize();
    if(!server.Bind(8000)){
        return -1;
    }

    server.MainLoop();

    server.Close();
}
