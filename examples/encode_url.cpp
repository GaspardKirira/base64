#include <base64/base64.hpp>

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
  const std::vector<std::uint8_t> data = {'h', 'e', 'l', 'l', 'o'};

  const std::string enc = base64::encode_url(data);

  std::cout << "input     : hello\n";
  std::cout << "base64url : " << enc << "\n";
  return 0;
}
