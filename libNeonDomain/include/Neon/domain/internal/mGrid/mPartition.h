#pragma once

#include "Neon/domain/interface/NghInfo.h"
#include "Neon/domain/internal/bGrid/bCell.h"
#include "Neon/domain/internal/bGrid/bPartition.h"

#include "Neon/sys/memory/CUDASharedMemoryUtil.h"

namespace Neon::domain::internal::mGrid {

class bPartitionIndexSpace;

template <typename T, int C = 0>
class mPartition : public Neon::domain::internal::bGrid::bPartition<T, C>
{
   public:
    using PartitionIndexSpace = Neon::domain::internal::bGrid::bPartitionIndexSpace;
    using Cell = Neon::domain::internal::bGrid::bCell;
    using nghIdx_t = Cell::nghIdx_t;
    using Type = T;

   public:
    mPartition();

    ~mPartition() = default;

    explicit mPartition(Neon::DataView  dataView,
                        int             level,
                        T*              mem,
                        T*              memParent,
                        T*              memChild,
                        int             cardinality,
                        uint32_t*       neighbourBlocks,
                        Neon::int32_3d* origin,
                        uint32_t*       parent,
                        Cell::Location* parentLocalID,
                        uint32_t*       mask,
                        uint32_t*       maskLowerLevel,
                        uint32_t*       childBlockID,
                        uint32_t*       parentNeighbourBlocks,
                        T               defaultValue,
                        nghIdx_t*       stencilNghIndex,
                        int*            refFactors,
                        int*            spacing);

    /**
     * get the child of a cell
     * @param parent_cell the parent at which the child is queried 
     * @param child which child to return. A cell has number of children defined by the branching factor 
     * at the level. This defines the 3d local index of the child 
     * @param card which cardinality is desired from the child 
     * @param alternativeVal in case the child requested is not present     
    */
    NEON_CUDA_HOST_DEVICE inline auto childVal(const Cell&   parent_cell,
                                               Neon::int8_3d child,
                                               int           card,
                                               const T&      alternativeVal) const -> NghInfo<T>;

    /**
     * Get a cell that represents the child of a parent cell
     * @param parent_cell the parent cell that its child is requested  
     * @param child the child 3d local index relative to the parent      
    */
    NEON_CUDA_HOST_DEVICE inline auto getChild(const Cell&   parent_cell,
                                               Neon::int8_3d child) const -> Cell;


    /**
     * Given a child cell (as returned by getChild), return the value of this child 
     * @param childCell the child cell as returned by getChild 
     * @param card the cardinality in case of vector-valued data      
    */
    NEON_CUDA_HOST_DEVICE inline auto childVal(const Cell& childCell,
                                               int         card = 0) -> T&;

    /**
     * Given a child cell (as returned by getChild), return the value of this child 
     * @param childCell the child cell as returned by getChild 
     * @param card the cardinality in case of vector-valued data      
    */
    NEON_CUDA_HOST_DEVICE inline auto childVal(const Cell& childCell,
                                               int         card = 0) const -> const T&;

    /**
     * Check if the cell is refined i.e., has children 
     * @param cell the cell i.e., parent at which the children are checked      
    */
    NEON_CUDA_HOST_DEVICE inline auto hasChildren(const Cell& cell) const -> bool;

    /**
     * Check if a neighbor to 'cell' has children 
     * @param cell the main cell 
     * @param nghDir the direction relative to cell 
    */
    NEON_CUDA_HOST_DEVICE inline auto hasChildren(const Cell& cell, const Neon::int8_3d nghDir) const -> bool;


    /**
     * Get a cell that represents the parent of a given cell
     * @param cell the child cell for which the parent is queried 
    */
    NEON_CUDA_HOST_DEVICE inline auto getParent(const Cell& cell) const -> Cell;

    /**
     * Given a cell (child), return the value of the parent 
     * @param eId the cell 
     * @param card the cardinality in case of vector-valued data           
    */
    NEON_CUDA_HOST_DEVICE inline auto parentVal(const Cell& eId,
                                                int         card) -> T&;

    /**
     * Given a cell (child), return the value of the parent 
     * @param eId the cell 
     * @param card the cardinality in case of vector-valued data           
    */
    NEON_CUDA_HOST_DEVICE inline auto parentVal(const Cell& eId,
                                                int         card) const -> const T&;

    /**
     * check if the cell has a parent as defined by the user during the construction of the mGrid 
     * @param cell the cell i.e., the child at which the parent is checked      
    */
    NEON_CUDA_HOST_DEVICE inline auto hasParent(const Cell& cell) const -> bool;

    /**
     * The uncle of a cell at level L is a cell at level L+1 and is a neighbor to the cell's parent. 
     * This function returns the uncle of a given cell in a certain direction w.r.t the cell's parent
     * @param cell the main cell at level L 
     * @param direction the direction w.r.t the parent of cell      
    */
    NEON_CUDA_HOST_DEVICE inline auto getUncle(const Cell&   cell,
                                               Neon::int8_3d direction) const -> Cell;

    /**
     * The uncle of a cell at level L is a cell at level L+1 and is a neighbor to the cell's parent. 
     * This function returns the value of a give cell in a certain direction w.r.t the cell's parent along a certain cardinality.  
     * @param cell the main cell at level L 
     * @param direction the direction w.r.t the parent of cell      
     * @param card the cardinality 
     * @param alternativeVal alternative value in case the uncle does not exist. 
    */
    NEON_CUDA_HOST_DEVICE inline auto uncleVal(const Cell&   cell,
                                               Neon::int8_3d direction,
                                               int           card,
                                               const T&      alternativeVal) const -> NghInfo<T>;

    /**
     * @brief similar to the above uncleVal but returns a reference. Additionally, it is now
     * the user responsibility to check if the uncle is active (we only assert it)
     * @param cell the main cell at level L 
     * @param direction the direction w.r.t the parent of cell      
     * @param card the cardinality      
    */
    NEON_CUDA_HOST_DEVICE inline auto uncleVal(const Cell&   cell,
                                               Neon::int8_3d direction,
                                               int           card) const -> T&;

    /**
     * Get the refinement factor i.e., number of children at each dimension
     * @param level at which the refinement factor is queried      
    */
    NEON_CUDA_HOST_DEVICE inline auto getRefFactor(const int level) const -> int;


    NEON_CUDA_HOST_DEVICE inline auto getSpacing(const int level) const -> int;

    /**
     * Map the cell to its global index as defined by the finest level of the grid (Level 0)
     * @param cell which will be mapped to global index space      
    */
    NEON_CUDA_HOST_DEVICE inline Neon::index_3d mapToGlobal(const Cell& cell) const;


   private:
    inline NEON_CUDA_HOST_DEVICE auto childID(const Cell& cell) const -> uint32_t;


    int             mLevel;
    T*              mMemParent;
    T*              mMemChild;
    uint32_t*       mParentBlockID;
    Cell::Location* mParentLocalID;
    uint32_t*       mMaskLowerLevel;
    uint32_t*       mChildBlockID;
    uint32_t*       mParentNeighbourBlocks;
    int*            mRefFactors;
    int*            mSpacing;
};
}  // namespace Neon::domain::internal::mGrid

#include "Neon/domain/internal/mGrid/mPartition_imp.h"