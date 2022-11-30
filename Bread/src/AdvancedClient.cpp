#include "AdvancedClient.hpp"
#include <chrono>

using namespace std::chrono;
using namespace Botcraft;

vector<Vector3<int>> AdvancedClient::findBlocks(bool(*match_function)(const Block *block), int search_radius, int max_blocks) const {
    vector<Vector3<int>> out;

    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());

    Position my_pos;
    {
        std::lock_guard<std::mutex> lock_localplayer(this->GetEntityManager()->GetLocalPlayer()->GetMutex());
        my_pos = this->GetEntityManager()->GetLocalPlayer()->GetPosition();
    }

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
                    if (out.size() == max_blocks) {
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
    LOG_INFO("Checking " << nb_checks << " blocks took " << duration.count() << " milliseconds." << endl);

    return out;
}

AdvancedClient::AdvancedClient(const bool use_renderer_) : TemplatedBehaviourClient<AdvancedClient>(use_renderer_)
{

}

AdvancedClient::~AdvancedClient()
{

}

