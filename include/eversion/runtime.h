/**
 * @file runtime.h
 * @brief Defines ABI-stable runtime metadata for dynamically loaded plugins.
 */

#pragma once

#include <eversion/version.h>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace eversion {

/** @brief ABI version for the `ComponentInfo` runtime metadata contract. */
inline constexpr std::uint32_t kComponentInfoAbiVersion = 1U;
/** @brief Default exported symbol name for plugin metadata entry points. */
inline constexpr std::string_view kPluginInfoSymbol = "eversion_plugin_info";

/**
 * @brief ABI-stable version representation for dynamic-library boundaries.
 *
 * This type avoids C++ library types at the plugin boundary. Convert it to
 * `eversion::Version` with `ToVersion()` after validation.
 */
struct RuntimeVersion {
  /** @brief Major version component. */
  std::uint32_t major = 0;
  /** @brief Minor version component. */
  std::uint32_t minor = 0;
  /** @brief Patch version component. */
  std::uint32_t patch = 0;
  /** @brief Optional prerelease identifier, or an empty string. */
  const char* prerelease = "";
  /** @brief Optional build metadata, or an empty string. */
  const char* build_metadata = "";
};

/**
 * @brief ABI-stable component metadata returned by a plugin entry point.
 */
struct ComponentInfo {
  /** @brief Runtime metadata ABI version. */
  std::uint32_t abi_version = 0;
  /** @brief Size of this structure as compiled by the producer. */
  std::size_t struct_size = 0;
  /** @brief Stable machine-readable component or plugin identifier. */
  const char* id = "";
  /** @brief Human-readable component or plugin name. */
  const char* name = "";
  /** @brief Component or plugin version. */
  RuntimeVersion version;
};

/**
 * @brief Function pointer type for a plugin metadata entry point.
 *
 * The function returns a pointer to static metadata owned by the plugin.
 */
using ComponentInfoFunction = const ComponentInfo* (*)() noexcept;

namespace detail {

[[nodiscard]]
constexpr std::string_view StringViewOrEmpty(const char* text) noexcept {
  return text == nullptr ? std::string_view{} : std::string_view{text};
}

}  // namespace detail

/**
 * @brief Converts an ABI-stable runtime version to `eversion::Version`.
 *
 * Null string pointers are treated as empty strings.
 *
 * @param version Runtime version metadata to convert.
 * @return A regular EVersion value.
 */
[[nodiscard]]
constexpr Version ToVersion(const RuntimeVersion& version) noexcept {
  return Version{
      .major = version.major,
      .minor = version.minor,
      .patch = version.patch,
      .prerelease = detail::StringViewOrEmpty(version.prerelease),
      .build_metadata = detail::StringViewOrEmpty(version.build_metadata),
  };
}

/**
 * @brief Validates basic ABI invariants for component metadata.
 *
 * @param info Metadata pointer returned by a plugin entry point.
 * @return `true` when @p info is non-null and matches the supported ABI.
 */
[[nodiscard]]
constexpr bool IsValidComponentInfo(const ComponentInfo* info) noexcept {
  return info != nullptr && info->abi_version == kComponentInfoAbiVersion &&
         info->struct_size >= sizeof(ComponentInfo) && info->id != nullptr &&
         info->name != nullptr && info->version.prerelease != nullptr &&
         info->version.build_metadata != nullptr;
}

}  // namespace eversion
