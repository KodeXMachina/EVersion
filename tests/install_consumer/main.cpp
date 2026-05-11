/**
 * @file main.cpp
 * @brief Minimal installed-package consumer used by EVersion tests.
 */

#include <eversion/runtime.h>
#include <install_consumer/version_info.h>

int main() {
  static_assert(install_consumer::version_info::kVersionMajor == 4U);
  static_assert(install_consumer::version_info::kVersionString == "4.5.6");

  constexpr eversion::RuntimeVersion kRuntimeVersion{
      .major = 7,
      .minor = 8,
      .patch = 9,
      .prerelease = "beta.1",
      .build_metadata = "",
  };
  static_assert(eversion::ToVersion(kRuntimeVersion) ==
                eversion::Version{7, 8, 9, "beta.1"});

  return install_consumer::version_info::kVersion.IsStable() ? 0 : 1;
}
