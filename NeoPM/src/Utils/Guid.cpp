#include <NeoPM/Utils/Guid.h>

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

namespace NeoPM::Utils {
    std::string GenerateGuid() {
        std::array<unsigned char, 16> bytes{};
        std::random_device device;
        for (auto& byte : bytes) {
            byte = static_cast<unsigned char>(device());
        }
        bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0f) | 0x40);
        bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3f) | 0x80);

        std::ostringstream output;
        output << std::hex << std::setfill('0') << std::uppercase;
        for (std::size_t index = 0; index < bytes.size(); ++index) {
            if (index == 4 || index == 6 || index == 8 || index == 10) output << '-';
            output << std::setw(2) << static_cast<unsigned int>(bytes[index]);
        }
        return output.str();
    }
}