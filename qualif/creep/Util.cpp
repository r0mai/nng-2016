#include "Util.hpp"

namespace sf {

std::ostream& operator<<(std::ostream& os, const Vector2i& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

} // namespace sf
