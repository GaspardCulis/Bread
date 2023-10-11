#include "tasks/AdvancedTasks.hpp"
#include "AdvancedClient.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/AI/Tasks/PathfindingTask.hpp"
#include "botcraft/Game/Entities/entities/Entity.hpp"
#include "botcraft/Game/Entities/entities/item/ItemEntity.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/Blockstate.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "protocolCraft/Types/NBT/NBT.hpp"
#include "protocolCraft/Types/NBT/Tag.hpp"
#include "protocolCraft/Utilities/Json.hpp"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <unordered_map>

Botcraft::Status AdvancedTasks::CollectItem(AdvancedClient &client, int id)
{
    std::shared_ptr<Botcraft::Entity> e = client.getEntity(id);
    // Go to the target once
    if (e != nullptr) 
    {
        const Vector3<double> entity_position = e->GetPosition();
        if (GoTo(client, entity_position, 1) == Status::Failure)
        {
            return Status::Failure;
        }
        e = client.getEntity(id);
    }
    // Retry while not picked up for 5s
    auto start = std::chrono::steady_clock::now();
    while (e != nullptr)
    {
        const Vector3<double> entity_position = e->GetPosition();
        if (GoTo(client, entity_position, 1) == Status::Failure)
        {
            return Status::Failure;
        }
        client.Yield();
        // Timeout
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 5000)
        {
            LOG_WARNING("Error trying to pick up block drop (Timeout)");
            return Status::Failure;
        }
        // Pathfind to the drop
        e = client.getEntity(id);
    }
    return Status::Success;
}

Botcraft::Status AdvancedTasks::CollectItems(AdvancedClient &client, std::function<bool(const int entity_id, std::shared_ptr<Botcraft::Entity> entity)> match_function, const unsigned int collect_radius)
{
    std::unordered_map<int, std::shared_ptr<Botcraft::Entity>> entities;
    {
        std::shared_ptr<EntityManager> entity_manager = client.GetEntityManager();
        std::lock_guard<std::mutex> lock_entity_manager(entity_manager->GetMutex());
        entities = entity_manager->GetEntities();
    }

    std::vector<int> to_be_collected;
    // Finds the items that need to bee collected
    for (auto e : entities) {
        if ((collect_radius == 0 || sqrt(e.second->GetPosition().SqrDist(client.getPosition())) <= collect_radius) && match_function(e.first, e.second)) {
            to_be_collected.push_back(e.first);
        }
    }
    // Collects them
    for (int item_id : to_be_collected) {
        CollectItem(client, item_id);
    }

    return Status::Success;
}

Botcraft::Status AdvancedTasks::CollectItems(AdvancedClient &client, const std::string &item_name, const unsigned int collect_radius)
{
    const auto item_id = AssetsManager::getInstance().GetItemID(item_name);
    return CollectItems(client, [item_id](const int entity_id, std::shared_ptr<Botcraft::Entity> entity) -> bool
    {
        if (entity->GetType() == EntityType::ItemEntity) 
        {
            std::shared_ptr<ItemEntity> item = std::static_pointer_cast<ItemEntity>(entity);
            if (item->GetDataItem().GetItemID() == item_id)
            {
                return true;
            }
        }   
        return false;
    }, collect_radius);
}

Botcraft::Status AdvancedTasks::DigAndCollect(AdvancedClient &client, const Position position)
{
    if (Botcraft::Dig(client, position, true) == Status::Failure)
    {
        LOG_WARNING("Error trying to break block at " << position);
        return Status::Failure;
    }
    client.Yield();
    std::set<int> entities = client.findEntities(
        [&position](const std::shared_ptr<Entity> entity) -> bool
        {
            if (entity->GetType() == EntityType::ItemEntity)
            {
                std::shared_ptr<ItemEntity> item = std::static_pointer_cast<ItemEntity>(entity);
                if (position.SqrDist(item->GetPosition()) <= 8)
                {
                    return true;
                }
            }
            return false;
        },
        1);

    if (entities.size() != 1)
    {
        LOG_WARNING("Couldn't find drop after breaking block at " << position << "!");
        return Status::Failure;
    }

    // Collect the drops
    for (int eid : entities)
    {
        if (AdvancedTasks::CollectItem(client, eid) == Status::Failure)
        {
            LOG_WARNING("Failed to collect item drop after digging block at " << position);
            return Status::Failure;
        }
    }

    return Status::Success;
}

