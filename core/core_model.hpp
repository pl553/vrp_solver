#pragma once

namespace Core {
    struct Point {
        float x;
        float y;
    };

    float EdgeCost(const Core::Point& a, const Core::Point& b);
}