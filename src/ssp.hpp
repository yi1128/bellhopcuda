#pragma once
#include "common.hpp"
#include "ldio.hpp"
#include "curves.hpp"
#include "attenuation.hpp"
#include "sourcereceiver.hpp"

constexpr int32_t MaxN = 100000;
constexpr int32_t MaxSSP = MaxN + 1;

constexpr real betaPowerLaw = RC(1.0);

struct rxyz_vector {
    real *r, *x, *y, *z;
};

struct SSPStructure {
    int32_t NPts, Nr, Nx, Ny, Nz;
    real z[MaxSSP], rho[MaxSSP];
    cpx c[MaxSSP], cz[MaxSSP], n2[MaxSSP], n2z[MaxSSP], cSpline[4][MaxSSP];
    cpx cCoef[4][MaxSSP], CSWork[4][MaxSSP]; // for PCHIP coefs.
    real *cMat, *czMat, *cMat3, *czMat3;
    rxyz_vector Seg;
    char Type;
    char AttenUnit[2];
};

struct HSInfo {
    real alphaR, betaR, alphaI, betaI; // compressional and shear wave speeds/attenuations in user units
    cpx cP, cS; // P-wave, S-wave speeds
    real rho, Depth; // density, depth
    char bc; // Boundary condition type
    char Opt[6];
};

/**
 * LP: Compare with BdryPtFull in boundary.hpp.
 */
struct BdryPtSmall {
    HSInfo hs;
};

struct BdryType {
    BdryPtSmall Top, Bot;
};


#define SSP_FN_ARGS const vec2 &x, const vec2 &t, cpx &ccpx, vec2 &gradc, \
    real &crr, real &crz, real &czz, real &rho, real freq, \
    const SSPStructure *ssp, int32_t &iSegz, int32_t &iSegr
#define SSP_CALL_ARGS x, t, ccpx, gradc, crr, crz, czz, rho, freq, ssp, iSegz, iSegr
#define SSP_INIT_ARGS vec2 x, const real &fT, \
    LDIFile &ENVFile, std::ofstream &PRTFile, std::string FileRoot, \
    SSPStructure *ssp, const AttenInfo *atten, const FreqInfo *freqinfo
#define SSP_CALL_INIT_ARGS x, fT, ENVFile, PRTFile, FileRoot, ssp, atten, freqinfo

#if 0
HOST_DEVICE inline void UpdateDepthSegment(const vec2 &x,
    const SSPStructure *ssp, int32_t &iSegz)
{
    /*
    // LP: Original code. Searches from the beginning, when in reality usually
    // we only change by one segment.
    if(x.y < ssp->z[iSegz] || x.y > ssp->z[iSegz+1]){
        // Search for bracketting Depths
        for(int32_t iz = 1; iz < ssp->Npts; ++iz){
            if(x.y < ssp->z[iz]){
                iSegz = iz - 1;
                break;
            }
        }
    }
    */
    // LP: New code. A more verbose version of this approach was implemented,
    // and commented out, within the original Quad function. It had the
    // following comment:
    // > The following tries to be more efficient than the code above by searching
    //   away from the current layer
    // > rather than searching through all the layers
    // > However, seems to be no faster
    // > Also, this code caused a problem on at/tests/Gulf for the range-dep. 
    //   test cases
    // Assuming the depths are monotonically increasing, the two algorithms
    // should always produce the same result--except for the cases where
    // x.y lands exactly on a boundary. All three algorithms permit
    // 0.0 <= w <= 1.0 in the initial check.
    // Slow algorithm: search returns 0.0 <= w < 1.0
    // Commented out fast algorithm: search returns 0.0 <= w <= 1.0, with the
    //   greater amount of motion to get to the right segment (i.e. will move by
    //   at least 2 segments if the result is on the boundary)
    // Fast algorithm here: search returns 0.0 <= w <= 1.0, with the least
    //   amount of motion to get to the right segment
    // Assuming the splines etc. which depend on this info are implemented
    // correctly, these differences should never matter.
    // Finally, while on the CPU some code which is inefficient but runs only on
    // a very small fraction of iterations doesn't affect much, on the GPU this
    // can be a huge bottleneck because it may make many other threads wait.
    // LP (later): It turns out that ReduceStep2D produces substantially different
    // results depending on how the endpoints are treated here. This probably
    // means there is some poor design elsewhere, but we have to match it.
    // So this modification allows the results to match the slow version.
    while(x.y < ssp->z[iSegz] && iSegz > 0) --iSegz;
    if(x.y <= ssp->z[iSegz+1]) return;
    while(x.y >= ssp->z[iSegz+1] && iSegz < ssp->NPts-2) ++iSegz;
    // Fast version replaces the above two lines:
    //while(x.y > ssp->z[iSegz+1] && iSegz < ssp->NPts-2) ++iSegz;
}

