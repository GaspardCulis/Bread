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
    // Match white_stained_glass
    const vector<Vector3<int>> blocks = client.findBlocks(
        [](const Block *block) {
            return block->GetBlockstate()->GetName() == "minecraft:white_stained_glass";
        }, 4);
    client.sortPositionsFromNearest(blocks);
    for (Vector3<int> pos : blocks) {
        //cout << "Bed is : " << pos << endl;
    }

    while (true)
    {
        sleep(1);
    }
        

    // Don't connect or anything because this is a minimal example
    return 0;
}