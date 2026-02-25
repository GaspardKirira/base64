from . import _native


def ffi_version() -> str:
    return _native.base64_ffi_version()


def encode(data: bytes) -> bytes:
    """
    Returns base64 bytes (ASCII), not null-terminated.
    """
    return _native.base64_encode(data)


def decode(data: bytes) -> bytes:
    """
    Returns decoded raw bytes.
    """
    return _native.base64_decode(data)
