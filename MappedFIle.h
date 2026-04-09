//
// Created by themi on 4/9/26.
//

#ifndef ITCH_PARSER_MAPPEDFILE_H
#define ITCH_PARSER_MAPPEDFILE_H
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include <stdexcept>
#include <string>

struct MappedFile {
    const uint8_t* data = nullptr;
    size_t         size = 0;
    int            fd   = -1;

    explicit MappedFile(const std::string& path) {
        fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) throw std::runtime_error("open() failed: " + path);

        struct stat st{};
        if (fstat(fd, &st) < 0) throw std::runtime_error("fstat() failed");
        size = static_cast<size_t>(st.st_size);

        data = static_cast<const uint8_t*>(
            mmap(nullptr, size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0)
        );
        if (data == MAP_FAILED) throw std::runtime_error("mmap() failed");

        // Tell the kernel: sequential access, prefetch aggressively
        madvise(const_cast<uint8_t*>(data), size, MADV_SEQUENTIAL);
    }

    ~MappedFile() {
        if (data && data != MAP_FAILED) munmap(const_cast<uint8_t*>(data), size);
        if (fd >= 0) close(fd);
    }

    // Non-copyable, moveable
    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;
};
#endif //ITCH_PARSER_MAPPEDFILE_H