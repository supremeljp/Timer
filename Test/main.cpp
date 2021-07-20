#include <iostream>
#include "../include/Timer.hpp"

void Func()
{
    std::cout << "Timer\n";
}

int main()
{
    Timer timer;
    timer.Start(1000, [](){
        std::cout << "Timer\n";
    });
    timer.Start(1000, Func);
    std::system("pause");
    timer.Stop();
    return 0;
}