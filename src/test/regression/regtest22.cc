
/*
THIS FILE IS AUTO-GENERATED BY REGMAKER. DO NOT EDIT MANUALLY (unless you really want to)

This file is an auto-generated regression test. It makes sure that the given
regex does not crash egret
*/

#include <gtest/gtest.h>
#include "src/egret.h"

TEST(RegressionTest, test_regtest22) {
  auto regex = "\\$?(?:\\.\\d+|(?:\\d{4,}|\\d{1,3}(?:,\\d{3})*)(?:\\.\\d+)?)";
  EXPECT_NO_FATAL_FAILURE(run_engine(regex, "evil"));
}
