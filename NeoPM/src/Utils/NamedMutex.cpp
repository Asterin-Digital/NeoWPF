#include <NeoPM/Utils/NamedMutex.h>
#include <NeoPM/Utils/Process.h>

#include <boost/interprocess/exceptions.hpp>

#include <memory>
#include <utility>

namespace NeoPM::Utils {
    NamedMutex::NamedMutex(std::string name) {
        m_name = std::move(name);
    }

    NamedMutex::~NamedMutex() {
        if (m_acquired) {
            m_mutex.reset();
            boost::interprocess::named_mutex::remove(m_name.c_str());
        }
    }

    bool NamedMutex::Acquire() {
        try {
            try {
                m_mutex = std::make_unique<boost::interprocess::named_mutex>(
                    boost::interprocess::open_or_create, m_name.c_str());
            }
            catch (const boost::interprocess::interprocess_exception&) {
                return false;
            }

            m_acquired = m_mutex->try_lock();
            if (!m_acquired) {
                m_mutex.reset();

                const auto daemonIsRunning =
#if defined(_WIN32)
                    Utils::Process::IsAnotherProcessRunning("NeoPM.exe");
#else
                    Utils::Process::IsAnotherProcessRunning("NeoPM");
#endif
                if (m_name == "NeoWPF.NeoPM.Daemon" && !daemonIsRunning) {
                    boost::interprocess::named_mutex::remove(m_name.c_str());
                    m_mutex = std::make_unique<boost::interprocess::named_mutex>(
                        boost::interprocess::open_or_create, m_name.c_str());
                    m_acquired = m_mutex->try_lock();
                }

                if (!m_acquired) {
                    m_mutex.reset();
                }
            }
            return m_acquired;
        }
        catch (const boost::interprocess::interprocess_exception&) {
            return false;
        }
    }

    bool NamedMutex::Acquired() const noexcept { return m_acquired; }

    bool NamedMutex::IsLocked(const std::string& name) {
        try {
            boost::interprocess::named_mutex mutex(boost::interprocess::open_only, name.c_str());
            if (!mutex.try_lock()) {
                return true;
            }
            mutex.unlock();
            return false;
        }
        catch (const boost::interprocess::interprocess_exception&) {
            return false;
        }
    }
}