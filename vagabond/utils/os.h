//
// Created by Dimitris on 23/08/2022.
//

#ifndef ROPE_OS_H
#define ROPE_OS_H

#ifdef _WIN64
    #define OS_WINDOWS
#elif _WIN32
    #define OS_WINDOWS
#elif __APPLE__
#include "TargetConditionals.h"
    #if TARGET_OS_IPHONE && TARGET_OS_SIMULATOR
        // define something for simulator
        // (although, checking for TARGET_OS_IPHONE should not be required).
    #elif TARGET_OS_IPHONE && TARGET_OS_MACCATALYST
        // define something for Mac's Catalyst
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
        #define OS_MACOSX
        #define OS_UNIX
    #endif
#elif __linux
    #define OS_LINUX
    #define OS_UNIX
#elif __unix // all unices not caught above
    #define OS_UNIX
#elif __posix
    #define OS_INVALID
#endif


#endif //ROPE_OS_H
