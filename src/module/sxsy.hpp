/*
bellhopcxx / bellhopcuda - C++/CUDA port of BELLHOP underwater acoustics simulator
Copyright (C) 2021-2023 The Regents of the University of California
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
#include "../common_setup.hpp"
#include "paramsmodule.hpp"

namespace bhc { namespace module {

/**
 * source x-y coordinates
 */
template<bool O3D, bool R3D> class SxSy : public ParamsModule<O3D, R3D> {
public:
    SxSy() {}
    virtual ~SxSy() {}

    virtual void Init(bhcParams<O3D, R3D> &params) const override
    {
        params.Pos->Sx = nullptr;
        params.Pos->Sy = nullptr;
    }
    virtual void SetupPre(bhcParams<O3D, R3D> &params) const override
    {
        params.Pos->SxSyInKm = true;
        params.Pos->NSx      = 1;
        params.Pos->NSy      = 1;
    }
    virtual void Default(bhcParams<O3D, R3D> &params) const override
    {
        trackallocate(params, "default source x-coordinates", params.Pos->Sx, 1);
        trackallocate(params, "default source y-coordinates", params.Pos->Sy, 1);
        params.Pos->Sx[0] = FL(0.0); // dummy x-coordinate
        params.Pos->Sy[0] = FL(0.0); // dummy y-coordinate
    }
    virtual void Read(
        bhcParams<O3D, R3D> &params, LDIFile &ENVFile, HSInfo &) const override
    {
        if constexpr(O3D) {
            ReadVector(params, params.Pos->Sx, params.Pos->NSx, ENVFile, DescriptionX);
            ReadVector(params, params.Pos->Sy, params.Pos->NSy, ENVFile, DescriptionY);
        } else {
            Default(params);
        }
    }
    virtual void Validate(bhcParams<O3D, R3D> &params) const override
    {
        if(params.Pos->NSx * params.Pos->NSy * params.Pos->NSz <= 0) {
            EXTERR(
                "Invalid number of sources: %d x %d y %d z", params.Pos->NSx,
                params.Pos->NSy, params.Pos->NSz);
        }

        ValidateVector(params, params.Pos->Sx, params.Pos->NSx, DescriptionX);
        ValidateVector(params, params.Pos->Sy, params.Pos->NSy, DescriptionY);
    }
    virtual void Echo(bhcParams<O3D, R3D> &params) const override
    {
        Preprocess(params);
        EchoVectorWDescr(
            params, params.Pos->Sx, params.Pos->NSx, FL(0.001), DescriptionX, Units);
        EchoVectorWDescr(
            params, params.Pos->Sy, params.Pos->NSy, FL(0.001), DescriptionY, Units);
    }
    virtual void Preprocess(bhcParams<O3D, R3D> &params) const override
    {
        if(!params.Pos->SxSyInKm) return;
        ToMeters(params.Pos->Sx, params.Pos->NSx);
        ToMeters(params.Pos->Sy, params.Pos->NSy);
        params.Pos->SxSyInKm = false;
    }
    virtual void Finalize(bhcParams<O3D, R3D> &params) const override
    {
        trackdeallocate(params, params.Pos->Sx);
        trackdeallocate(params, params.Pos->Sy);
    }

private:
    constexpr static const char *DescriptionX = "Source   x-coordinates, Sx";
    constexpr static const char *DescriptionY = "Source   y-coordinates, Sy";
    constexpr static const char *Units        = "km";
};

}} // namespace bhc::module
