// Copyright (c) Darrin Stewart. All rights reserved.

#pragma once

#if defined(NDEBUG)
    #define DORF_ASSERT_ENABLED 0
#else
    #define DORF_ASSERT_ENABLED 1
#endif

#if DORF_ASSERT_ENABLED
    #include <cassert>
    // TODO: replace with custom assert handler
    #define DORF_ASSERT(expr) assert(expr)
#else
    #define DORF_ASSERT(expr) ((void)0)
#endif
