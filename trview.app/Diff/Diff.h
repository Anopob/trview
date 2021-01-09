#pragma once

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
    };
}
