/*
 * Created by Brett Terpstra 6920201 on 20/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef CMAKE_CONFIG

// Are we building with debug (engine tools) or release (no engine tools)
/* #undef DEBUG_ENABLED */
#define DEBUG_ENABLED_BOOL false
// are we building with release or debug mode?
/* #undef COMPILER_DEBUG_ENABLED */
#define COMPILER_DEBUG_ENABLED_BOOL false

/* #undef COMPILE_GUI */
/* #undef COMPILE_OPENCL */
#define USE_GLFW
#define USE_OPENMP
/* #undef USE_MPI */


#define CMAKE_CONFIG
#endif
