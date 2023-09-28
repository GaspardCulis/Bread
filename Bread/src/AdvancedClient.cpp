#include "AdvancedClient.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Item.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include <string>

using namespace std::chrono;
using namespace Botcraft;

Vector3<double> AdvancedClient::getPosition() const
{
    /*Vector3<double> my_pos;
    {
        std::lock_guard<std::mutex> lock_localplayer(this->GetEntityManager()->GetLocalPlayer()->GetMutex());
        my_pos = this->GetEntityManager()->GetLocalPlayer()->GetPosition();
    }*/
    return this->GetEntityManager()->GetLocalPlayer()->GetPosition();
}

const Block *AdvancedClient::getBlock(Vector3<int> position) const
{
    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());
    return world->GetBlock(position);
}

const shared_ptr<Entity> AdvancedClient::getEntity(int id) const
{
    std::shared_ptr<EntityManager> entity_manager = this->GetEntityManager();
    std::lock_guard<std::mutex> lock_entity_manager(entity_manager->GetMutex());

    return entity_manager->GetEntity(id);
}

vector<Vector3<int>> AdvancedClient::findBlocks(std::function<bool(const Block *block, const Position position, std::shared_ptr<World> world)> match_function, const int search_radius, const int max_results, const std::optional<Position> origin) const
{
    vector<Vector3<int>> out;

    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());

    Position origin_pos = origin.value_or(getPosition());

    const Vector3<int> min_pos(origin_pos.x - search_radius, max(origin_pos.y - search_radius, world->GetMinY()), origin_pos.z - search_radius);
    const Vector3<int> max_pos(origin_pos.x + search_radius, min(origin_pos.y + search_radius, world->GetHeight()), origin_pos.z + search_radius);
    Vector3<int> current;

    int nb_checks = 0;
    auto start = high_resolution_clock::now();
    current.y = min_pos.y;

    while (current.y <= max_pos.y)
    {
        current.x = min_pos.x;
        while (current.x <= max_pos.x)
        {
            current.z = min_pos.z;
            while (current.z <= max_pos.z)
            {
                const Block *block = world->GetBlock(current);
                nb_checks++;
                if (block != nullptr && match_function(block, (Position)current, world))
                {
                    out.push_back(Vector3(current));
                    if (out.size() == max_results)
                    {
                        goto end;
                    }
                }
                current.z++;
            }
            current.x++;
        }
        current.y++;
    }
end:
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    LOG_DEBUG("Checking " << nb_checks << " blocks took " << duration.count() << " milliseconds.");
    cout.flush();

    return out;
}

vector<Vector3<int>> AdvancedClient::findBlocks(const string block_name, const int search_radius, const int max_results, const std::optional<Position> origin) const
{
    return this->findBlocks(
        [block_name](const Block *block, const Position _, std::shared_ptr<World> __) -> bool
        {
            return block->GetBlockstate()->GetName() == block_name;
        },
        search_radius, max_results, origin);
}

Vector3<int> AdvancedClient::findNearestBlock(std::function<bool(const Block *block, const Position position, const std::shared_ptr<World> world)> match_function, const int search_radius, const std::optional<Position> origin) const
{
    vector<Vector3<int>> blocks = this->findBlocks(match_function, search_radius, 10, origin);
    sortPositionsFromClosest(blocks, origin.value_or(getPosition()));
    if (blocks.size() > 0)
    {
        return blocks[0];
    }
    else
    {
        throw std::range_error("Couldn't find requested block");
    }
}

Vector3<int> AdvancedClient::findNearestBlock(const string block_name, const int search_radius, const std::optional<Position> origin) const
{
    return this->findNearestBlock(
        [block_name](const Block *block, const Position _, std::shared_ptr<World> __) -> bool
        {
            return block->GetBlockstate()->GetName() == block_name;
        },
        search_radius, origin);
}

std::set<int> AdvancedClient::findEntities(std::function<bool(const std::shared_ptr<Entity> entity)> match_function, int max_results) const
{
    std::set<int> out;

    std::shared_ptr<EntityManager> entity_manager = this->GetEntityManager();
    std::lock_guard<std::mutex> lock_entity_manager(entity_manager->GetMutex());

    for (const auto &e : entity_manager->GetEntities())
    {
        if (match_function(e.second))
        {
            out.insert(e.first);
            if (out.size() == max_results)
                break;
        }
    }

    return out;
}

