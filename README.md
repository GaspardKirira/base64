# base64

Minimal Base64 and Base64URL encoding/decoding utilities for modern C++.

`base64` provides strict and deterministic Base64 utilities, including
URL-safe encoding for JWT and web tokens.

Header-only. Zero dependencies.

## Download

https://vixcpp.com/registry/pkg/gaspardkirira/base64

## Why base64?

Base64 is used everywhere:

-   JWT tokens
-   API authentication
-   Binary-to-text encoding
-   Cryptographic signatures
-   HTTP payloads
-   Serialization layers

This library provides:

-   Standard Base64 encoding (with `=` padding)
-   Standard Base64 decoding
-   Base64URL encoding (no padding)
-   Base64URL decoding
-   Strict input validation

Minimal. Predictable. Secure.

## Installation

### Using Vix Registry

``` bash
vix add gaspardkirira/base64
vix deps
```

### Manual

``` bash
git clone https://github.com/GaspardKirira/base64.git
```

Add the `include/` directory to your project.

## Quick Examples

### Encode Standard Base64

``` cpp
#include <base64/base64.hpp>
#include <iostream>
#include <vector>

int main()
{
  std::vector<std::uint8_t> data = {'h','e','l','l','o'};

  std::string encoded = base64::encode(data);

  std::cout << encoded << "\n";
}
```

Output:
```
    aGVsbG8=
```
### Decode Standard Base64

``` cpp
#include <base64/base64.hpp>
#include <iostream>

int main()
{
  auto bytes = base64::decode("aGVsbG8=");

  for (auto b : bytes)
    std::cout << static_cast<char>(b);

  std::cout << "\n";
}
```

Output:
```
    hello
```
### Encode Base64URL (JWT-style)

``` cpp
#include <base64/base64.hpp>
#include <iostream>
#include <vector>

int main()
{
  std::vector<std::uint8_t> data = {'h','e','l','l','o'};

  std::string encoded = base64::encode_url(data);

  std::cout << encoded << "\n";
}
```

Output:
```
    aGVsbG8
```
No padding. URL-safe alphabet.

### Decode Base64URL

``` cpp
#include <base64/base64.hpp>
#include <iostream>

int main()
{
  auto bytes = base64::decode_url("aGVsbG8");

  for (auto b : bytes)
    std::cout << static_cast<char>(b);

  std::cout << "\n";
}
```

## API Overview

``` cpp
base64::encode(bytes);
base64::decode(string);

base64::encode_url(bytes);
base64::decode_url(string);
```

### encode()

-   Standard Base64
-   Includes `=` padding
-   Alphabet: `A-Z a-z 0-9 + /`

### decode()

Strict validation.

Throws `std::invalid_argument` on:

-   Invalid characters
-   Invalid padding
-   Invalid length

### encode_url()

-   URL-safe alphabet: `A-Z a-z 0-9 - _`
-   No padding
-   Suitable for JWT

### decode_url()

-   Accepts missing padding
-   Strict validation
-   Throws on invalid input

## Technical Details

-   Deterministic encoding
-   Strict padding validation
-   No global state
-   No dynamic allocation beyond output container
-   Fully header-only
-   Safe for crypto/token systems

## Tests

Run:

``` bash
vix build
vix tests
```

Tests verify:

-   Known vectors (`hello`)
-   Roundtrip integrity
-   URL-safe behavior
-   Invalid input detection
-   Padding correctness

## Design Philosophy

`base64` focuses on:

-   Minimal API
-   Explicit behavior
-   Strict validation
-   JWT compatibility
-   Clean integration with crypto stacks

Works perfectly alongside:

-   `secure_random`
-   `hex`
-   `endian`
-   `uuid`
-   `hashing`
-   `hmac`

## License

MIT License
Copyright (c) Gaspard Kirira

