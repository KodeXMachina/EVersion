/**
 * @file semver_precedence.h
 * @brief SemVer 2.0.0 precedence comparison primitives.
 *
 * Provides `constexpr` building blocks for comparing prerelease identifiers
 * according to the Semantic Versioning 2.0.0 precedence rules:
 *   - numeric identifiers compare by value, with leading zeros stripped;
 *   - alphanumeric identifiers compare lexicographically (ASCII);
 *   - numeric identifiers always have lower precedence than alphanumeric;
 *   - a smaller set of dot-separated identifiers has lower precedence when
 *     all preceding identifiers are equal.
 *
 * These primitives are header-only so that callers can use them in constant
 * expressions.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <compare>
#include <cstddef>
#include <string_view>

namespace eversion::detail {

/**
 * @brief  Returns whether @p value is an ASCII decimal digit (`0`-`9`).
 *
 * Locale-independent replacement for `std::isdigit`, usable in constant
 * expressions.
 *
 * @param  value  Character to test.
 * @return        `true` iff @p value is in the range `'0'..'9'`.
 */
constexpr bool IsAsciiDigit(char value) noexcept {
  return value >= '0' && value <= '9';
}

/**
 * @brief  Returns whether @p text is a SemVer numeric identifier.
 *
 * A numeric identifier is a non-empty sequence of ASCII digits. The empty
 * string is not numeric.
 *
 * @param  text  Identifier to test (no surrounding dots).
 * @return       `true` iff @p text is non-empty and contains only digits.
 */
constexpr bool IsNumericIdentifier(std::string_view text) noexcept {
  if (text.empty()) {
    return false;
  }
  return std::all_of(text.begin(), text.end(),
                     [](char c) { return IsAsciiDigit(c); });
}

/**
 * @brief  Strips leading `'0'` characters from @p text, keeping at least one.
 *
 * Used to normalise numeric identifiers before length-based comparison so that
 * `"007"` and `"7"` compare equal.
 *
 * @param  text  Numeric identifier to normalise.
 * @return       View into @p text with leading zeros removed; never empty if
 *               @p text was non-empty.
 */
constexpr std::string_view StripLeadingZeros(std::string_view text) noexcept {
  std::size_t offset = 0;
  while (offset + 1 < text.size() && text[offset] == '0') {
    ++offset;
  }
  return text.substr(offset);
}

/**
 * @brief  Compares two SemVer numeric identifiers by numeric value.
 *
 * Implemented via length-then-lexicographic comparison after stripping leading
 * zeros, which is equivalent to numeric comparison without converting to an
 * integer (and therefore tolerates arbitrarily large identifiers).
 *
 * @param  left   Left operand; must satisfy IsNumericIdentifier().
 * @param  right  Right operand; must satisfy IsNumericIdentifier().
 * @return        Strong ordering between the two numeric values.
 */
constexpr std::strong_ordering CompareNumericIdentifiers(
    std::string_view left, std::string_view right) noexcept {
  const std::string_view normalized_left = StripLeadingZeros(left);
  const std::string_view normalized_right = StripLeadingZeros(right);

  if (normalized_left.size() != normalized_right.size()) {
    return normalized_left.size() <=> normalized_right.size();
  }
  return normalized_left <=> normalized_right;
}

/**
 * @brief  Compares two SemVer prerelease identifiers.
 *
 * Applies SemVer 2.0.0 precedence rules:
 *   - if both are numeric, compare numerically;
 *   - numeric identifiers always have lower precedence than alphanumeric;
 *   - otherwise compare lexicographically (ASCII).
 *
 * @param  left   Left identifier (no surrounding dots).
 * @param  right  Right identifier (no surrounding dots).
 * @return        Strong ordering between @p left and @p right.
 */
constexpr std::strong_ordering CompareIdentifier(std::string_view left,
                                                 std::string_view right) {
  const bool left_is_numeric = IsNumericIdentifier(left);
  const bool right_is_numeric = IsNumericIdentifier(right);

  if (left_is_numeric && right_is_numeric) {
    return CompareNumericIdentifiers(left, right);
  }
  if (left_is_numeric) return std::strong_ordering::less;
  if (right_is_numeric) return std::strong_ordering::greater;
  return left <=> right;
}

/**
 * @brief  Returns the first dot-separated identifier of @p text.
 *
 * @param  text  Dot-separated identifier list (e.g. `"alpha.1.foo"`).
 * @return       View of the substring before the first `'.'`, or all of
 *               @p text if no dot is present.
 */
constexpr std::string_view FirstIdentifier(std::string_view text) noexcept {
  const std::size_t dot = text.find('.');
  if (dot == std::string_view::npos) return text;
  return text.substr(0, dot);
}

/**
 * @brief  Returns @p text with its first dot-separated identifier removed.
 *
 * @param  text  Dot-separated identifier list.
 * @return       View of the substring after the first `'.'`, or an empty view
 *               if no dot is present.
 */
constexpr std::string_view RemoveFirstIdentifier(
    std::string_view text) noexcept {
  const std::size_t dot = text.find('.');
  if (dot == std::string_view::npos) return {};
  return text.substr(dot + 1);
}

/**
 * @brief  Compares two SemVer prerelease strings by precedence.
 *
 * Per SemVer 2.0.0:
 *   - a version *with* prerelease has lower precedence than the same version
 *     *without* one (so an empty string compares **greater**);
 *   - otherwise identifiers are compared pairwise left-to-right via
 *     CompareIdentifier();
 *   - if all leading identifiers are equal, the shorter list has lower
 *     precedence.
 *
 * @param  left   Prerelease string for the left version (no leading `-`).
 * @param  right  Prerelease string for the right version (no leading `-`).
 * @return        Strong ordering of the two prerelease strings.
 */
constexpr std::strong_ordering ComparePrerelease(
    std::string_view left, std::string_view right) noexcept {
  if (left.empty() && right.empty()) return std::strong_ordering::equal;
  if (left.empty()) return std::strong_ordering::greater;
  if (right.empty()) return std::strong_ordering::less;

  while (!left.empty() && !right.empty()) {
    const std::strong_ordering order =
        CompareIdentifier(FirstIdentifier(left), FirstIdentifier(right));
    if (order != std::strong_ordering::equal) return order;

    left = RemoveFirstIdentifier(left);
    right = RemoveFirstIdentifier(right);
  }

  if (left.empty() && right.empty()) return std::strong_ordering::equal;
  return left.empty() ? std::strong_ordering::less
                      : std::strong_ordering::greater;
}

}  // namespace eversion::detail
