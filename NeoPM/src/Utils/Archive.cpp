#include <NeoPM/Utils/Archive.h>

#include <zip.h>

#include <fstream>
#include <vector>

namespace NeoPM::Utils {
    bool Archive::ExtractZip(const std::filesystem::path& archivePath,
                             const std::filesystem::path& destinationPath) {
        std::error_code error;
        std::filesystem::create_directories(destinationPath, error);
        if (error) return false;

        int openError = 0;
        auto* archive = zip_open(archivePath.string().c_str(), ZIP_RDONLY, &openError);
        if (!archive) return false;

        const auto closeArchive = [&archive]() {
            zip_close(archive);
        };

        const auto entryCount = zip_get_num_entries(archive, 0);
        for (zip_uint64_t index = 0; index < entryCount; ++index) {
            zip_stat_t entry{};
            if (zip_stat_index(archive, index, 0, &entry) != 0 || !entry.name) {
                closeArchive();
                return false;
            }

            const std::filesystem::path relativePath(entry.name);
            if (relativePath.is_absolute() || relativePath.has_root_name()) {
                closeArchive();
                return false;
            }
            for (const auto& part : relativePath) {
                if (part == "..") {
                    closeArchive();
                    return false;
                }
            }

            const auto outputPath = destinationPath / relativePath;
            const bool isDirectory = relativePath.filename().empty() ||
                relativePath.generic_string().back() == '/';
            if (isDirectory) {
                std::filesystem::create_directories(outputPath, error);
                if (error) {
                    closeArchive();
                    return false;
                }
                continue;
            }

            std::filesystem::create_directories(outputPath.parent_path(), error);
            if (error) {
                closeArchive();
                return false;
            }

            auto* file = zip_fopen_index(archive, index, 0);
            if (!file) {
                closeArchive();
                return false;
            }

            std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
            if (!output) {
                zip_fclose(file);
                closeArchive();
                return false;
            }

            std::vector<char> buffer(64 * 1024);
            zip_int64_t bytesRead = 0;
            do {
                bytesRead = zip_fread(file, buffer.data(), buffer.size());
                if (bytesRead < 0) {
                    output.close();
                    zip_fclose(file);
                    closeArchive();
                    return false;
                }
                output.write(buffer.data(), bytesRead);
            } while (bytesRead > 0);

            zip_fclose(file);
            output.close();
            if (!output) {
                closeArchive();
                return false;
            }
        }

        closeArchive();
        return true;
    }
}