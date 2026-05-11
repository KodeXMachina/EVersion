/**
 * @file version_test.cpp
 * @brief Tests the EVersion value type and precedence comparison.
 */

#include <eversion/version.h>

#include <compare>
#include <iostream>
#include <string_view>

namespace {

constexpr eversion::Version V(std::uint32_t major, std::uint32_t minor,
                              std::uint32_t patch,
                              std::string_view prerelease = {},
                              std::string_view build_metadata = {}) {
  return eversion::Version{major, minor, patch, prerelease, build_metadata};
}

bool Expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
  }
  return condition;
}

bool RunVersionTests() {
  bool ok = true;

  ok &= Expect(V(1, 2, 3) == V(1, 2, 3), "same versions compare equal");
  ok &= Expect(V(1, 2, 3) != V(1, 2, 4),
               "different patch versions are not equal");
  ok &= Expect(V(1, 2, 3, {}, "git.aaa") == V(1, 2, 3, {}, "git.bbb"),
               "build metadata is ignored for precedence equality");
  ok &= Expect(V(1, 0, 0, "alpha") < V(1, 0, 0),
               "prerelease ranks below final release");
  ok &= Expect(V(1, 0, 0, "alpha") < V(1, 0, 0, "alpha.1"),
               "shorter prerelease identifier list ranks lower");
  ok &= Expect(V(1, 0, 0, "alpha.1") < V(1, 0, 0, "alpha.beta"),
               "numeric prerelease identifiers rank below text identifiers");
  ok &= Expect(V(1, 0, 0, "beta.2") < V(1, 0, 0, "beta.11"),
               "numeric prerelease identifiers compare numerically");
  ok &= Expect(!V(0, 9, 0).IsStable(), "0.x releases are not stable");
  ok &= Expect(!V(1, 0, 0, "rc.1").IsStable(),
               "prerelease versions are not stable");
  ok &= Expect(V(1, 0, 0).IsStable(), "1.0.0 is stable");
  ok &= Expect(V(1, 2, 3, "rc.1", "git.deadbeef").ToString() ==
                   "1.2.3-rc.1+git.deadbeef",
               "version string includes prerelease and build metadata");

  return ok;
}

static_assert(V(1, 2, 3) == V(1, 2, 3));
static_assert(V(1, 0, 0, "alpha") < V(1, 0, 0));
static_assert(V(1, 2, 3, {}, "x") == V(1, 2, 3, {}, "y"));
static_assert(V(1, 0, 0).IsStable());

}  // namespace

bool RunEversionVersionTests() { return RunVersionTests(); }
