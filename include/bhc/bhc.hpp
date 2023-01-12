/*
bellhopcxx / bellhopcuda - C++/CUDA port of BELLHOP underwater acoustics simulator
Copyright (C) 2021-2022 The Regents of the University of California
c/o Jules Jaffe team at SIO / UCSD, jjaffe@ucsd.edu
Based on BELLHOP, which is Copyright (C) 1983-2020 Michael B. Porter

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#define _USE_MATH_DEFINES 1 // must be before anything which includes math.h
#include <math.h>

#define _BHC_INCLUDED_ 1
#include "platform.hpp"
#include "math.hpp"
#include "structs.hpp"
#undef _BHC_INCLUDED_

namespace bhc {

/**
 * Main BELLHOP setup from an environment file. Call this to create and
 * initialize the params. You may modify the params after calling this and
 * before calling run().
 *
 * You may use "multiple instances" of bellhopcxx / bellhopcuda within the same
 * process by calling this (and the other functions below) with different params
 * and outputs; there are no global variables in the library.
 *
 * FileRoot: Relative path to environment file, without the .env extension. E.g.
 * path/to/MunkB_ray_rot (where path/to/MunkB_ray_rot.env and also path/to/
 * MunkB_ray_rot.ssp, path/to/MunkB_ray_rot.bty, etc. exist).
 *
 * outputCallback: Callback called by setup/run code which will be called for
 * messages (e.g. debug output, error messages). If nullptr is passed, it will
 * open a PRTFile (<FileRoot>.prt) and put the messages in there. If you are
 * using multiple instances (multiple calls to setup with different params),
 * and you pass a callback function here, the callback must be thread-safe as it
 * will get called from each of the instances at the same time. If you are using
 * multiple instances and PRTFiles (nullptr here), each instance must use a
 * different FileRoot or there will be issues with the multiple instances trying
 * to write to the same PRTFile.
 *
 * params, outputs: Just create uninitialized structs and pass them in to be
 * initialized. You may modify params after setup.
 *
 * returns: false on fatal errors, true otherwise. If a fatal error occurs,
 * must call finalize() and setup() again before continuing to use the library.
 *
 * O3D stands for "ocean 3D" and R3D stands for "ray(s) 3D".
 * O3D=false, R3D=false: 2D mode
 * O3D=true, R3D=false: Nx2D mode
 * O3D=true, R3D=true: 3D mode
 */
template<bool O3D, bool R3D> bool setup(
    const char *FileRoot, void (*outputCallback)(const char *message),
    bhcParams<O3D, R3D> &params, bhcOutputs<O3D, R3D> &outputs);

/// 2D version, see template.
extern template BHC_API bool setup<false, false>(
    const char *FileRoot, void (*outputCallback)(const char *message),
    bhcParams<false, false> &params, bhcOutputs<false, false> &outputs);
/// Nx2D or 2D-3D version, see template.
extern template BHC_API bool setup<true, false>(
    const char *FileRoot, void (*outputCallback)(const char *message),
    bhcParams<true, false> &params, bhcOutputs<true, false> &outputs);
/// 3D version, see template.
extern template BHC_API bool setup<true, true>(
    const char *FileRoot, void (*outputCallback)(const char *message),
    bhcParams<true, true> &params, bhcOutputs<true, true> &outputs);

/**
 * Runs the selected run type and places the results in the appropriate struct
 * within outputs.
 *
 * An env file should usually be read directly first, by calling setup then
 * run-finalize. But, before calling finalize, you may edit parameters and rerun
 * with an expected pattern
 *     setup-run-change params-run-change params...-finalize.
 * TODO: Only a few parameters can be updated, notably sources and 1D SSP - JS, 8/25/2022.
 *
 * returns: false on fatal errors, true otherwise. If a fatal error occurs,
 * must call finalize() and setup() again before continuing to use the library.
 */
template<bool O3D, bool R3D> bool run(
    bhcParams<O3D, R3D> &params, bhcOutputs<O3D, R3D> &outputs);

/// 2D version, see template.
extern template BHC_API bool run<false, false>(
    bhcParams<false, false> &params, bhcOutputs<false, false> &outputs);
/// Nx2D or 2D-3D version, see template.
extern template BHC_API bool run<true, false>(
    bhcParams<true, false> &params, bhcOutputs<true, false> &outputs);
/// 3D version, see template.
extern template BHC_API bool run<true, true>(
    bhcParams<true, true> &params, bhcOutputs<true, true> &outputs);

/**
 * Write results for the past run to BELLHOP-formatted files, i.e. a ray file,
 * a shade file, or an arrivals file. If you only want to use the results in
 * memory, there is no need to call this.
 *
 * Note that the writeout process modifies the data for most runs:
 * - eigenrays: During run(), only information about which rays hit the
 *   receiver is stored, not the full ray trajectories. During writeout(), those
 *   rays are traced again and written.
 * - rays and eigenrays: Compresses the ray data (discards some points)
 * - TL: scales the field in various ways
 *
 * run() must have been called previously. Don't forget to call finalize()
 * when you're done.
 */
template<bool O3D, bool R3D> bool writeout(
    const bhcParams<O3D, R3D> &params, bhcOutputs<O3D, R3D> &outputs);

/// 2D version, see template.
extern template BHC_API bool writeout<false, false>(
    const bhcParams<false, false> &params, bhcOutputs<false, false> &outputs);
/// Nx2D or 2D-3D version, see template.
extern template BHC_API bool writeout<true, false>(
    const bhcParams<true, false> &params, bhcOutputs<true, false> &outputs);
/// 3D version, see template.
extern template BHC_API bool writeout<true, true>(
    const bhcParams<true, true> &params, bhcOutputs<true, true> &outputs);

/**
 * Frees memory. You may call run() many times (with changed parameters), you do
 * not have to call setup - run - finalize every time.
 */
template<bool O3D, bool R3D> void finalize(
    bhcParams<O3D, R3D> &params, bhcOutputs<O3D, R3D> &outputs);

/// 2D version, see template.
extern template BHC_API void finalize<false, false>(
    bhcParams<false, false> &params, bhcOutputs<false, false> &outputs);
/// Nx2D or 2D-3D version, see template.
extern template BHC_API void finalize<true, false>(
    bhcParams<true, false> &params, bhcOutputs<true, false> &outputs);
/// 3D version, see template.
extern template BHC_API void finalize<true, true>(
    bhcParams<true, true> &params, bhcOutputs<true, true> &outputs);

} // namespace bhc

#ifdef BHC_UNDEF_STD_AFTER
#undef STD
#endif
