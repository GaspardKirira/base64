#include <base64/base64.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main()
{
  // This demonstrates a JWT-style base64url segment (no padding).
  const std::string json = R"({"alg":"HS256","typ":"JWT"})";

  std::vector<std::uint8_t> bytes(json.begin(), json.end());

  const std::string seg = base64::encode_url(bytes);

  std::cout << "json   : " << json << "\n";
  std::cout << "segment: " << seg << "\n";

  // Decode it back
  const auto decoded = base64::decode_url(seg);
  const std::string back(decoded.begin(), decoded.end());

  std::cout << "decoded: " << back << "\n";
  return 0;
}
