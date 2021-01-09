#include <time.h>

class Timer{
public:
    void Initialize();

    float Deltatime(){return deltaTime;}
    float Totaltime(){return accumulatedTime;}

    void Update();
private:
    float accumulatedTime;
    float deltaTime;
    struct timeval lasttval;
};