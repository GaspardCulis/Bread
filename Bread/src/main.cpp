#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <unistd.h>
#include <string>
#include "BedwarsTasks.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    Botcraft::SimpleBehaviourClient client(false);
    client.Connect("localhost", "Maurice");
    client.SetAutoRespawn(true);

    auto bedwars_tree = Botcraft::Builder<Botcraft::SimpleBehaviourClient>()
                                .sequence()
                                    .selector()
                                        .leaf([=](Botcraft::SimpleBehaviourClient& c) { return FindBed(c, 100); })
                                    .end()
                                    .leaf([](Botcraft::SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Botcraft::Status::Success; })
                                .end()
                                .build();


    client.SetBehaviourTree(bedwars_tree);
    
    client.RunBehaviourUntilClosed();

    while (true)
    {
        sleep(1);
    }
        

    // Don't connect or anything because this is a minimal example
    return 0;
}