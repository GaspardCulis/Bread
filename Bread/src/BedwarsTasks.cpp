#include "BedwarsTasks.hpp"
#include <algorithm> 
#include <iostream>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Game/Inventory/InventoryManager.hpp>
#include <botcraft/Game/Inventory/Window.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/Entities/entities/npc/VillagerEntity.hpp>
#include <botcraft/Game/Entities/entities/item/ItemEntity.hpp>
#include <botcraft/Game/AssetsManager.hpp>
#include <botcraft/Utilities/Logger.hpp>
#include <unistd.h>

#include <botcraft/AI/Tasks/AllTasks.hpp>

#include <protocolCraft/Types/NBT/TagCompound.hpp>
#include <protocolCraft/Types/NBT/TagList.hpp>
#include <protocolCraft/Types/NBT/TagString.hpp>


using namespace std;
using namespace Botcraft;
using namespace ProtocolCraft;

Status FindBed(Botcraft::BehaviourClient& client, int search_radius) {
    cout << "Finding bed ..." << endl;
    sleep(5);
    std::shared_ptr<World> world = client.GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());
    Blackboard& blackboard = client.GetBlackboard();

    const string suffix = "_bed";

    Position my_pos;
    {
        std::lock_guard<std::mutex> lock_localplayer(client.GetEntityManager()->GetLocalPlayer()->GetMutex());
        my_pos = client.GetEntityManager()->GetLocalPlayer()->GetPosition();
    }

    cout << "Bot position is " << my_pos << endl;
    Vector3<int> check_pos;
    Vector3<int> max_pos(my_pos.x +search_radius, world->GetHeight(), my_pos.z + search_radius);
    check_pos.y = world->GetMinY();
    while (check_pos.y < max_pos.y) {
        check_pos.x = my_pos.x - search_radius;
        while(check_pos.x < max_pos.x) {
            check_pos.z = my_pos.z - search_radius;
            while (check_pos.z < max_pos.z) {
                //cout << "Checking " << pos + relative_pos << endl;
                const Block *block = world->GetBlock(check_pos);
                if (block) {
                    const string& block_name = block->GetBlockstate()->GetName();
                    if (block_name.size() >= suffix.size() && 0 == block_name.compare(block_name.size()-suffix.size(), suffix.size(), suffix)) {
                        cout << "Found bed at " << check_pos << " ! " << endl;
                        return Status::Success;
                    } 
                }
                check_pos.z += 1;
            }
            check_pos.x += 1;
        }
        check_pos.y += 1;
    }

    return Status::Failure;

}