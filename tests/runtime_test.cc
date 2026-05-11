/**
 * @file runtime_test.cpp
 * @brief Tests runtime metadata conversion and validation.
 */

#include <eversion/runtime.h>

#include <iostream>
#include <string_view>

namespace {

bool Expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
  }
  return condition;
}

constexpr eversion::RuntimeVersion kRuntimeVersion{
    .major = 3,
    .minor = 4,
    .patch = 5,
    .prerelease = "rc.1",
    .build_metadata = "git.abc",
};

constexpr eversion::ComponentInfo kComponentInfo{
    .abi_version = eversion::kComponentInfoAbiVersion,
    .struct_size = sizeof(eversion::ComponentInfo),
    .id = "test.component",
    .name = "Test Component",
    .version = kRuntimeVersion,
};

bool RunRuntimeTests() {
  bool ok = true;

  const eversion::Version version = eversion::ToVersion(kRuntimeVersion);
  ok &= Expect(version.major == 3U, "runtime major converts to Version");
  ok &= Expect(version.minor == 4U, "runtime minor converts to Version");
  ok &= Expect(version.patch == 5U, "runtime patch converts to Version");
  ok &= Expect(version.prerelease == "rc.1",
               "runtime prerelease converts to Version");
  ok &= Expect(version.build_metadata == "git.abc",
               "runtime build metadata converts to Version");
  ok &= Expect(eversion::IsValidComponentInfo(&kComponentInfo),
               "component info validates");
  ok &= Expect(!eversion::IsValidComponentInfo(nullptr),
               "null component info is invalid");

  eversion::ComponentInfo wrong_abi = kComponentInfo;
  wrong_abi.abi_version = eversion::kComponentInfoAbiVersion + 1U;
  ok &= Expect(!eversion::IsValidComponentInfo(&wrong_abi),
               "wrong ABI version is invalid");

  eversion::ComponentInfo wrong_size = kComponentInfo;
  wrong_size.struct_size = sizeof(eversion::ComponentInfo) - 1U;
  ok &= Expect(!eversion::IsValidComponentInfo(&wrong_size),
               "too-small component info is invalid");

  return ok;
}

static_assert(eversion::ToVersion(kRuntimeVersion) ==
              eversion::Version{3, 4, 5, "rc.1", "other.build"});
static_assert(eversion::IsValidComponentInfo(&kComponentInfo));
static_assert(!eversion::IsValidComponentInfo(nullptr));

}  // namespace

bool RunEversionRuntimeTests() { return RunRuntimeTests(); }
