#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include "Core/Vec2.hpp"

//We divide the 2D window into a grid of cells. A particle at (𝑥,𝑦) only needs to check collisions with particles in its 
//own cell and the 8 adjacent neighbor cells (3×3 neighborhood). This reduces collision complexity from 𝑂(𝑁^2)
//to O(N)!

namespace ape{

    struct Grid
    {
        //The width and height of each square grid cell. Usually set to ≥ 2 * 𝑟_max 
        //so a particle cannot span more than adjacent neighbor cells.
        float cellSize{16.0f}; // Diameter of largest particle of serach radius
        std::size_t gridWidth{0}; // number off cell columns
        std::size_t gridHeight{0}; // '' '' '' rows

        // Flattened 2D grid storing indices of particles present in each cell
        std::vector<std::vector<std::size_t>> cells;

        //Initialize or resize the spatial grid based on screen dimensions
        void init(float screenWidht, float screenHeight, float size)
        {
            cellSize = size;
            //Computes how many columns fit across the screen horizontally.
            gridWidth = static_cast<std::size_t>(screenWidht/cellSize) + 1; // pixel widht of a cell
            gridHeight = static_cast<std::size_t>(screenHeight/cellSize) + 1; // pixel height of a cell
            cells.resize(gridWidth * gridHeight); // pixels^2
        }

        void clear() noexcept //Resets all cell contents at the start of each physics frame
        {
            const std::size_t totalCells = cells.size();
            for (std::size_t i = 0; i < totalCells; i++)
            {
                cells[i].clear();
            }
            
        }
        
        // Convert world 2D coordinates into a 1D flattened cell index
        [[nodiscard]] std::size_t getCellIndex (float x, float y) const noexcept
        {
            const auto cellX = static_cast<std::size_t>(std::max(0.0f,x) / cellSize);
            const auto cellY = static_cast<std::size_t>(std::max(0.0f,y) / cellSize);

            const std::size_t clampedX = std::min(cellX, gridWidth - 1);
            const std::size_t clampedY = std::min(cellY, gridHeight - 1);

            //index = y * width + x
            return clampedY * gridWidth + clampedX; //The standard row-major mapping formula -> index for 2D grids
        }

        // Insert all active particles from SoA (Struct of Arrays.) positions into their corresponding grid cells
        void populate(const std::vector<Vec2>& positions)
        {
            const std::size_t count = positions.size();
            for (std::size_t i = 0; i < count; i++)
            {
                const std::size_t cellIdx = getCellIndex(positions[i].x, positions[i].y);
                cells[cellIdx].push_back(i); //Stores the index i of the particle inside its corresponding cell.
            }
            
        }

    };
    
}