#ifndef BASE64_FFI_H
#define BASE64_FFI_H

#include <vix/ffi/vix_ffi.h>

VIX_FFI_EXTERN_C
{

  /* Returns a static string literal, do not free. */
  VIX_FFI_API vix_str VIX_FFI_CALL base64_ffi_version(vix_status * out_status);

  /*
    Encodes bytes to base64 ASCII bytes (not null-terminated).
    If out_cap is too small, returns error and sets *out_len to required size.
  */
  VIX_FFI_API int VIX_FFI_CALL base64_encode(
      const uint8_t *in_ptr,
      size_t in_len,
      uint8_t *out_ptr,
      size_t out_cap,
      size_t *out_len,
      vix_status *out_status);

  /*
    Decodes base64 ASCII bytes to raw bytes.
    If out_cap is too small, returns error and sets *out_len to required size.
  */
  VIX_FFI_API int VIX_FFI_CALL base64_decode(
      const uint8_t *in_ptr,
      size_t in_len,
      uint8_t *out_ptr,
      size_t out_cap,
      size_t *out_len,
      vix_status *out_status);

} /* extern "C" */

#endif
