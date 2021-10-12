cc_library(
  name = "libhttp",
  srcs = glob(["src/*.cpp"], exclude=["src/App.cpp"]),
  includes = ["include/"],
  hdrs = glob(["include/*.hpp"]),
  deps = ["@com_github_google_glog//:glog"],
)

cc_binary(
  name = "shttps",
  srcs = ["src/App.cpp"],
  includes = ["include"],
  deps = [
    ":libhttp",
    "@com_github_google_glog//:glog",
  ],
)