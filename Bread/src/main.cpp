#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/BehaviourTree.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <unistd.h>
#include <string>
#include "tasks/SkyblockTasks.hpp"
#include "AdvancedClient.hpp"

using namespace std;

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();

int main(int argc, char *argv[])
{
    AdvancedClient client;
    client.Connect("127.0.0.1:25565", "Maurice");
    client.SetAutoRespawn(true);

    sleep(5);

    client.SetBehaviourTree(CreateTree());
    client.StartBehaviour();
    client.RunBehaviourUntilClosed();

    while (true)
    {
        sleep(1);
    }

    return 0;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .tree(SkyblockTasks::CreateTree())
        .end();
    // clang-format on
}