/**
 * @file plugin_info_test.cpp
 * @brief Tests dynamic loading of generated plugin metadata.
 */

#include <eversion/runtime.h>

#include <iostream>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

#if defined(_WIN32)
using LibraryHandle = HMODULE;

LibraryHandle OpenLibrary(const char* path) { return LoadLibraryA(path); }

void CloseLibrary(LibraryHandle handle) {
  if (handle != nullptr) {
    FreeLibrary(handle);
  }
}

void* LoadSymbol(LibraryHandle handle, const char* name) {
  return reinterpret_cast<void*>(GetProcAddress(handle, name));
}
#else
using LibraryHandle = void*;

LibraryHandle OpenLibrary(const char* path) { return dlopen(path, RTLD_NOW); }

void CloseLibrary(LibraryHandle handle) {
  if (handle != nullptr) {
    dlclose(handle);
  }
}

void* LoadSymbol(LibraryHandle handle, const char* name) {
  return dlsym(handle, name);
}
#endif

bool Expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
  }
  return condition;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: eversion_plugin_info_test <plugin-path>\n";
    return 1;
  }

  LibraryHandle library = OpenLibrary(argv[1]);
  if (library == nullptr) {
    std::cerr << "failed to load plugin: " << argv[1] << '\n';
    return 1;
  }

  void* symbol = LoadSymbol(library, eversion::kPluginInfoSymbol.data());
  if (symbol == nullptr) {
    std::cerr << "failed to find symbol: " << eversion::kPluginInfoSymbol
              << '\n';
    CloseLibrary(library);
    return 1;
  }

  const auto plugin_info =
      reinterpret_cast<eversion::ComponentInfoFunction>(symbol);
  const eversion::ComponentInfo* info = plugin_info();

  bool ok = true;
  ok &= Expect(eversion::IsValidComponentInfo(info),
               "plugin component info validates");

  if (ok) {
    ok &= Expect(std::string_view{info->id} == "test.plugin",
                 "plugin id matches");
    ok &= Expect(std::string_view{info->name} == "EVersion Test Plugin",
                 "plugin name matches");
    ok &= Expect(eversion::ToVersion(info->version) ==
                     eversion::Version{9, 8, 7, "beta.2", "other.build"},
                 "plugin version follows version precedence");
    ok &= Expect(eversion::ToVersion(info->version).ToString() ==
                     "9.8.7-beta.2+build.42",
                 "plugin version string is complete");
  }

  CloseLibrary(library);
  return ok ? 0 : 1;
}
