/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <boost/interprocess/sync/named_mutex.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace NeoPM::Utils {
    class NamedMutex {
    public:
        explicit NamedMutex(std::string name);
        ~NamedMutex();
        NamedMutex(const NamedMutex&) = delete;
        NamedMutex& operator=(const NamedMutex&) = delete;
        bool Acquire();
        bool Acquired() const noexcept;
        static bool IsLocked(const std::string& name);

    private:
        std::string m_name;
        std::unique_ptr<boost::interprocess::named_mutex> m_mutex;
        bool m_acquired = false;
    };
}