Botcraft::Status StoreSingleItemInContainer(AdvancedClient &client, const std::string &item_name, std::shared_ptr<Window> container, short container_id)
{
    std::shared_ptr<InventoryManager> inventory_manager = client.GetInventoryManager();
        
    short dst_slot = -1;
    short src_slot = -1;
    
    {
        std::lock_guard<std::mutex> lock_inventory_manager(inventory_manager->GetMutex());
        const auto item_id = AssetsManager::getInstance().GetItemID(item_name);

        for (const auto &s : container->GetSlots())
        {
            if (
                dst_slot == -1 && s.first < container->GetFirstPlayerInventorySlot() && // If dst slot not found and not in player inventory
                (
                    s.second.IsEmptySlot() || // If empty or same item and enough space
                    (
#if PROTOCOL_VERSION < 340 /* < 1.12.2 */
                        s.second.GetBlockID() == item_id.first && s.second.GetItemDamage() == item_id.second
#else
                        s.second.GetItemID() == item_id
#endif
                        && s.second.GetItemCount() < AssetsManager::getInstance().Items().at(item_id)->GetStackSize()
                    )
                )
            ) {
                dst_slot = s.first;
            }
            else if (
                src_slot == -1 && s.first >= container->GetFirstPlayerInventorySlot() &&
#if PROTOCOL_VERSION < 340 /* < 1.12.2 */
                s.second.GetBlockID() == item_id.first && s.second.GetItemDamage() == item_id.second
#else
                s.second.GetItemID() == item_id
#endif
            )
            {
                src_slot = s.first;
            }
            else if (src_slot != -1 && dst_slot != -1)
            {
                break;
            }                
        }
    }

    // No dispenser in inventory, nothing to do
    if (src_slot == -1)
    {
        LOG_WARNING("No " << item_name << " in inventory");
        return Status::Failure;
    }

    if (dst_slot == -1)
    {
        LOG_WARNING("Can't find a place for " << item_name << " in the container");
        return Status::Failure;
    }

    if (PutOneItemInContainerSlot(client, container_id, src_slot, dst_slot) == Status::Failure)
    {
        LOG_WARNING("Error trying to transfer " << item_name << " into the container");
        return Status::Failure;
    }

    return Status::Success;
}

Botcraft::Status AdvancedTasks::EnsureItemCount(AdvancedClient &client, const Position storage_pos, const std::string &item_name, const int min, const int max)
{
    const int count = client.getItemCountInInventory(item_name);
    if (count >= min && count <= max) {
        return Botcraft::Status::Success;
    }

    std::shared_ptr<InventoryManager> inventory_manager = client.GetInventoryManager();

    if (OpenContainer(client, storage_pos) == Status::Failure)
    {
        LOG_WARNING("Can't open output chest to store items");
        return Status::Failure;
    }
    for (int i = 0; i < 100; ++i)
    {
        client.Yield();
    }

    short container_id;
    std::shared_ptr<Window> container;
    {
        std::lock_guard<std::mutex> lock_inventory_manager(inventory_manager->GetMutex());
        container_id = inventory_manager->GetFirstOpenedWindowId();
        container = inventory_manager->GetWindow(container_id);
    }

    if (count > max) {
        for (int i = 0; i < count - min; i++) {
            if (StoreSingleItemInContainer(client, item_name, container, container_id) == Status::Failure) {
                break;
            }
            client.Yield();        
        }   
    }
    

    // TODO Item retreaving

    CloseContainer(client);

    return Status::Failure;
}

Botcraft::Status AdvancedTasks::FindNearestBlockBlackboard(AdvancedClient &client, const std::string block_name, const std::string blackboard_key) {
    try {
        Position block_pos = client.findNearestBlock(block_name);
        client.GetBlackboard().Set(blackboard_key, block_pos);
    } catch (std::range_error) {
        return Status::Failure;
    }

    return Status::Success;
}

Botcraft::Status AdvancedTasks::FindNamedChestBlackboard(AdvancedClient &client, const std::string text, const std::string blackboard_key) {
    try {
        auto chest_pos = client.findNearestBlock(
            [text](const Blockstate *block, Position position, std::shared_ptr<World> world) -> bool
            {
                if (block->GetName() == "minecraft:chest") {
                    const std::string suffix = "_wall_sign";
                    // Check all places where a sign can be placed on the chest
                    for (auto offset : {
                        Position(0, 0, 1),
                        Position(1, 0, 0),
                        Position(0, 0, -1),
                        Position(-1, 0, 0),
                    }) {
                        const std::string block_name = world->GetBlock(offset + position)->GetName();
                        // If is a sign of any type
                        if (block_name.size() >= suffix.size() && 
                            block_name.compare(block_name.size() - suffix.size(), suffix.size(), suffix) == 0
                        ) {
                            const auto ed = world->GetBlockEntityData(position + offset);
                            const auto messages = ed["front_text"]["messages"];
                            for (auto line : messages.as_list_of<ProtocolCraft::NBT::TagString>()) {
                                const ProtocolCraft::Json::Value content = ProtocolCraft::Json::Parse(line);
                                if (content.is_object() && 
                                    content.contains("text") &&
                                    content["text"].get_string() == text 
                                ) {
                                    return true;
                                }
                            }
                        }
                    }
                }
                return false;
            }
        );

        client.GetBlackboard().Set(blackboard_key, chest_pos);

        LOG_INFO("Chest having " << text << " text found at " << chest_pos);
        return Status::Success;
    } catch (std::range_error) {
        return Status::Failure;
    }
    
}