HOST_DEVICE inline void UpdateRangeSegment(const vec2 &x,
    const SSPStructure *ssp, int32_t &iSegr)
{
    // LP: Same deal as UpdateDepthSegment, except there was no commented out
    // fast version.
    /*
    if(x.x < ssp->Seg.r[iSegr] || x.x > ssp->Seg.r[iSegr+1]){
        // Search for bracketting segment ranges
        for(int32_t iSegT = 1; iSegT < ssp->Nr; ++iSegT){
            if(x.x < ssp->Seg.r[iSegT]){
                iSegr = iSegT - 1;
                break;
            }
        }
    }
    */
    while(x.x < ssp->Seg.r[iSegr] && iSegr > 0) --iSegr;
    if(x.x <= ssp->Seg.r[iSegr+1]) return;
    while(x.x >= ssp->Seg.r[iSegr+1] && iSegr < ssp->Nr-2) ++iSegr;
    // Fast version replaces the above two lines:
    //while(x.x > ssp->Seg.r[iSegr+1] && iSegr < ssp->Nr-2) ++iSegr;
}
#endif

HOST_DEVICE inline void UpdateDepthSegmentT(const vec2 &x, const vec2 &t,
    const SSPStructure *ssp, int32_t &iSegz)
{
    //LP: Handles edge cases based on which direction the ray is going. If the
    //ray takes a small step in the direction of t, it will remain in the same
    //segment as it is now.
    if(t.y >= RC(0.0)){
        //ssp->z[iSegz] <= x.y < ssp->z[iSegz+1]
        while(x.y < ssp->z[iSegz] && iSegz > 0) --iSegz;
        while(x.y >= ssp->z[iSegz+1] && iSegz < ssp->NPts-2) ++iSegz;
    }else{
        //ssp->z[iSegz] < x.y <= ssp->z[iSegz+1]
        while(x.y > ssp->z[iSegz+1] && iSegz < ssp->NPts-2) ++iSegz;
        while(x.y <= ssp->z[iSegz] && iSegz > 0) --iSegz;
    }
}

HOST_DEVICE inline void UpdateRangeSegmentT(const vec2 &x, const vec2 &t,
    const SSPStructure *ssp, int32_t &iSegr)
{
    //LP: Handles edge cases based on which direction the ray is going. If the
    //ray takes a small step in the direction of t, it will remain in the same
    //segment as it is now.
    if(t.x >= RC(0.0)){
        //ssp->Seg.r[iSegr] <= x.x < ssp->Seg.r[iSegr+1]
        while(x.x < ssp->Seg.r[iSegr] && iSegr > 0) --iSegr;
        while(x.x >= ssp->Seg.r[iSegr+1] && iSegr < ssp->Nr-2) ++iSegr;
    }else{
        //ssp->Seg.r[iSegr] < x.x <= ssp->Seg.r[iSegr+1]
        while(x.x > ssp->Seg.r[iSegr+1] && iSegr < ssp->Nr-2) ++iSegr;
        while(x.x <= ssp->Seg.r[iSegr] && iSegr > 0) --iSegr;
    }
}

HOST_DEVICE inline real LinInterpDensity(const vec2 &x,
    const SSPStructure *ssp, const int32_t &iSegz, real &rho)
{
    real w = (x.y - ssp->z[iSegz]) / (ssp->z[iSegz+1] - ssp->z[iSegz]);
    rho = (RC(1.0) - w) * ssp->rho[iSegz] + w * ssp->rho[iSegz+1];
    return w;
}

/**
 * N2-linear interpolation of SSP data
 */
HOST_DEVICE inline void n2Linear(SSP_FN_ARGS)
{
    UpdateDepthSegmentT(x, t, ssp, iSegz);
    real w = LinInterpDensity(x, ssp, iSegz, rho);
    
    ccpx = RC(1.0) / STD::sqrt((RC(1.0) - w) * ssp->n2[iSegz] + w * ssp->n2[iSegz+1]);
    real c = ccpx.real();
    
    gradc = vec2(RC(0.0), RC(-0.5) * c * c * c * ssp->n2z[iSegz].real());
    crr = crz = RC(0.0);
    czz = RC(3.0) * gradc.y * gradc.y / c;
}

