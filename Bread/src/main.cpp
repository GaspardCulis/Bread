#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <unistd.h>
#include <string>
#include "BedwarsTasks.hpp"
#include "AdvancedClient.hpp"

using namespace std;

bool match_bed(const Block *block) {
    const string suffix = "_bed";
    const string& block_name = block->GetBlockstate()->GetName();
    return block_name.size() >= suffix.size() && 0 == block_name.compare(block_name.size()-suffix.size(), suffix.size(), suffix);
}

int main(int argc, char* argv[])
{
    AdvancedClient client;
    client.Connect("localhost", "Maurice");
    client.SetAutoRespawn(true);

    sleep(5);

    Vector3<int> block = client.findNearestBlock("minecraft:brewing_stand", 256);

    client.SendChatCommand("tp " + to_string(block.x) + " " + to_string(block.y) + " " + to_string(block.z));


    // Match white_stained_glass
    cout << "Goal reached" << endl;

    while (true)
    {
        sleep(1);
    }
        

    // Don't connect or anything because this is a minimal example
    return 0;
}