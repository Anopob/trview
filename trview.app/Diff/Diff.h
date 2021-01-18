#pragma once

#include <cstdint>
#include <vector>
#include <trview.app/Elements/Item.h>
#include <trview.app/Elements/Trigger.h>

namespace trview
{
    struct Diff
    {
        struct Change
        {
            enum class Type
            {
                Add,
                Edit,
                Delete
            };

            enum class Subject
            {
                Item,
                Trigger
            };

            uint32_t number{ 0u };
            Type type{ Type::Add };
            Subject subject{ Subject::Item };
            uint32_t index{ 0 };
        };

        std::vector<Change> changes;
        std::vector<Item> left_items;
        std::vector<Item> right_items;
        std::vector<const Trigger*> left_triggers;
        std::vector<const Trigger*> right_triggers;
    };
}
