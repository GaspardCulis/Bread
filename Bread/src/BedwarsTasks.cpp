#include "BedwarsTasks.hpp"

#include <botcraft/Game/World/World.hpp>
#include <botcraft/Game/Inventory/InventoryManager.hpp>
#include <botcraft/Game/Inventory/Window.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/Entities/entities/npc/VillagerEntity.hpp>
#include <botcraft/Game/Entities/entities/item/ItemEntity.hpp>
#include <botcraft/Game/AssetsManager.hpp>
#include <botcraft/Utilities/Logger.hpp>

#include <botcraft/AI/Tasks/AllTasks.hpp>

#include <protocolCraft/Types/NBT/TagCompound.hpp>
#include <protocolCraft/Types/NBT/TagList.hpp>
#include <protocolCraft/Types/NBT/TagString.hpp>

using namespace Botcraft;
using namespace ProtocolCraft;

Status FindBed(Botcraft::BehaviourClient& client) {
    std::shared_ptr<World> world = client.GetWorld();
    Blackboard& blackboard = client.GetBlackboard();

    Vector3<double> pos = client.GetEntityManager().get()->GetLocalPlayer().get()->GetPosition();
    Vector3<double> relative_pos(0, 0, 0);
    while (true) {
        
    }

}