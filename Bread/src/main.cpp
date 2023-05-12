#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/BehaviourTree.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <unistd.h>
#include <string>
#include "tasks/FarmingTasks.hpp"
#include "tasks/AdvancedTasks.hpp"
#include "AdvancedClient.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    AdvancedClient client;
    client.Connect("localhost", "Maurice");
    client.SetAutoRespawn(true);

    sleep(5);

    client.SetBehaviourTree(FarmingTasks::CreateTree());
    client.StartBehaviour();
    client.RunBehaviourUntilClosed();

    while (true)
    {
        sleep(1);
    }

    return 0;
}