std::set<int> AdvancedClient::findEntities(const EntityType entity_type, int max_results) const
{
    return this->findEntities(
        [entity_type](const std::shared_ptr<Entity> entity) -> bool
        {
            return entity->GetType() == entity_type;
        },
        max_results);
}

short AdvancedClient::getItemSlotInInventory(std::function<bool(short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item)> match_function)
{
    std::shared_ptr<InventoryManager> inventory_manager = this->GetInventoryManager();
    std::lock_guard<std::mutex> inventory_lock(inventory_manager->GetMutex());

    const std::map<short, ProtocolCraft::Slot> &slots = inventory_manager->GetPlayerInventory()->GetSlots();
    for (auto it = slots.begin(); it != slots.end(); ++it)
    {
        if (!it->second.IsEmptySlot())
        {
#if PROTOCOL_VERSION < 347
            auto item = AssetsManager::getInstance().Items().at(it->second.GetBlockID()).at(static_cast<unsigned char>(it->second.GetItemDamage())).get();
#else
            auto item = AssetsManager::getInstance().Items().at(it->second.GetItemID()).get();
#endif
            if (match_function(it->first, it->second, item))
            {
                return it->first;
            }
        }
    }

    return -1;
}

short AdvancedClient::getItemSlotInInventory(const std::string item_name)
{
    return getItemSlotInInventory(
        [item_name](short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item) -> bool
        {
            return item->GetName() == item_name;
        });
}

int AdvancedClient::getItemCountInInventory(std::function<bool(short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item)> match_function)
{
    int total = 0;
    
    std::shared_ptr<InventoryManager> inventory_manager = this->GetInventoryManager();
    std::lock_guard<std::mutex> inventory_lock(inventory_manager->GetMutex());

    const std::map<short, ProtocolCraft::Slot> &slots = inventory_manager->GetPlayerInventory()->GetSlots();
    for (auto it = slots.begin(); it != slots.end(); ++it)
    {
        if (!it->second.IsEmptySlot())
        {
#if PROTOCOL_VERSION < 347
            auto item = AssetsManager::getInstance().Items().at(it->second.GetBlockID()).at(static_cast<unsigned char>(it->second.GetItemDamage())).get();
#else
            auto item = AssetsManager::getInstance().Items().at(it->second.GetItemID()).get();
#endif
            if (match_function(it->first, it->second, item))
            {
                total += it->second.GetItemCount();
            }
        }
    }

    return total;
}

int AdvancedClient::getItemCountInInventory(const std::string item_name)
{
    return getItemCountInInventory(
        [item_name](short slitId, ProtocolCraft::Slot current_slot, Botcraft::Item *item) -> bool
        {
            return item->GetName() == item_name;
        });
}

bool AdvancedClient::sendOTM(const std::string message, const std::optional<std::string> message_identifier)
{
    std::size_t msg_hash = std::hash<std::string>{}(message_identifier.value_or(message));

    Blackboard &b = this->GetBlackboard();
    char buffer[100];
    sprintf(buffer, "OTM.%ld", msg_hash);
    if (!b.Get<bool>(buffer, false))
    {
        this->SendChatMessage(message);
        b.Set<bool>(buffer, true);
        return true;
    }
    return false;
}

void AdvancedClient::resetOTM(const std::string message_identifier)
{
    std::size_t msg_hash = std::hash<std::string>{}(message_identifier);

    Blackboard &b = this->GetBlackboard();
    char buffer[100];
    sprintf(buffer, "OTM.%ld", msg_hash);
    b.Erase(buffer);
}

void AdvancedClient::sortPositionsFromClosest(vector<Vector3<int>> &positions, const Vector3<double> origin) const
{
    std::sort(positions.begin(), positions.end(), [origin](const Vector3<int> &a, const Vector3<int> &b) -> bool
              { return a.SqrDist(origin) < b.SqrDist(origin); });
}

AdvancedClient::AdvancedClient(const bool use_renderer_) : TemplatedBehaviourClient<AdvancedClient>(use_renderer_)
{
}

AdvancedClient::~AdvancedClient()
{
}
