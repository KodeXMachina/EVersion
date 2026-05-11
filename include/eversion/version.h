/**
 * @file version.h
 * @brief Defines EVersion's version value type and comparison helpers.
 */

#pragma once

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

#include "eversion/detail/semver_precedence.h"

namespace eversion {

struct Version;

/**
 * @brief Formats a version as `MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]`.
 *
 * @param version Version value to format.
 * @return A canonical string representation of @p version.
 */
[[nodiscard]] std::string ToString(const Version& version);

/**
 * @brief Version value used by generated headers and runtime metadata.
 *
 * `Version` is intentionally lightweight and can be used in constant
 * expressions. Build metadata is kept for display, but ignored by comparison
 * operators.
 */
struct Version {
  /** @brief Major version component. */
  std::uint32_t major = 0;
  /** @brief Minor version component. */
  std::uint32_t minor = 0;
  /** @brief Patch version component. */
  std::uint32_t patch = 0;
  /** @brief Optional prerelease identifier without the leading hyphen. */
  std::string_view prerelease;
  /** @brief Optional build metadata without the leading plus sign. */
  std::string_view build_metadata;

  /**
   * @brief Returns whether this version represents a stable release.
   *
   * EVersion treats `0.x.y` versions and prerelease versions as unstable.
   */
  [[nodiscard]]
  constexpr bool IsStable() const noexcept {
    return major > 0 && prerelease.empty();
  }

  /** @brief Returns whether the version has prerelease metadata. */
  [[nodiscard]]
  constexpr bool HasPrerelease() const noexcept {
    return !prerelease.empty();
  }

  /** @brief Returns whether the version has build metadata. */
  [[nodiscard]]
  constexpr bool HasBuildMetadata() const noexcept {
    return !build_metadata.empty();
  }

  /** @brief Formats this version as a canonical string. */
  [[nodiscard]]
  std::string ToString() const;

  /**
   * @brief Compares two versions by release precedence.
   *
   * Build metadata is ignored for precedence.
   */
  friend constexpr std::strong_ordering operator<=>(
      const Version& left, const Version& right) noexcept;
  /**
   * @brief Returns whether two versions have equal precedence.
   *
   * Build metadata is ignored for equality.
   */
  friend constexpr bool operator==(const Version& left,
                                   const Version& right) noexcept;
};

[[nodiscard]] constexpr std::strong_ordering operator<=>(
    const Version& left, const Version& right) noexcept {
  if (const std::strong_ordering order = left.major <=> right.major;
      order != std::strong_ordering::equal) {
    return order;
  }
  if (const std::strong_ordering order = left.minor <=> right.minor;
      order != std::strong_ordering::equal) {
    return order;
  }
  if (const std::strong_ordering order = left.patch <=> right.patch;
      order != std::strong_ordering::equal) {
    return order;
  }
  return detail::ComparePrerelease(left.prerelease, right.prerelease);
}

[[nodiscard]] constexpr bool operator==(const Version& left,
                                        const Version& right) noexcept {
  return (left <=> right) == std::strong_ordering::equal;
}

inline std::string ToString(const Version& version) {
  std::string text = std::to_string(version.major);
  text.push_back('.');
  text += std::to_string(version.minor);
  text.push_back('.');
  text += std::to_string(version.patch);

  if (!version.prerelease.empty()) {
    text.push_back('-');
    text.append(version.prerelease.data(), version.prerelease.size());
  }
  if (!version.build_metadata.empty()) {
    text.push_back('+');
    text.append(version.build_metadata.data(), version.build_metadata.size());
  }
  return text;
}

inline std::string Version::ToString() const {
  return eversion::ToString(*this);
}

}  // namespace eversion
