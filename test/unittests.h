#pragma once

#include <unittesting.h>

#define EXPECT_FLOAT_EQ_DELTA(a, b, d) ASSERT(FLOAT_EQ_DELTA(a, b, d))
#define EXPECT_FLOAT_EQ(a, b) EXPECT_FLOAT_EQ_DELTA(a, b, 1e-6)
