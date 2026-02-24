#include <base64/base64.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static void expect_true(bool v, const char *msg)
{
  if (!v)
  {
    std::cerr << "FAIL: " << msg << "\n";
    std::exit(1);
  }
}

int main()
{
  // "hello" -> aGVsbG8=
  {
    const std::vector<std::uint8_t> data = {'h', 'e', 'l', 'l', 'o'};
    const std::string enc = base64::encode(data);
    expect_true(enc == "aGVsbG8=", "encode hello");

    const auto dec = base64::decode(enc);
    expect_true(dec == data, "decode hello");
  }

  // base64url: "hello" -> aGVsbG8 (no padding)
  {
    const std::vector<std::uint8_t> data = {'h', 'e', 'l', 'l', 'o'};
    const std::string enc = base64::encode_url(data);
    expect_true(enc == "aGVsbG8", "encode_url hello");

    const auto dec = base64::decode_url(enc);
    expect_true(dec == data, "decode_url hello");
  }

  // Roundtrip random-like bytes
  {
    const std::vector<std::uint8_t> data = {0x00, 0xFF, 0x10, 0x20, 0x33, 0x7F};
    const auto enc = base64::encode(data);
    const auto dec = base64::decode(enc);
    expect_true(dec == data, "roundtrip standard");

    const auto encu = base64::encode_url(data);
    const auto decu = base64::decode_url(encu);
    expect_true(decu == data, "roundtrip url");
  }

  // Invalid length for base64url (mod 4 == 1)
  {
    bool threw = false;
    try
    {
      (void)base64::decode_url("a");
    }
    catch (...)
    {
      threw = true;
    }
    expect_true(threw, "invalid base64url length throws");
  }

  // Invalid char
  {
    bool threw = false;
    try
    {
      (void)base64::decode("!!!!");
    }
    catch (...)
    {
      threw = true;
    }
    expect_true(threw, "invalid char throws");
  }

  std::cout << "ok\n";
  return 0;
}
