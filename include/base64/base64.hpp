/**
 * @file base64.hpp
 * @brief Minimal Base64 and Base64URL encode/decode for modern C++.
 *
 * Header-only.
 *
 * Provides:
 *   - base64::encode(bytes)        (standard base64, with padding '=')
 *   - base64::decode(string)       (standard base64)
 *   - base64::encode_url(bytes)    (base64url, no padding)
 *   - base64::decode_url(string)   (base64url, accepts no padding)
 *
 * Notes:
 *   - Standard alphabet: A-Z a-z 0-9 + /
 *   - URL alphabet      : A-Z a-z 0-9 - _
 *
 * MIT License
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace base64
{
  namespace detail
  {
    static constexpr char kB64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static constexpr char kB64Url[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789-_";

    inline std::uint8_t decode_char(char c, bool url)
    {
      if (c >= 'A' && c <= 'Z')
        return static_cast<std::uint8_t>(c - 'A');
      if (c >= 'a' && c <= 'z')
        return static_cast<std::uint8_t>(c - 'a' + 26);
      if (c >= '0' && c <= '9')
        return static_cast<std::uint8_t>(c - '0' + 52);

      if (!url)
      {
        if (c == '+')
          return 62;
        if (c == '/')
          return 63;
      }
      else
      {
        if (c == '-')
          return 62;
        if (c == '_')
          return 63;
      }

      throw std::invalid_argument("invalid base64 character");
    }

    inline std::string encode_impl(const std::vector<std::uint8_t> &data,
                                   const char *alphabet,
                                   bool pad)
    {
      std::string out;
      out.reserve(((data.size() + 2) / 3) * 4);

      std::size_t i = 0;
      while (i + 3 <= data.size())
      {
        const std::uint32_t n =
            (static_cast<std::uint32_t>(data[i]) << 16) |
            (static_cast<std::uint32_t>(data[i + 1]) << 8) |
            (static_cast<std::uint32_t>(data[i + 2]));

        out.push_back(alphabet[(n >> 18) & 63]);
        out.push_back(alphabet[(n >> 12) & 63]);
        out.push_back(alphabet[(n >> 6) & 63]);
        out.push_back(alphabet[n & 63]);

        i += 3;
      }

      const std::size_t rem = data.size() - i;

      if (rem == 1)
      {
        const std::uint32_t n =
            (static_cast<std::uint32_t>(data[i]) << 16);

        out.push_back(alphabet[(n >> 18) & 63]);
        out.push_back(alphabet[(n >> 12) & 63]);

        if (pad)
        {
          out.push_back('=');
          out.push_back('=');
        }
      }
      else if (rem == 2)
      {
        const std::uint32_t n =
            (static_cast<std::uint32_t>(data[i]) << 16) |
            (static_cast<std::uint32_t>(data[i + 1]) << 8);

        out.push_back(alphabet[(n >> 18) & 63]);
        out.push_back(alphabet[(n >> 12) & 63]);
        out.push_back(alphabet[(n >> 6) & 63]);

        if (pad)
          out.push_back('=');
      }

      return out;
    }

    inline std::vector<std::uint8_t> decode_impl(const std::string &in, bool url)
    {
      if (in.empty())
        return {};

      // Copy and normalize input:
      // - base64: must be multiple of 4
      // - base64url: allow missing padding, we add it logically
      std::string s = in;

      if (url)
      {
        const std::size_t mod = s.size() % 4;
        if (mod == 1)
          throw std::invalid_argument("invalid base64url length");

        if (mod == 2)
          s.append("==");
        if (mod == 3)
          s.append("=");
      }
      else
      {
        if (s.size() % 4 != 0)
          throw std::invalid_argument("invalid base64 length");
      }

      // Count padding
      std::size_t pad = 0;
      if (!s.empty() && s.back() == '=')
        pad++;
      if (s.size() >= 2 && s[s.size() - 2] == '=')
        pad++;

      // '=' only allowed at the end, in last 2 positions
      for (std::size_t i = 0; i + 2 < s.size(); ++i)
      {
        if (s[i] == '=')
          throw std::invalid_argument("invalid padding position");
      }

      std::vector<std::uint8_t> out;
      out.reserve((s.size() / 4) * 3);

      for (std::size_t i = 0; i < s.size(); i += 4)
      {
        const char c0 = s[i];
        const char c1 = s[i + 1];
        const char c2 = s[i + 2];
        const char c3 = s[i + 3];

        const std::uint8_t b0 = detail::decode_char(c0, url);
        const std::uint8_t b1 = detail::decode_char(c1, url);

        const std::uint8_t b2 = (c2 == '=') ? 0 : detail::decode_char(c2, url);
        const std::uint8_t b3 = (c3 == '=') ? 0 : detail::decode_char(c3, url);

        const std::uint32_t n =
            (static_cast<std::uint32_t>(b0) << 18) |
            (static_cast<std::uint32_t>(b1) << 12) |
            (static_cast<std::uint32_t>(b2) << 6) |
            (static_cast<std::uint32_t>(b3));

        out.push_back(static_cast<std::uint8_t>((n >> 16) & 0xFF));
        if (c2 != '=')
          out.push_back(static_cast<std::uint8_t>((n >> 8) & 0xFF));
        if (c3 != '=')
          out.push_back(static_cast<std::uint8_t>(n & 0xFF));
      }

      // Extra strictness: if padding existed, ensure output size matches
      if (pad != 0)
      {
        const std::size_t expected = (s.size() / 4) * 3 - pad;
        if (out.size() != expected)
          throw std::invalid_argument("invalid base64 padding");
      }

      return out;
    }
  } // namespace detail

  inline std::string encode(const std::vector<std::uint8_t> &data)
  {
    return detail::encode_impl(data, detail::kB64, true);
  }

  inline std::vector<std::uint8_t> decode(const std::string &s)
  {
    return detail::decode_impl(s, false);
  }

  inline std::string encode_url(const std::vector<std::uint8_t> &data)
  {
    return detail::encode_impl(data, detail::kB64Url, false);
  }

  inline std::vector<std::uint8_t> decode_url(const std::string &s)
  {
    return detail::decode_impl(s, true);
  }

} // namespace base64
