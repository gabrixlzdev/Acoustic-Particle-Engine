#include <cassert>
#include <vector>
#include "Physics/Grid.hpp"
#include "Core/Vec2.hpp"

int main() {
    ape::Grid grid;

    // Initialize an 800x600 grid with 16px cell size
    grid.init(800.0f, 600.0f, 16.0f);
    assert(grid.gridWidth == (800 / 16) + 1); // 51 columns
    assert(grid.gridHeight == (600 / 16) + 1); // 38 rows
    assert(grid.cells.size() == grid.gridWidth * grid.gridHeight); // pixles^2 size of a cell

    //Test cell index mapping (y * width + x)
    // Particle at (0, 0) should be at cell (0, 0) -> index 0
    assert(grid.getCellIndex(0.0f, 0.0f) == 0);

    // Particle at (16.0f, 0.0f) should be at column 1, row 0 -> index 1
    assert(grid.getCellIndex(16.0f, 0.0f) == 1);

    // Particle at (0.0f, 16.0f) should be at column 0, row 1 -> index gridWidth or in this case 51 columns
    // index = (y*width + x); (x = 0, y =1)
    assert(grid.getCellIndex(0.0f, 16.0f) == grid.gridWidth);

    // 3. Test populate and clear
    std::vector<ape::Vec2> positions = {
        {5.0f, 5.0f},   // In cell 0
        {10.0f, 10.0f}, // Also in cell 0
        {32.0f, 0.0f}   // In cell 2 (col 2, row 0)
    };

    grid.populate(positions);
    assert(grid.cells[0].size() == 2); // 2 particles in cell 0
    assert(grid.cells[2].size() == 1); // 1 particle in cell 2

    // 4. Test clear
    grid.clear();
    assert(grid.cells[0].empty());
    assert(grid.cells[2].empty());

    return 0;
}