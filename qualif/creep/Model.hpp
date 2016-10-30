#include <boost/variant.hpp>
#include <boost/multi_array.hpp>

struct Hatchery {};
struct Wall {};
struct Plain {};
struct Creep {};

using Tile = boost::variant<
    Wall,
    Hatchery,
    Plain,
    Creep
>;

using TileMatrix = boost::multi_array<Tile, 2>;
