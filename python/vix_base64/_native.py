import ctypes
import os
import sys
from pathlib import Path


# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------
def _is_windows() -> bool:
    return sys.platform.startswith("win")


def _is_macos() -> bool:
    return sys.platform == "darwin"


def _default_lib_filenames() -> list:
    # We set OUTPUT_NAME base64 in CMake, so the library base name is "base64".
    # On Linux it becomes libbase64.so (and friends).
    if _is_windows():
        return ["base64.dll"]
    if _is_macos():
        return ["libbase64.dylib", "base64.dylib"]
    return ["libbase64.so", "libbase64.so.1", "libbase64.so.1.0.0"]


def _split_paths(env_value: str) -> list:
    if not env_value:
        return []
    sep = ";" if _is_windows() else ":"
    return [p.strip() for p in env_value.split(sep) if p.strip()]


def _candidate_dirs() -> list:
    # 1) VIX_LIB_PATH (colon-separated on Linux/macOS, semicolon on Windows)
    dirs = []
    dirs.extend(_split_paths(os.environ.get("VIX_LIB_PATH", "")))

    # 2) package directory (next to this file)
    dirs.append(str(Path(__file__).resolve().parent))

    # 3) current working directory (small convenience)
    dirs.append(str(Path.cwd()))

    # Keep order, unique
    out = []
    seen = set()
    for d in dirs:
        if d not in seen:
            seen.add(d)
            out.append(d)
    return out


def _load_library() -> ctypes.CDLL:
    filenames = _default_lib_filenames()
    dirs = _candidate_dirs()

    last_err = None
    for d in dirs:
        for name in filenames:
            p = Path(d) / name
            if p.exists():
                try:
                    return ctypes.CDLL(str(p))
                except OSError as e:
                    last_err = e

    # fallback: try system linker paths by name
    for name in filenames:
        try:
            return ctypes.CDLL(name)
        except OSError as e:
            last_err = e

    msg = "could not load base64 native library.\n"
    msg += f"searched filenames: {filenames}\n"
    msg += f"searched dirs: {dirs}\n"
    if last_err:
        msg += f"last error: {last_err}\n"
    raise OSError(msg)


# -----------------------------------------------------------------------------
# ABI Types
# -----------------------------------------------------------------------------
class VixStr(ctypes.Structure):
    _fields_ = [
        ("ptr", ctypes.c_char_p),
        ("len", ctypes.c_size_t),
    ]


class VixStatus(ctypes.Structure):
    _fields_ = [
        ("code", ctypes.c_int32),
        ("message", VixStr),
    ]


def _status_message(st: VixStatus) -> str:
    if not st.message.ptr or st.message.len == 0:
        return ""
    raw = ctypes.string_at(st.message.ptr, st.message.len)
    try:
        return raw.decode("utf-8", errors="replace")
    except Exception:
        return str(raw)


# -----------------------------------------------------------------------------
# Load + bind symbols
# -----------------------------------------------------------------------------
_LIB = _load_library()

# vix_str base64_ffi_version(vix_status*)
_LIB.base64_ffi_version.argtypes = [ctypes.POINTER(VixStatus)]
_LIB.base64_ffi_version.restype = VixStr

# int base64_encode(const uint8_t*, size_t, uint8_t*, size_t, size_t*, vix_status*)
_LIB.base64_encode.argtypes = [
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
    ctypes.POINTER(VixStatus),
]
_LIB.base64_encode.restype = ctypes.c_int

# int base64_decode(...)
_LIB.base64_decode.argtypes = [
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
    ctypes.POINTER(VixStatus),
]
_LIB.base64_decode.restype = ctypes.c_int


# -----------------------------------------------------------------------------
# Low-level wrappers used by api.py
# -----------------------------------------------------------------------------
def base64_ffi_version() -> str:
    st = VixStatus()
    s = _LIB.base64_ffi_version(ctypes.byref(st))
    if st.code != 0:
        raise RuntimeError(f"base64_ffi_version failed: code={st.code} msg={_status_message(st)}")
    if not s.ptr or s.len == 0:
        return ""
    return ctypes.string_at(s.ptr, s.len).decode("utf-8", errors="replace")


def _call_buf_op(fn, data: bytes) -> bytes:
    if data is None:
        data = b""

    # First attempt with a reasonable buffer, then grow if needed.
    out_cap = max(64, len(data) * 2 + 32)
    for _ in range(4):
        out = (ctypes.c_ubyte * out_cap)()
        out_len = ctypes.c_size_t(0)
        st = VixStatus()

        in_ptr = ctypes.c_void_p(0)
        if len(data) > 0:
            in_buf = ctypes.create_string_buffer(data, len(data))
            in_ptr = ctypes.cast(in_buf, ctypes.c_void_p)
        else:
            in_buf = None

        rc = fn(
            in_ptr,
            ctypes.c_size_t(len(data)),
            ctypes.cast(out, ctypes.c_void_p),
            ctypes.c_size_t(out_cap),
            ctypes.byref(out_len),
            ctypes.byref(st),
        )

        if rc == 0 and st.code == 0:
            return bytes(out[: out_len.value])

        # buffer too small pattern: out_len contains required size
        # We treat code==3 as VIX_STATUS_BUFFER_TOO_SMALL (per your spec),
        # but if code differs we still grow when out_len looks meaningful.
        if out_len.value > out_cap:
            out_cap = int(out_len.value)
            continue

        raise RuntimeError(f"native op failed: rc={rc} code={st.code} msg={_status_message(st)}")

    raise RuntimeError("native op failed: exceeded resize attempts")


def base64_encode(data: bytes) -> bytes:
    return _call_buf_op(_LIB.base64_encode, data)


def base64_decode(data: bytes) -> bytes:
    return _call_buf_op(_LIB.base64_decode, data)
