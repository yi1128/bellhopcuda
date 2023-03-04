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
#include "common.hpp"

static bhc::bhcInit init;

template<bool O3D, bool R3D> int mainmain()
{
    bhc::bhcParams<O3D, R3D> params;
    bhc::bhcOutputs<O3D, R3D> outputs;
    if(!bhc::setup<O3D, R3D>(init, params, outputs)) return 1;
    if(!bhc::run<O3D, R3D>(params, outputs)) return 1;
    if(!bhc::writeout<O3D, R3D>(params, outputs)) return 1;
    bhc::finalize<O3D, R3D>(params, outputs);
    return 0;
}

int main(int argc, char **argv)
{
    int dimmode = BHC_DIM_ONLY;
    std::string FileRoot;
    for(int32_t i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if(argv[i][0] == '-') {
            if(s.length() >= 2 && argv[i][1] == '-') { // two dashes
                s = s.substr(1);
            }
            if(s == "-1" || s == "-singlethread") {
                init.numThreads = 1;
            } else if(s == "-2" || s == "-2D") {
                dimmode = 2;
            } else if(s == "-Nx2D" || s == "-2D3D" || s == "-2.5D" || s == "-4") {
                dimmode = 4;
            } else if(s == "-3" || s == "-3D") {
                dimmode = 3;
            } else {
                size_t equalspos = s.find("=");
                if(equalspos == std::string::npos) {
                    std::cout << "Unknown command-line option \"" << s << "\"\n";
                    std::abort();
                }
                std::string key   = s.substr(0, equalspos);
                std::string value = s.substr(equalspos + 1);
                if(key == "-gpu") {
                    if(!bhc::isInt(value, false)) {
                        std::cout << "Value \"" << value
                                  << "\" for --gpu argument is invalid\n";
                        std::abort();
                    }
                    init.gpuIndex = std::stoi(value);
                } else if(key == "-mem" || key == "--memory") {
                    size_t multiplier = 1u;
                    size_t base       = 1000u;
                    if(bhc::endswith(value, "B") || bhc::endswith(value, "b")) {
                        value = value.substr(0, value.length() - 1);
                    }
                    if(bhc::endswith(value, "i")) {
                        base  = 1024u;
                        value = value.substr(0, value.length() - 1);
                    }
                    if(bhc::endswith(value, "k") || bhc::endswith(value, "K")) {
                        multiplier = base;
                        value      = value.substr(0, value.length() - 1);
                    } else if(bhc::endswith(value, "M")) {
                        multiplier = base * base;
                        value      = value.substr(0, value.length() - 1);
                    } else if(bhc::endswith(value, "G")) {
                        multiplier = base * base * base;
                        value      = value.substr(0, value.length() - 1);
                    }
                    if(!bhc::isInt(value, false)) {
                        std::cout << "Value \"" << value
                                  << "\" for --memory argument is invalid\n";
                        std::abort();
                    }
                    init.maxMemory = multiplier * std::stoi(value);
                } else {
                    std::cout << "Unknown command-line option \"-" << key << "=" << value
                              << "\"\n";
                    std::abort();
                }
            }
        } else {
            if(FileRoot.empty()) {
                FileRoot = s;
            } else {
                std::cout << "Intepreting both \"" << FileRoot << "\" and \"" << s
                          << "\" as FileRoot, error\n";
                std::abort();
            }
        }
    }
    if(FileRoot.empty()) {
        std::cout << "Must provide FileRoot as command-line parameter\n";
        std::abort();
    }
    init.FileRoot = FileRoot.c_str();

#if BHC_DIM_ONLY > 0
    if(dimmode != BHC_DIM_ONLY) {
        std::cout << "This version of " BHC_PROGRAMNAME " was compiled to only support ";
        if(BHC_DIM_ONLY == 4) {
            std::cout << "Nx2D";
        } else {
            std::cout << BHC_DIM_ONLY << "D";
        }
        std::cout << " runs\n";
        std::abort();
    }
#else
    if(dimmode < 2 || dimmode > 4) {
        std::cout << "No dimensionality specified (--2D, --Nx2D, --3D), assuming 2D\n";
        dimmode = 2;
    }
#endif

    if(dimmode == 2) {
#if BHC_ENABLE_2D
        return mainmain<false, false>();
#else
        std::cout << "This version of " BHC_PROGRAMNAME
                     " was compiled with 2D support disabled\n";
#endif
    }
    if(dimmode == 3) {
#if BHC_ENABLE_3D
        return mainmain<true, true>();
#else
        std::cout << "This version of " BHC_PROGRAMNAME
                     " was compiled with 3D support disabled\n";
#endif
    }
    if(dimmode == 4) {
#if BHC_ENABLE_NX2D
        return mainmain<true, false>();
#else
        std::cout << "This version of " BHC_PROGRAMNAME
                     " was compiled with Nx2D support disabled\n";
#endif
    }
    std::abort();
}
