cc_library(
    name = "egret-lib",
    srcs = glob(
        ["*.cpp"],
        exclude = [
            "egret_ext.cpp",
            "main.cpp",
        ],
    ),
    hdrs = glob(["*.h"]),
    include_prefix = "egret",
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "egret",
    srcs = ["main.cpp"],
    deps = [":egret-lib"],
)
