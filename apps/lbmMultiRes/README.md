# Overview 

We outline here the complete guide on how to reproduce the results presented in the [Optimized GPU Implementation of Grid Refinement in Lattice Boltzmann Method](http://escholarship.org/uc/item/0x86w4w1) paper. We also outline how to use the code to run other user-defined examples along with areas for customization. For more information about Neon, please check out the main [README](/../../README.md)

# Getting Started 

## Prerequisites
Neon runs on all major systems that support running Nvidia GPUs. We have tested the code on Windows 10/11 (VS 2019/2022) and Ubuntu 20.04/22.04. 

- C++ compiler with C++17 standard support
- CUDA version 12 or higher
- CMake version 3.19 or higher


## Build

To build and compile the grid refinement LBM application, first, make sure to be connected to the internet for Neon to download its dependencies. Then, run:

```
git clone -b v0.5.1 https://github.com/Autodesk/Neon
cd Neon
mkdir build
cd build
cmake ../
cmake --build . --target app-lbmMultiRes --config Release -j 10
```

# Grid Refinement LBM

`app-lbmMultiRes` comes with two main problem setup 

1. Virtual wind tunnel where we simulate a flow over an input geometry defined by a triangle mesh 
2. Lid-driven cavity which is a classical CFD test case for measuring the accuracy of the simulation 

Both problem setups can be run on either GPU (for fast high-performance simulation) or CPU (for debugging). The executable comes with a set of input user-defined parameters. To display them, run 

```bash
./bin/app-lbmMultiRes -h
```
### Problem setup parameters:

| Parameter          | Option             | Comment                                                                                                                    |
|--------------------|--------------------|----------------------------------------------------------------------------------------------------------------------------|
| `--deviceType`     |  `cpu`, `gpu`      | to select between running on GPU or CPU                                                                                    |
| `--deviceId`       |  integer value     | The GPU device ID                                                                                                          |
| `--numIter`        |  integer value     | Number of LBM iterations run on the coarsest level                                                                         |
| `--problemType`    |  `lid`, `mesh`     | Problem type where `lid` is the lid-driven cavity problem and `mesh` is the flow over an input mesh, i.e., virtual wind tunnel |
| `--dataType`       |  `float`, `double` | The precision of data type used in the simulation                                                                          |
| `--re`             |  real value        | Reynolds number used in the simulation                                                                                     |
| `--thetaX`         |  real value        | For the `mesh` problem, the angle of rotation of the input mesh along the X axis                                                   |
| `--thetaY`         |  real value        | For the `mesh` problem, the angle of rotation of the input mesh along the Y axis                                                   |
| `--thetaZ`         |  real value        | For the `mesh` problem, the angle of rotation of the input mesh along the Z axis                                                   |
| `--scale`          |  integer value     | A value that allows scaling up the problem to a larger size to allow easy benchmarking                                        |

### Visualization parameters:

| Parameter          | Option             | Comment                                                                                                                    |
|--------------------|--------------------|----------------------------------------------------------------------------------------------------------------------------|
| `--benchmark`      |                    | Run in benchmark mode, i.e., no visualization output                                                                       |
| `--visual`         |                    | Run in visualization mode where we output PNG images of the simulation at the specified frequency                         |
| `--freq`           |  integer value     | Frequency of the output for visualization. This option is allowed only with `--visual` mode                                |
| `--vtk`            |                    | Output VTK files of the simulation. This option is allowed only with `--visual` mode                                       |
| `--binary`         |                    | Output binary down-sampled files of the simulation. This option is allowed only with `--visual` mode                       |
| `--sliceX`         |  integer value     | Slice along the X axis for output images/VTK                                                                                   |
| `--sliceY`         |  integer value     | Slice along the Y axis for output images/VTK                                                                                   |
| `--sliceZ`         |  integer value     | Slice along the Z axis for output images/VTK                                                                                   |

### Performance parameters:
By default, we run the best possible configuration as presented in our [paper](http://escholarship.org/uc/item/0x86w4w1). Below are the parameters that would help to reproduce the ablation study along with the figure in the paper that schematically shows the operation 

| Parameter                    | Option             | Comment                                                                                                                    |
|------------------------------|--------------------|----------------------------------------------------------------------------------------------------------------------------|
| `--storeCoarse`              |                    |  Initiate the Accumulate operation from the coarse level as done in the baseline algorithm (Figure 4.a)                    |
| `--storeFine`                |                    |  Initiate the Accumulate operation from the fine level (Figure 4.b)                                                        |
| `--collisionFusedStore`      |                    |  Fuse Collision with Accumulate operation (Figure 4.c)                                                                     |
| `--streamFusedExpl`          |                    |  Fuse Stream with Explosion (Figure 4.d)                                                                                   |
| `--streamFusedCoal`          |                    |  Fuse Stream with Coalescence (Figure 4.e)                                                                                 |
| `--streamFuseAll`            |                    |  Fuse Stream with Coalescence and Explosion (Figure 4.f)                                                                   |
| `--fusedFinest`              |                    |  Fuse all operations on the finest level, i.e., Collision, Accumulate, Explosion, Stream  (Figure 4.f)                     |

Finally, to switch between LBM collision models (`KBC` and `BGK`), change the `#define` directive parameter at the top of the [`Neon/apps/lbmMultiRes/lbmMultiRes.cu`](https://github.com/Autodesk/Neon/blob/v0.5.1/apps/lbmMultiRes/lbmMultiRes.cu).

## Lid-driven cavity
Once the execution of the lid-driven cavity problem is completed, the simulation will output two files (`NeonMultiResLBM_####_Y.dat`, `NeonMultiResLBM_####_X.dat`) which can be used to reproduce Figure 7 in the paper. To reproduce the figure, pass these two files to the python script under [`Neon/apps/lbmMultiRes/scripts/plot.py`](https://github.com/Autodesk/Neon/blob/v0.5.1/apps/lbmMultiRes/scripts/plot.py). 

Example (without visualization): 

`bin/app-lbmMultiRes --deviceType gpu --deviceId 0 --numIter 1000 --problemType lid --scale 2 --benchmark`

## Virtual Wind Tunnel 

The `flowOverMesh` method in [`Neon/apps/lbmMultiRes/flowOverShape.h`](https://github.com/Autodesk/Neon/blob/v0.5.1/apps/lbmMultiRes/flowOverShape.h) defined various geometric properties to run a fluid simulation over a shape.  The method is fully documented to facilitate customization. 

The airplane input mesh used in Figure 1 can be found at [`Neon/apps/lbmMultiRes/practice_v28.obj`](https://github.com/Autodesk/Neon/blob/v0.5.1/apps/lbmMultiRes/practice_v28.obj).

Example (with visualization): 

`bin/app-lbmMultiRes --deviceType gpu  --deviceId 0 --problemType mesh --vtk --visual --binary --re 2000 --thetaZ 30 --freq 10000 --numIter 10000 --scale 84 --meshFile ../apps/lbmMultiRes/practice_v28.obj`

---

After the simulation is complete, we output a JSON file that outlines the following 
- Github SHA
- CPU and GPU system specs 
- Backend (CPU vs. GPU)
- Command line used
- Gird size and active voxels at each level 
- Data type (float vs. double)
- Algorithm 
- LBM Million Lattice Updates per Second (MLUPS)
- Number of iterations 
- Collision model
- Input velocity 
- Problem scale 


# Citation

```
@inproceedings{Mahmoud:2024:OGI,
  author    = {Mahmoud, Ahmed H. and Salehipour, Hesam and Meneghin, Massimiliano},
  booktitle = {Proceedings of the 38th IEEE International Parallel and Distributed Processing Symposium},
  title     = {Optimized GPU Implementation of Grid Refinement in Lattice Boltzmann Method},
  year      = 2024,
  month     = {},
  pages     = {},
  doi       = {},
  url       = {http://escholarship.org/uc/item/0x86w4w1}
}
```