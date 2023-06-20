#include "core_model.hpp"

#include <cmath>

namespace Core {
    float EdgeCost(const Core::Point& a, const Core::Point& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }
}