/**
 * c-linear interpolation of SSP data
 */
HOST_DEVICE inline void cLinear(SSP_FN_ARGS)
{
    UpdateDepthSegmentT(x, t, ssp, iSegz);
    LinInterpDensity(x, ssp, iSegz, rho);
    
    ccpx = ssp->c[iSegz] + (x.y - ssp->z[iSegz]) * ssp->cz[iSegz];
    gradc = vec2(RC(0.0), ssp->cz[iSegz].real());
    crr = crz = czz = RC(0.0);
}

/**
 * This implements the new monotone piecewise cubic Hermite interpolating
 * polynomial (PCHIP) algorithm for the interpolation of the sound speed c.
 */
HOST_DEVICE inline void cPCHIP(SSP_FN_ARGS)
{
    UpdateDepthSegmentT(x, t, ssp, iSegz);
    LinInterpDensity(x, ssp, iSegz, rho);
    
    real xt = x.y - ssp->z[iSegz];
    if(STD::abs(xt) > RC(1.0e10)){
        printf("Invalid xt %g\n", xt);
    }
    for(int32_t i=0; i<4; ++i)
        if(STD::abs(ssp->cCoef[i][iSegz]) > RC(1.0e10))
            printf("Invalid ssp->cCoef[%d][%d] = (%g,%g)\n", i, iSegz,
                ssp->cCoef[i][iSegz].real(), ssp->cCoef[i][iSegz].imag());
    
    ccpx = ssp->cCoef[0][iSegz]
        + (ssp->cCoef[1][iSegz]
        + (ssp->cCoef[2][iSegz]
        +  ssp->cCoef[3][iSegz] * xt) * xt) * xt;
    
    gradc = vec2(RC(0.0), (ssp->cCoef[1][iSegz]
              + (RC(2.0) * ssp->cCoef[2][iSegz]
              +  RC(3.0) * ssp->cCoef[3][iSegz] * xt) * xt).real());
    
    crr = crz = RC(0.0);
    czz = (RC(2.0) * ssp->cCoef[2][iSegz] + RC(6.0) * ssp->cCoef[3][iSegz] * xt).real();
}

/**
 * Cubic spline interpolation
 */
HOST_DEVICE inline void cCubic(SSP_FN_ARGS)
{
    UpdateDepthSegmentT(x, t, ssp, iSegz);
    LinInterpDensity(x, ssp, iSegz, rho);
    
    real hSpline = x.y - ssp->z[iSegz];
    cpx czcpx, czzcpx;
    
    SplineALL(ssp->cSpline[0][iSegz], ssp->cSpline[1][iSegz], ssp->cSpline[2][iSegz],
        ssp->cSpline[3][iSegz], hSpline, ccpx, czcpx, czzcpx);
    
    // LP: Only for these conversions, BELLHOP uses DBLE() instead of REAL().
    // The manual for DBLE simply says that it converts the argument to double
    // precision and complex is a valid input, but it doesn't say how that
    // conversion is done. Assuming it does real part rather than magnitude.
    gradc = vec2(RC(0.0), czcpx.real());
    crr = crz = RC(0.0);
    czz = czzcpx.real();
}

/**
 * Bilinear quadrilatteral interpolation of SSP data in 2D
 */
