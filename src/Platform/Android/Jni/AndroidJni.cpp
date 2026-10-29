#include <NeoWPF/Diagnostics/Logger.h>

#include <android/log.h>
#include <jni.h>

namespace {

void WriteLogcat(const char* message) noexcept
{
    __android_log_write(ANDROID_LOG_INFO, "NeoWPF", message);
}

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_asterin_neowpf_android_MainActivity_nativeStartup(JNIEnv* environment, jobject)
{
    NeoWPF::Diagnostics::SetOutputHandler(WriteLogcat);
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Info);
    NeoWPF::Diagnostics::LogInfo("Kotlin Activity connected to the NeoWPF JNI bridge");
    return environment->NewStringUTF("NeoWPF JNI bridge ready");
}