
/*
THIS FILE IS AUTO-GENERATED BY REGMAKER. DO NOT EDIT MANUALLY (unless you really want to)

This file is an auto-generated regression test. It makes sure that the given
regex does not crash egret
*/

#include <gtest/gtest.h>
#include "src/egret.h"

TEST(RegressionTest, test_regtest8) {
  auto regex = "^(..?;)*([^147]|1[^2]|4[^5]|7[^8]|12[^3]|45[^6]|[14]$|12$|45$)";
  EXPECT_NO_FATAL_FAILURE(run_engine(regex, "evil"));
}
