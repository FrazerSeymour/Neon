#pragma once

#include "Neon/domain/mGrid.h"
#include "Neon/skeleton/Skeleton.h"

template <typename T, int Q>
void postProcess(Neon::domain::mGrid&                        grid,
                 const int                                   numLevels,
                 const Neon::domain::mGrid::Field<T>&        fpop,
                 const Neon::domain::mGrid::Field<CellType>& cellType,
                 const int                                   iteration,
                 Neon::domain::mGrid::Field<T>&              vel,
                 Neon::domain::mGrid::Field<T>&              rho,
                 bool                                        generateValidateFile = false)
{
    grid.getBackend().syncAll();

    for (int level = 0; level < numLevels; ++level) {
        auto container =
            grid.getContainer(
                "postProcess_" + std::to_string(level), level,
                [&, level](Neon::set::Loader& loader) {
                    const auto& pop = fpop.load(loader, level, Neon::MultiResCompute::STENCIL);
                    const auto& type = cellType.load(loader, level, Neon::MultiResCompute::MAP);
                    auto&       u = vel.load(loader, level, Neon::MultiResCompute::MAP);
                    auto&       rh = rho.load(loader, level, Neon::MultiResCompute::MAP);


                    return [=] NEON_CUDA_HOST_DEVICE(const typename Neon::domain::bGrid::Cell& cell) mutable {
                        if (!pop.hasChildren(cell)) {
                            if (type(cell, 0) == CellType::bulk) {

                                //fin
                                T ins[Q];
                                for (int i = 0; i < Q; ++i) {
                                    ins[i] = pop(cell, i);
                                }

                                //density
                                T r = 0;
                                for (int i = 0; i < Q; ++i) {
                                    r += ins[i];
                                }
                                rh(cell, 0) = r;

                                //velocity
                                const Neon::Vec_3d<T> vel = velocity<T, Q>(ins, r);

                                u(cell, 0) = vel.v[0];
                                u(cell, 1) = vel.v[1];
                                u(cell, 2) = vel.v[2];
                            }
                            if (type(cell, 0) == CellType::movingWall) {
                                rh(cell, 0) = 1.0;

                                for (int d = 0; d < 3; ++d) {
                                    int i = (d == 0) ? 3 : ((d == 1) ? 1 : 9);
                                    u(cell, d) = pop(cell, i) / (6.0 * 1.0 / 18.0);
                                }
                            }
                        }
                    };
                });

        container.run(0);
    }

    grid.getBackend().syncAll();


    vel.updateIO();
    //rho.updateIO();


    int                precision = 4;
    std::ostringstream suffix;
    suffix << std::setw(precision) << std::setfill('0') << iteration;

    vel.ioToVtk("Velocity_" + suffix.str());
    //rho.ioToVtk("Density_" + suffix.str());

    if (generateValidateFile) {
        const Neon::index_3d grid_dim = grid.getDimension();
        std::ofstream        file;
        file.open("NeonMultiResLBM_" + suffix.str() + ".dat");

        for (int level = 0; level < numLevels; ++level) {
            vel.forEachActiveCell(
                level, [&](const Neon::index_3d& id, const int& card, T& val) {
                    if (id.x == grid_dim.x / 2 && id.z == grid_dim.z / 2) {
                        if (card == 0) {
                            file << static_cast<double>(id.v[1]) / static_cast<double>(grid_dim.y) << " " << val << "\n";
                        }
                    }
                },
                Neon::computeMode_t::seq);
        }
        file.close();
    }
}