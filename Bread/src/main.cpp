#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <unistd.h>
#include <string>
#include "BedwarsTasks.hpp"
#include "AdvancedClient.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    AdvancedClient client;
    client.Connect("localhost", "Maurice");
    client.SetAutoRespawn(true);

    sleep(5);

    client.SetBehaviourTree(createTree());
    client.StartBehaviour();
    client.RunBehaviourUntilClosed();

    while (true)
    {
        sleep(1);
    }
        

    // Don't connect or anything because this is a minimal example
    return 0;
}