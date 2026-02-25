#include <cstring>
#include <string>
#include <vector>

#include <base64/base64.hpp>
#include <base64/base64_ffi.h>

static vix_str make_str_lit(const char *s)
{
  vix_str out;
  out.ptr = s;
  out.len = std::strlen(s);
  return out;
}

static void set_error(vix_status *st, int32_t code, const char *msg)
{
  if (!st)
    return;
  st->code = code;
  st->message = make_str_lit(msg);
}

static void set_ok(vix_status *st)
{
  if (!st)
    return;
  *st = vix_status_ok();
}

vix_str VIX_FFI_CALL base64_ffi_version(vix_status *out_status)
{
  set_ok(out_status);
  return make_str_lit("1.0.0");
}

int VIX_FFI_CALL base64_encode(
    const uint8_t *in_ptr,
    size_t in_len,
    uint8_t *out_ptr,
    size_t out_cap,
    size_t *out_len,
    vix_status *out_status)
{
  set_ok(out_status);

  if (!out_len)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "out_len is null");
    return 1;
  }

  if (!in_ptr && in_len != 0)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "in_ptr is null but in_len != 0");
    return 1;
  }

  if (!out_ptr && out_cap != 0)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "out_ptr is null but out_cap != 0");
    return 1;
  }

  try
  {
    // base64::encode expects a vector (per compiler error), not (ptr,len).
    std::vector<uint8_t> input;
    input.reserve(in_len);
    if (in_len != 0)
      input.insert(input.end(), in_ptr, in_ptr + in_len);

    const std::string encoded = base64::encode(input);

    if (out_cap < encoded.size())
    {
      *out_len = encoded.size();
      set_error(out_status, VIX_STATUS_BUFFER_TOO_SMALL, "output buffer too small");
      return 1;
    }

    if (!encoded.empty())
      std::memcpy(out_ptr, encoded.data(), encoded.size());

    *out_len = encoded.size();
    return 0;
  }
  catch (const std::exception &e)
  {
    set_error(out_status, VIX_STATUS_ERROR, e.what());
    return 1;
  }
  catch (...)
  {
    set_error(out_status, VIX_STATUS_ERROR, "unknown native exception");
    return 1;
  }
}

int VIX_FFI_CALL base64_decode(
    const uint8_t *in_ptr,
    size_t in_len,
    uint8_t *out_ptr,
    size_t out_cap,
    size_t *out_len,
    vix_status *out_status)
{
  set_ok(out_status);

  if (!out_len)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "out_len is null");
    return 1;
  }

  if (!in_ptr && in_len != 0)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "in_ptr is null but in_len != 0");
    return 1;
  }

  if (!out_ptr && out_cap != 0)
  {
    set_error(out_status, VIX_STATUS_INVALID_ARG, "out_ptr is null but out_cap != 0");
    return 1;
  }

  try
  {
    const std::string input(reinterpret_cast<const char *>(in_ptr), in_len);
    const std::vector<uint8_t> decoded = base64::decode(input);

    if (out_cap < decoded.size())
    {
      *out_len = decoded.size();
      set_error(out_status, VIX_STATUS_BUFFER_TOO_SMALL, "output buffer too small");
      return 1;
    }

    if (!decoded.empty())
      std::memcpy(out_ptr, decoded.data(), decoded.size());

    *out_len = decoded.size();
    return 0;
  }
  catch (const std::exception &e)
  {
    set_error(out_status, VIX_STATUS_ERROR, e.what());
    return 1;
  }
  catch (...)
  {
    set_error(out_status, VIX_STATUS_ERROR, "unknown native exception");
    return 1;
  }
}
