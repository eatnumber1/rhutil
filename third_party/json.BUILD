cc_library(
    name = "json",
    hdrs = [
        "nlohmann/json_fwd.hpp",
        "nlohmann/json.hpp",
    ],
    srcs = glob(
        include = ["**/*.hpp"],
        exclude = [
            "nlohmann/json_fwd.hpp",
            "nlohmann/json.hpp",
        ],
    ),
    includes = ["."],
    visibility = ["//visibility:public"],
)
