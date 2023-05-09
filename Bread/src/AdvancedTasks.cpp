#include "AdvancedTasks.hpp"
#include "botcraft/Game/Entities/entities/item/ItemEntity.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"

Botcraft::Status AdvancedTasks::DigAndCollect(AdvancedClient& client, const Position position, const std::string &item_name) {
    if (Botcraft::Dig(client, position) == Status::Failure) {
        LOG_WARNING("Error trying to break block at " << position);
        return Status::Failure;
    }

    for (int i = 0; i < 50; ++i) {
        client.Yield();
    }

    Position entity_position;
    int entity_id;
    vector<int> entities = client.findEntities(
        [item_name, &entity_position, &entity_id](const std::shared_ptr<Entity> entity) -> bool {
            if (entity->GetType() == EntityType::ItemEntity) {
                std::shared_ptr<ItemEntity> item = std::static_pointer_cast<ItemEntity>(entity);
#if PROTOCOL_VERSION < 340
                if (AssetsManager::getInstance().Items().at(item->GetDataItem().GetBlockID()).at(item->GetDataItem().GetItemDamage())->GetName() == item_name)
#else
                if (AssetsManager::getInstance().Items().at(item->GetDataItem().GetItemID())->GetName() == item_name)
#endif
                {
                    entity_position = entity->GetPosition();
                    entity_id = entity->GetEntityID();
                    return true;
                }
            }
            return false;
        }, 1);

    if (entities.size() != 1) {
        LOG_WARNING("Couldn't find drop for item "<<item_name<<" while digging block at "<<position<<"!");
        return Status::Failure;
    }

    //Pathfind to the drop

    if (GoTo(client, entity_position, 2) == Status::Failure) {
        LOG_WARNING("Error trying to pick up block drop (can't get close enough to " << entity_position << ")");
        return Status::Failure;
    }
    
    // Wait for item pickup

    auto start = std::chrono::steady_clock::now();
    while (true) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 5000) {
            LOG_WARNING("Error waiting for block drop pick-up (Timeout).");
            return Status::Failure;
        }

        if(client.getEntity(entity_id) == nullptr) {
            break;
        }

        client.Yield();
    }

    return Status::Success;
}