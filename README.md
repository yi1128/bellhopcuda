# bellhopcuda
CUDA and C++ port of BELLHOP.

## Two Orthogonal Options
bellhopcuda is set up to be built in pure C++11 or in CUDA/C++. The `HOST_DEVICE`
and `STD` macros faciliate this; most of the code transparently uses either.

Also, bellhopcuda is set up to be built with single-precision or double-
precision. The `real` type is the selected type (float or double) and the `cpx`
type is complex numbers of this same precision. The `RC` macro ("real constant")
transforms floating-point literals to the appropriate precision, by adding `f`
if in single-precision mode. Due to C/C++ type casting rules which generally
require floating-point arithmetic to be performed in the widest type of the
input values, if a double-precision literal is used in a CUDA kernel, CUDA may
emit double-precision instructions around this literal and destroy the kernel
performance. Strictly speaking, this may not be needed in all cases (e.g.
`float f = 0.0;` and `f *= 2.0;` will probably not emit double-precision
instructions, as the results should always be bitwise identical whether the
operations were performed in 32-bit precision or performed in 64-bit precision
and then typecasted), but for consistency it should be used on every floating-
point literal.

## Comments
Unattributed comments in all translated code are copied directly from the original
BELLHOP and/or Acoustics Toolbox code, mostly by Michael B. Porter. Unattributed
comments in new code are by the bellhopcuda team, mostly Louis Pisha.

## Code style
Code style (things like where newlines and if statements are) is kept as close
as possible to the original FORTRAN code, for ease of comparing the source files.

# Map

### Support code

`common.hpp`: Main support code, CUDA compatibility, utilities, etc.
- `misc/monotonicMod.f90`
- `misc/SortMod.f90`
- `misc/subtabulate.f90`
- `misc/MathConstants.f90`
- `misc/cross_products.f90`: Only used by BELLHOP3D, provided by glm.
- `misc/FatalError.f90`: Not applicable.

`ldio.hpp`: C++ emulation of FORTRAN list-directed I/O.

`curves.hpp`: Templated splines and Piecewise Cubic Hermite Interpolating Polynomial.
- `misc/pchipMod.f90`
- `misc/splinec.f90`
- `misc/splined.f90`: Double version, not used by BELLHOP.
- `misc/spliner.f90`: Float version, not used by BELLHOP.

### Data I/O code

`ssp.hpp`: Sound speed as a function of position.
- `Bellhop/sspMod.f90`
- `misc/sspMod.f90`: Base implementation for other programs, but without
derivatives and with less pre-computed info. Not used by BELLHOP.

`attenuation.hpp`: Sound speed and attenuation.
- `misc/AttenMod.f90`

`boundary.hpp`: Ocean surface and floor.
- `Bellhop/bdryMod.f90`
- `Bellhop/bdry3DMod.f90`: BELLHOP3D version.

`refcoef.hpp`: Reflection coefficients.
- `misc/RefCoef.f90`

`sourcereceiver.hpp`: Source and receiver (single or array) positions, also
frequency information.
- `misc/SourceReceiverPositions.f90`

`angles.hpp`: Source ray angles.
- `Bellhop/angleMod.f90`

`beams.hpp`: Beam information and source beam pattern.
- `misc/beampattern.f90`
- `Bellhop/bellhopMod.f90`: BeamStructure

`arrivals.hpp`: Ray arrival recording.
- `Bellhop/ArrMod.f90`: NEED all non-3D

`readenv.cpp`: Main environment file reading.
- `Bellhop/ReadEnvironmentBell.f90`
- `misc/ReadEnvironmentMod.f90`: Base implementation for other programs, not
used by BELLHOP.

`output.cpp`: Output files writing.
- `Bellhop/ReadEnvironmentBell.f90`: OpenOutputFiles
- `Bellhop/WriteRay.f90`
- `misc/RWSHDFile.f90`: NEED all

### Core simulation code

`step.hpp`: Ray/beam tracing single step.
- `Bellhop/Step.f90`
- `Bellhop/Step3DMod.f90`: BELLHOP3D version.
- `Bellhop/RayNormals.f90`: Used by Step3D and Reflect3D, but not by (2D)
BELLHOP.
- `Bellhop/bellhopMod.f90`: ray2DPt

`trace.hpp`: Ray/beam tracing main.
- `Bellhop/bellhop.f90`: TraceRay2D, Distances2D, Reflect2D
- `Bellhop/ReflectMod.f90`: Alternate implementation of Reflect2D. BELLHOP
seems to use the implementation within `Bellhop/bellhop.f90`. BELLHOP3D uses
this (despite it not being 3D).
- `Bellhop/Reflect3DMod.f90`: BELLHOP3D version.
- `Bellhop/Cone.f90`: Used by ReflectMod and Reflect3DMod, but not by (2D)
BELLHOP.

`influence.hpp`: Contribution of a beam to the field.
- `Bellhop/influence.f90`: NEED all
- `Bellhop/influence3D.f90`: BELLHOP3D version.

### Top-level executables

TODO
- `Bellhop/bellhop.f90`: 
- `Bellhop/bellhop3D.f90`: 

### FORTRAN code not used by BELLHOP or BELLHOP3D:
- `misc/calculateweights.f90`
- `misc/interpolation.f90`
- `misc/MergeVectorsMod.f90`
- `misc/munk.f90`
- `misc/norms.f90`
- `misc/PekRoot.f90`
- `misc/PolyMod.f90`
- `misc/RootFinderSecantMod.f90`
- `misc/weight New in progress.f90`


# Test results

### Ray tracing only

Succeeding:

A: Both succeed; have not checked numerical results yet.
B: Results seem to be within machine precision, with same step counts.
C: Results seem to be within machine precision, but at some boundary reflections,
   BELLHOP has an extra point (3 points at boundary) compared to bellhopcuda (2
   points at boundary). However, sometimes (e.g. blockB_ray:2085) there are 2
   points from bellhopcuda and only 1 point from BELLHOP.

block/blockB_ray.env:       C
calib/calibray.env:         C
calib/calibraygrad.env:     B
Dickins/DickinsBray.env:    C
Dickins/DickinsFixedSlashes.env: C
Dickins/DickinsFlatBray.env: C
Gulf/Gulf_ray_rd.env:       B
Gulf/Gulf_ray_ri.env:       B
halfspace/vacuumBFixed.env: C
Munk/MunkB_ray.env:         C
Munk/MunkB_ray_rot.env:     C
ParaBot/ParaBot.env:        C; one ray checked seemed to be farther off than usual, e.g. 1e-4 relative error rather than the usual <1e-6 relative error.

Failing:

X: Both fail due to env file syntax, not easily correctible without changing the
run.
Y: Both fail due to correctible env file syntax issues.
Z: Both fail, but BELLHOP gives error about number of media, whereas bellhopcuda
gives error about syntax due to the run format being wrong.

Dickins/Dickins.env:        Y
halfspace/vacuumB.env:      Y
head/head.env:              Z
head/headS.env:             Z
head/iso.env:               Z
sduct/sductBray.env:        X
sduct/sductnew.env:         X
sduct/single.env:           X
