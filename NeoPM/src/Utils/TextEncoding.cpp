#include <NeoPM/Utils/TextEncoding.h>

#include <boost/nowide/convert.hpp>

namespace NeoPM::Utils {
    std::string WideToUtf8(const std::wstring& value) {
        try { return boost::nowide::narrow(value); }
        catch (...) { return {}; }
    }

    std::wstring Utf8ToWide(const std::string& value) {
        try { return boost::nowide::widen(value); }
        catch (...) { return {}; }
    }
}