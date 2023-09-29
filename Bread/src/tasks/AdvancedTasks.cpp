#include "tasks/AdvancedTasks.hpp"
#include "AdvancedClient.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/Game/Entities/entities/item/ItemEntity.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Utilities/Logger.hpp"

Botcraft::Status AdvancedTasks::CollectItem(AdvancedClient &client, int id)
{
    std::shared_ptr<EntityManager> entity_manager = client.GetEntityManager();

    // Wait for entity momentum to chill
    auto start = std::chrono::steady_clock::now();
    std::shared_ptr<Botcraft::Entity> e = client.getEntity(id);
    while (e != nullptr)
    {
        // Timeout
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 5000)
        {
            LOG_WARNING("Error trying to pick up block drop (Timeout)");
            return Status::Failure;
        }
        // Pathfind to the drop
        Vector3<double> entity_position = e->GetPosition();
        if (GoTo(client, entity_position, 1) == Status::Failure)
        {
            LOG_WARNING("Error trying to pick up block drop (can't get close enough to " << entity_position << ")");
            return Status::Failure;
        }
        client.Yield();
        e = client.getEntity(id);
    }
    return Status::Success;
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