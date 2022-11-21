#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <unistd.h>
#include <string>
#include "BedwarsTasks.hpp"
#define BOT_COUNT 1000

using namespace std;

int main(int argc, char* argv[])
{
    Botcraft::SimpleBehaviourClient client(false);
    client.Connect("localhost", "Maurice");

    while (true)
    {
        sleep(1);
    }
        

    // Don't connect or anything because this is a minimal example
    return 0;
}