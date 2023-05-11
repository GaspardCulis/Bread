#include "AdvancedTasks.hpp"
#include "botcraft/Game/Entities/entities/item/ItemEntity.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"

Botcraft::Status AdvancedTasks::CollectItem(AdvancedClient &client, int id)
{
    std::shared_ptr<EntityManager> entity_manager = client.GetEntityManager();

    Vector3<double> entity_position = client.getEntity(id)->GetPosition();

    // Pathfind to the drop
    if (GoTo(client, entity_position, 2) == Status::Failure)
    {
        LOG_WARNING("Error trying to pick up block drop (can't get close enough to " << entity_position << ")");
        return Status::Failure;
    }

    // Wait for item pickup
    auto start = std::chrono::steady_clock::now();
    while (true)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 4000)
        {
            LOG_WARNING("Error waiting for block drop pick-up (Timeout).");
            return Status::Failure;
        }

        if (client.getEntity(id) == nullptr)
        {
            break;
        }

        client.Yield();
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

    for (int i = 0; i < 50; ++i)
    {
        client.Yield();
    }

    std::set<int> entities = client.findEntities(
        [&position](const std::shared_ptr<Entity> entity) -> bool
        {
            if (entity->GetType() == EntityType::ItemEntity)
            {
                std::shared_ptr<ItemEntity> item = std::static_pointer_cast<ItemEntity>(entity);
                if (position.SqrDist(item->GetPosition()) <= 2)
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