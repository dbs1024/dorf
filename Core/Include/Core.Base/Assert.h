// Copyright (c) Darrin Stewart. All rights reserved.

#pragma once

#if defined(NDEBUG)
    #define ACE_ASSERT_ENABLED 0
#else
    #define ACE_ASSERT_ENABLED 1
#endif

#if ACE_ASSERT_ENABLED
    #include <cassert>
    // TODO: replace with custom assert handler
    #define ACE_ASSERT(expr) assert(expr)
#else
    #define ACE_ASSERT(expr) ((void)0)
#endif
