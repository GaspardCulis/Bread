#include "AdvancedClient.hpp"

using namespace std::chrono;
using namespace Botcraft;

Vector3<double> AdvancedClient::getPosition() const {
    std::lock_guard<std::mutex> lock_localplayer(this->GetEntityManager()->GetLocalPlayer()->GetMutex());
    return this->GetEntityManager()->GetLocalPlayer()->GetPosition();
}

const Block* AdvancedClient::getBlock(Vector3<int> position) const {
    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());
    return world->GetBlock(position);
}

const shared_ptr<Entity> AdvancedClient::getEntity(int id) const {
    std::shared_ptr<EntityManager> entity_manager = this->GetEntityManager();
    std::lock_guard<std::mutex> lock_entity_manager(entity_manager->GetMutex());

    return entity_manager->GetEntity(id);
}

vector<Vector3<int>> AdvancedClient::findBlocks(std::function<bool(const Block *block)> match_function, int search_radius, int max_results) const {
    vector<Vector3<int>> out;


    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());

    Position my_pos = getPosition();

    const Vector3<int> min_pos(my_pos.x - search_radius, max(my_pos.y - search_radius, world->GetMinY()), my_pos.z - search_radius);
    const Vector3<int> max_pos(my_pos.x + search_radius, min(my_pos.y + search_radius, world->GetHeight()), my_pos.z + search_radius);
    Vector3<int> current;
    
    int nb_checks = 0;
    auto start = high_resolution_clock::now();
    current.y = min_pos.y;
    while(current.y <= max_pos.y) {
        current.x = min_pos.x;
        while(current.x <= max_pos.x) {
            current.z = min_pos.z;
            while(current.z <= max_pos.z) {
                const Block *block = world->GetBlock(current);
                nb_checks++;
                if ( block != nullptr && match_function(block) ) {
                    out.push_back(Vector3(current));
                    if (out.size() == max_results) {
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
    LOG_INFO("Checking " << nb_checks << " blocks took " << duration.count() << " milliseconds.");

    return out;
}

vector<Vector3<int>> AdvancedClient::findBlocks(const string block_name, int search_radius, int max_results) const {
    return this->findBlocks(
        [block_name](const Block *block) -> bool {
            return block->GetBlockstate()->GetName() == block_name;
        }, search_radius, max_results);
}

Vector3<int> AdvancedClient::findNearestBlock(std::function<bool(const Block *block)> match_function, int search_radius) const {
    vector<Vector3<int>> blocks = this->findBlocks(match_function, search_radius);
    sortPositionsFromClosest(blocks);
    if (blocks.size() > 0) {
        return blocks[0];
    } else {
        return NULL;
    }
}

Vector3<int> AdvancedClient::findNearestBlock(const string block_name, int search_radius) const {
    return this->findNearestBlock(
        [block_name](const Block *block) -> bool {
            return block->GetBlockstate()->GetName() == block_name;
        }, search_radius);
}

vector<int> AdvancedClient::findEntities(std::function<bool(const std::shared_ptr<Entity> entity)> match_function, int max_results) const {
    vector<int> out;

    std::shared_ptr<EntityManager> entity_manager = this->GetEntityManager();
    std::lock_guard<std::mutex> lock_entity_manager(entity_manager->GetMutex());

    for (const auto& e : entity_manager->GetEntities()) {
        if (match_function(e.second)) {
            out.push_back(e.first);
            if (out.size() == max_results) break;
        }
    }

    return out;
}

vector<int> AdvancedClient::findEntities(const EntityType entity_type, int max_results) const {
    return this->findEntities(
        [entity_type](const std::shared_ptr<Entity> entity) -> bool {
            return entity->GetType() == entity_type;
        }, max_results);
}

void AdvancedClient::sortPositionsFromClosest(vector<Vector3<int>> &positions) const {
    const Vector3<double> my_pos = getPosition();
    std::sort(positions.begin(), positions.end(), [my_pos](const Vector3<int> &a, const Vector3<int> &b) -> bool {
        return a.SqrDist(my_pos) < b.SqrDist(my_pos);
    });
}

AdvancedClient::AdvancedClient(const bool use_renderer_) : TemplatedBehaviourClient<AdvancedClient>(use_renderer_)
{

}

AdvancedClient::~AdvancedClient()
{

}

