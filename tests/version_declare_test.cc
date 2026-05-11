/**
 * @file version_declare_test.cpp
 * @brief Tests generated target version constants.
 */

#include <eversion/version.h>
#include <eversion_demo/version_info.h>

#include <iostream>
#include <string_view>

bool RunEversionVersionTests();
bool RunEversionRuntimeTests();

namespace {

bool Expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
  }
  return condition;
}

bool RunDeclareTests() {
  bool ok = true;

  ok &= Expect(eversion_demo::version_info::kVersionMajor == 1U,
               "generated major version");
  ok &= Expect(eversion_demo::version_info::kVersionMinor == 2U,
               "generated minor version");
  ok &= Expect(eversion_demo::version_info::kVersionPatch == 3U,
               "generated patch version");
  ok &= Expect(eversion_demo::version_info::kVersionPrerelease == "rc.1",
               "generated prerelease");
  ok &= Expect(
      eversion_demo::version_info::kVersionBuildMetadata == "git.deadbeef",
      "generated build metadata");
  ok &= Expect(
      eversion_demo::version_info::kVersionString == "1.2.3-rc.1+git.deadbeef",
      "generated version string");
  ok &= Expect(eversion_demo::version_info::kVersion ==
                   eversion::Version{1, 2, 3, "rc.1", "other.build"},
               "generated version participates in version comparison");

  return ok;
}

static_assert(eversion_demo::version_info::kVersionMajor == 1U);
static_assert(eversion_demo::version_info::kVersionString ==
              "1.2.3-rc.1+git.deadbeef");
static_assert(eversion_demo::version_info::kVersion <
              eversion::Version{1, 2, 3});

}  // namespace

int main() {
  const bool ok = RunEversionVersionTests() && RunEversionRuntimeTests() &&
                  RunDeclareTests();
  return ok ? 0 : 1;
}
