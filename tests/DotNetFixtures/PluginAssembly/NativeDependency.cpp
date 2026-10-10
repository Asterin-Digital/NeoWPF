#if defined(_WIN32)
#define NEO_WPF_EXPORT __declspec(dllexport)
#else
#define NEO_WPF_EXPORT __attribute__((visibility("default")))
#endif

extern "C" NEO_WPF_EXPORT int AddSeven(int value)
{
    return value + 7;
}