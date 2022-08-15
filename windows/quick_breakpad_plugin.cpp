#include "include/quick_breakpad/quick_breakpad_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <iostream>

#include "client/windows/handler/exception_handler.h"

namespace {

class QuickBreakpadPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  QuickBreakpadPlugin();

  virtual ~QuickBreakpadPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void QuickBreakpadPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "quick_breakpad",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<QuickBreakpadPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

static bool dumpCallback(
  const wchar_t* dump_path,
  const wchar_t* minidump_id,
  void *context,
  EXCEPTION_POINTERS* exinfo,
  MDRawAssertionInfo* assertion,
  bool succeeded
) {
  std::wcout << L"dump_path: " << dump_path << std::endl;
  std::wcout << L"minidump_id: " << minidump_id << std::endl;
  return succeeded;
}

inline
std::wstring StringToWString(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wide = new wchar_t[len + 1];
    memset(wide, '\0', sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, len);
    std::wstring w_str(wide);
    delete[] wide;
    return w_str;
}

QuickBreakpadPlugin::QuickBreakpadPlugin() {
}

QuickBreakpadPlugin::~QuickBreakpadPlugin() {}

void QuickBreakpadPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
   static google_breakpad::ExceptionHandler handler(L".", nullptr, dumpCallback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("regCrashHandler") == 0) {
      auto* path = std::get_if<std::string>(method_call.arguments());
      if (path != nullptr) {
          auto wPath =  StringToWString(*path);
          static google_breakpad::ExceptionHandler handler(wPath, nullptr, dumpCallback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);
      }
  }
  else {
      result->NotImplemented();
  }
}

}  // namespace

void QuickBreakpadPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  QuickBreakpadPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
