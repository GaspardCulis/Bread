#include "tasks/SurvivalTasks.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Utilities/Logger.hpp"

// https://minecraft.fandom.com/wiki/Food
const std::vector<std::string> edible_items({"minecraft:rabbit_stew",
                                             "minecraft:cooked_porkchop",
                                             "minecraft:cooked_beef",
                                             "minecraft:golden_carrot",
                                             "minecraft:cooked_mutton",
                                             "minecraft:cooked_salmon",
                                             // "minecraft:echanted_golden_apple", // Not gonna eat that right?
                                             "minecraft:beetroot_soup",
                                             "minecraft:cooked_chicken",
                                             "minecraft:mushroom_stew",
                                             "minecraft:suspicious_stew",
                                             "minecraft:pumpkin_pie",
                                             "minecraft:bread",
                                             "minecraft:baked_potato",
                                             "minecraft:cooked_cod",
                                             "minecraft:cooked_rabbit",
                                             "minecraft:honey_bottle",
                                             "minecraft:carrot",
                                             "minecraft:apple",
                                             "minecraft:chorus_fruit",
                                             "minecraft:raw_beef",
                                             "minecraft:raw_porkchop",
                                             "minecraft:raw_rabbit",
                                             "minecraft:rotten_flesh",
                                             "minecraft:melon_slice",
                                             "minecraft:poisonous_potato",
                                             "minecraft:raw_chicken",
                                             "minecraft:raw_mutton",
                                             "minecraft:cookie",
                                             "minecraft:glow_berries",
                                             "minecraft:raw_cod",
                                             "minecraft:raw_salmon",
                                             "minecraft:sweet_berries",
                                             "minecraft:beetroot",
                                             "minecraft:dried_kelp",
                                             "minecraft:potato",
                                             "minecraft:pufferfish",
                                             "minecraft:tropical_fish"});

Botcraft::Status SurvivalTasks::InitializeBlocks(AdvancedClient &client, const int search_radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    /*
    auto _ = client.findBlocks(
        [&b](const Blockstate *block, const Position position, std::shared_ptr<World> world) -> bool
        {
            Position down = position + Position(0, -1, 0);
            const std::string &block_name = block->GetName();
            const std::string bed_suffix = "_bed";

            if (
                block_name.size() >= bed_suffix.size() &&
                0 == block_name.compare(block_name.size() - bed_suffix.size(), bed_suffix.size(), bed_suffix)
            ) {
                b.Set("SurvivalTasks.bed_pos", position);
                LOG_INFO(block_name << " found at: " << position << "!");
            }

            return false;
        },
        search_radius);
    */

    b.Set("SurvivalTasks.initialized", true);
    LOG_INFO("Blocks initialized");

    return Status::Success;
}

Botcraft::Status SurvivalTasks::FindBestFoodInInventory(AdvancedClient &client)
{
    int i = 0;
    while (i < edible_items.size() && HasItemInInventory(client, edible_items[i]) == Status::Failure)
    {
        i++;
    }
    if (i == edible_items.size())
    {
        LOG_WARNING("[FindBestFoodInInventory] No edible food found in inventory");
        return Status::Failure;
    }

    Botcraft::Blackboard &b = client.GetBlackboard();
    b.Set<std::string>("SurvivalTasks.best_food_in_inventory", edible_items[i]);

    return Status::Success;
}

// TODO: Fix save bed but different block issue
Botcraft::Status SurvivalTasks::FindUniqueBedBlackboard(AdvancedClient &client, const std::string blackboard_key) {
    Botcraft::Blackboard &b = client.GetBlackboard();

    const int id = b.Get("id", 0);

    auto beds = client.findBlocks(
        [](const Blockstate *block, const Position position, std::shared_ptr<World> world) -> bool
        {
            Position down = position + Position(0, -1, 0);
            const std::string &block_name = block->GetName();
            const std::string bed_suffix = "_bed";

            if (
                block_name.size() >= bed_suffix.size() &&
                0 == block_name.compare(block_name.size() - bed_suffix.size(), bed_suffix.size(), bed_suffix)
            ) {
                return true;
            }

            return false;
        }
    );

    if (beds.size() == 0) {
        LOG_WARNING("[FindUniqueBedBlackboard] No bed found");
        return Status::Failure;
    }

    std::sort(beds.begin(), beds.end());

    Position bed_pos = beds[id % beds.size()];

    LOG_INFO("[FindUniqueBedBlackboard] Found bed at " << bed_pos << " for unique id " << id);

    b.Set(blackboard_key, bed_pos);

    return Status::Success;
}

// Absolutely not stolen code...
std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateSleepTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>("sleep")
        .selector()
            // If it's night
            .inverter().leaf("check if night", Botcraft::IsNightTime)
            .sequence()
                // Go to the bed
                .leaf(Botcraft::CopyBlackboardData, "SurvivalTasks.bed_pos", "GoTo.goal") // See? Not the same
                .leaf(Botcraft::SetBlackboardData<int>, "GoTo.dist_tolerance", 2)
                .leaf("go to bed", Botcraft::GoToBlackboard)
                // Right click the bed every second until it's day time
                .repeater(0).sequence()
                    .leaf(Botcraft::CopyBlackboardData, "SurvivalTasks.bed_pos", "InteractWithBlock.pos")
                    .leaf(Botcraft::SetBlackboardData<bool>, "InteractWithBlock.animation", true)
                    .leaf("interact with bed", Botcraft::InteractWithBlockBlackboard)
                    // Wait ~1s
                    .repeater(100).leaf(Botcraft::Yield)
                    .leaf("check if night", Botcraft::IsNightTime)
                .end()
            .end()
        .end();
    // clang-format on
}

// Same
std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateEatTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>("eat")
        .selector()
            // If hungry
            .inverter().leaf("check is hungry", Botcraft::IsHungry, 20)
            // Go buy some food, then eat
            .inverter()
            .sequence()
                .leaf(SurvivalTasks::FindBestFoodInInventory)
                .leaf(Botcraft::CopyBlackboardData, "SurvivalTasks.best_food_in_inventory", "Eat.food_name")
                .leaf(Botcraft::SetBlackboardData<bool>, "Eat.wait_confirmation", true)
                .leaf("eat", Botcraft::EatBlackboard)
            .end()
        .end();
    // clang-format on
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> SurvivalTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SurvivalTasks.initialized")
                .sequence()
                    .leaf(SurvivalTasks::InitializeBlocks, 128)
                    .leaf(SurvivalTasks::FindUniqueBedBlackboard, "SurvivalTasks.bed_pos")
                .end()
            .end()
            .tree(CreateSleepTree())
            .tree(CreateEatTree())
        .end();
    // clang-format on
}