HOST_DEVICE inline void Quad(SSP_FN_ARGS)
{
    real c1, c2, cz1, cz2, cr, cz, s1, s2, delta_r, delta_z;
    
    if(x.x < ssp->Seg.r[0] || x.x > ssp->Seg.r[ssp->Nr-1]){
        printf("Quad: ray is outside the box where the soundspeed is defined\n");
        bail();
    }
    
    UpdateDepthSegmentT(x, t, ssp, iSegz);
    UpdateRangeSegmentT(x, t, ssp, iSegr);
    LinInterpDensity(x, ssp, iSegz, rho);
    if(iSegz >= ssp->Nz - 1 || iSegr >= ssp->Nr - 1){
        printf("iSeg error in Quad: z %d/%d r %d/%d\n",
            iSegz, ssp->Nz, iSegr, ssp->Nr);
        bail();
    }
    
    // for this depth, x.y, get the sound speed at both ends of the segment
    int32_t Nr = ssp->Nr;
    cz1 = ssp->czMat[iSegz*Nr + iSegr  ];
    cz2 = ssp->czMat[iSegz*Nr + iSegr+1];
    
    s2      = x.y - ssp->z[iSegz];
    delta_z = ssp->z[iSegz + 1] - ssp->z[iSegz];
    
    c1 = ssp->cMat[iSegz*Nr + iSegr  ] + s2 * cz1;
    c2 = ssp->cMat[iSegz*Nr + iSegr+1] + s2 * cz2;
    
    // s1 = proportional distance of x.x in range
    delta_r = ssp->Seg.r[iSegr+1] - ssp->Seg.r[iSegr];
    s1 = (x.x - ssp->Seg.r[iSegr]) / delta_r;
    s1 = math::min(s1, RC(1.0)); // force piecewise constant extrapolation for points outside the box
    s1 = math::max(s1, RC(0.0)); // "
    
    real c = (RC(1.0) - s1) * c1 + s1 * c2;
    
    // interpolate the attenuation !!!! This will use the wrong segment if the ssp in the envil is sampled at different depths
    s2 = s2 / delta_z; // convert to a proportional depth in the layer
    real cimag = ((RC(1.0) - s2) * ssp->c[iSegz] + s2 * ssp->c[iSegz+1]).imag(); // volume attenuation is taken from the single c(z) profile
    
    ccpx = cpx(c, cimag);
    
    cz = (RC(1.0) - s1) * cz1 + s1 * cz2;
    
    cr  = (c2  - c1 ) / delta_r;
    crz = (cz2 - cz1) / delta_r;
    
    gradc = vec2(cr, cz);
    crr = RC(0.0);
    czz = RC(0.0);
}

HOST_DEVICE inline void Analytic(SSP_FN_ARGS)
{
    real c0, cr, cz, DxtDz, xt;
    
    iSegz = 0;
    c0 = RC(1500.0);
    rho = RC(1.0);
    
    const real unk1 = RC(1300.0);
    const real unk2 = RC(0.00737);
    
    // homogeneous halfspace was removed since BELLHOP needs to get gradc just a little below the boundaries, on ray reflection
    
    //if(x.y < 5000.0){
    xt    = RC(2.0) * (x.y - unk1) / unk1;
    real emxt = STD::exp(-xt);
    DxtDz = RC(2.0) / unk1;
    ccpx  = cpx(c0 * (RC(1.0) + unk2 * (xt - RC(1.0) + emxt)), RC(0.0));
    cz    = c0 * unk2 * (RC(1.0) - emxt) * DxtDz;
    czz   = c0 * unk2 * emxt * SQ(DxtDz);
    //}else{
    // Homogeneous half-space
    //xt    = RC(2.0) * (5000.0 - unk1) / unk1;
    //ccpx  = cpx(c0 * (RC(1.0) + unk2 * (xt - RC(1.0) + emxt)), RC(0.0)); // LP: BUG: cimag never set on this codepath
    //cz    = RC(0.0);
    //czz   = RC(0.0);
    //}
    
    cr = RC(0.0);
    gradc = vec2(cr, cz);
    crz = RC(0.0);
    crr = RC(0.0);
}

HOST_DEVICE inline void EvaluateSSP(SSP_FN_ARGS)
{
    switch(ssp->Type){
    case 'N': // N2-linear profile option
        n2Linear(SSP_CALL_ARGS); break;
    case 'C': // C-linear profile option
        cLinear (SSP_CALL_ARGS); break;
    case 'P': // monotone PCHIP ACS profile option
        cPCHIP  (SSP_CALL_ARGS); break;
    case 'S': // Cubic spline profile option
        cCubic  (SSP_CALL_ARGS); break;
    case 'Q':
        Quad    (SSP_CALL_ARGS); break;
    /* case 'H':
        // this is called by BELLHOP3D only once, during READIN
        // possibly the logic should be changed to call EvaluateSSP2D or 3D
        x3 = vec3(RC(0.0), RC(0.0), x.y);
        Hexahedral(x3, c, cimag, gradc_3d, cxx, cyy, czz, cxy, cxz, cyz, rho, freq); break; */
    case 'A': // Analytic profile option
        Analytic(SSP_CALL_ARGS); break;
    default:
        printf("EvaluateSSP: Invalid profile option %c\n", ssp->Type);
        bail();
    }
}

HOST_DEVICE inline void EvaluateSSPCOnly(const vec2 &x, const vec2 &t, cpx &ccpx,
    real freq, const SSPStructure *ssp, int32_t &iSegz, int32_t &iSegr)
{
    vec2 gradc;
    real crr, crz, czz, rho;
    EvaluateSSP(SSP_CALL_ARGS);
}

void InitializeSSP(SSP_INIT_ARGS);
 
