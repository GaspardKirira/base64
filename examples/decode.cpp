#include <base64/base64.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static void print_bytes(const std::vector<std::uint8_t> &v)
{
  std::cout << std::hex;
  for (std::size_t i = 0; i < v.size(); ++i)
  {
    std::cout << (i ? " " : "") << static_cast<unsigned>(v[i]);
  }
  std::cout << std::dec << "\n";
}

int main()
{
  const std::string s = "aGVsbG8="; // "hello"

  const auto bytes = base64::decode(s);

  std::cout << "base64: " << s << "\n";
  std::cout << "bytes: ";
  print_bytes(bytes);

  std::cout << "as text: ";
  for (auto b : bytes)
    std::cout << static_cast<char>(b);
  std::cout << "\n";

  return 0;
}
