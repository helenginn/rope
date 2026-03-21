//
// Created by dtriand on 21/03/2026.
//

#ifndef ROPE_COMPATIBILITY_H
#define ROPE_COMPATIBILITY_H

// Detect compiler
#ifdef _MSC_VER
    #define ROPE_COMPILER_MSVC 1
#else
    #define ROPE_COMPILER_MSVC 0
#endif

// Microsoft Visual C++ checks
#if ROPE_COMPILER_MSVC
    // Define _USE_MATH_DEFINES to get M_PI and other math constants
    // Issue arising from <cmath>
    #ifndef _USE_MATH_DEFINES
        #define _USE_MATH_DEFINES
    #endif

    // Define NOMINMAX to prevent Windows headers from defining min and max macros
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

#endif


#endif //ROPE_COMPATIBILITY_H