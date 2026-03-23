// ==WindhawkMod==
// @id              taskbar-title-prefix-cleaner
// @name            Taskbar Title Prefix Cleaner
// @description     Keep only the final segment of taskbar titles, plus optional per-app regex transforms
// @version         1.0.0
// @author          0xjio
// @github          https://github.com/jihedkdiss
// @include         explorer.exe
// @compilerOptions -lole32 -loleaut32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Title Prefix Cleaner

Taskbar Title Prefix Cleaner trims noisy taskbar labels by keeping only the final
segment of a title. It is designed for users who want compact labels like
"z" instead of "x - y - z".

## Cleanup Modes

Choose how trimming is applied:

* **File Explorer only** (default): Keeps the classic Explorer behavior by
  removing the built-in File Explorer tail text.
* **Universal**: Keeps only the text after the rightmost separator for any
  window title.
* **Off**: Disables automatic cleanup.

## Universal Separator Support

Universal mode recognizes all of the following separators:

* ` - `
* ` — `
* `—`
* ` | `
* `|`
* ` • `
* `•`

Examples:

* "x - y - z" -> "z"
* "Dashboard | Browser" -> "Browser"
* "Playlist • Spotify" -> "Spotify"
* "Project — Editor" -> "Editor"

## Advanced Rule Engine

After automatic cleanup, you can run custom regex transformations.

Each rule contains:

* **Process identifier**: Process name (`notepad.exe`), full executable path,
  or app ID. Leave empty to match all windows.
* **Search pattern**: Regex to find in the current taskbar title.
* **Replace with**: Replacement text with capture groups support (`$1`, `$2`,
  etc.). Leave empty to remove matched text.

All matching rules are applied in order from top to bottom.

## Notes

* This changes taskbar display text only. The real window title is untouched.
* Universal cleanup runs before custom rules.
* Process matching is case-insensitive.
* Invalid regex patterns are skipped and logged.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- titleCleanupMode: fileExplorerOnly
  $name: Cleanup mode
  $options:
    - off: Off
    - fileExplorerOnly: File Explorer only
    - universal: Universal
  $description: >-
    Controls automatic cleanup behavior. "Universal" keeps only the part after
    the last supported separator (e.g., "x - y - z" becomes "z").
- titleRules:
  - - processIdentifier: ""
      $name: Process (name, path, or App ID)
      $description: >-
        Can be a process name (explorer.exe), full path
        (C:\Windows\explorer.exe), or App ID
        (Microsoft.WindowsCalculator_8wekyb3d8bbwe!App). Leave empty to match
        all processes.
    - search: ""
      $name: Search pattern (regex)
      $description: >-
        Regular expression pattern to search for in window titles. Example: " -
        Notepad$" to match " - Notepad" at the end of the title.
    - replace: ""
      $name: Replace with
      $description: >-
        Replacement text. Can use regex capture groups ($1, $2, etc.). Leave
        empty to remove the matched text.
  $name: Custom title rules
  $description: >-
    Define regex search/replace rules to apply after automatic cleanup.
    Multiple matching rules run in order.
*/
// ==/WindhawkModSettings==

#include <psapi.h>

#include <regex>
#include <string>
#include <vector>

#include <winrt/base.h>

enum class SuffixRemovalMode {
  Off,
  FileExplorerOnly,
  Universal,
};

struct SuffixRule {
  std::wstring processIdentifier;  // Stored in uppercase, empty = match all
  std::wregex search;
  std::wstring replace;
};

struct {
  SuffixRemovalMode suffixRemovalMode;
  std::vector<SuffixRule> suffixRules;
} g_settings;

HWND FindCurrentProcessTaskbarWnd() {
  HWND hTaskbarWnd = nullptr;

  EnumWindows(
    [](HWND hWnd, LPARAM lParam) WINAPI -> BOOL {
      DWORD dwProcessId;
      WCHAR className[32];
      if (GetWindowThreadProcessId(hWnd, &dwProcessId) &&
        dwProcessId == GetCurrentProcessId() &&
        GetClassName(hWnd, className, ARRAYSIZE(className)) &&
        _wcsicmp(className, L"Shell_TrayWnd") == 0) {
        *reinterpret_cast<HWND*>(lParam) = hWnd;
        return FALSE;
      }
      return TRUE;
    },
    reinterpret_cast<LPARAM>(&hTaskbarWnd));

  return hTaskbarWnd;
}

HWND GetTaskBandWnd() {
  HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
  if (hTaskbarWnd) {
    return (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
  }

  return nullptr;
}

// https://gist.github.com/m417z/451dfc2dad88d7ba88ed1814779a26b4
std::wstring GetWindowAppId(HWND hWnd) {
  // {c8900b66-a973-584b-8cae-355b7f55341b}
  constexpr winrt::guid CLSID_StartMenuCacheAndAppResolver{
    0x660b90c8,
    0x73a9,
    0x4b58,
    {0x8c, 0xae, 0x35, 0x5b, 0x7f, 0x55, 0x34, 0x1b}};

  // {de25675a-72de-44b4-9373-05170450c140}
  constexpr winrt::guid IID_IAppResolver_8{
    0xde25675a,
    0x72de,
    0x44b4,
    {0x93, 0x73, 0x05, 0x17, 0x04, 0x50, 0xc1, 0x40}};

  struct IAppResolver_8 : public IUnknown {
     public:
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcut() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcutObject() = 0;
    virtual HRESULT STDMETHODCALLTYPE
    GetAppIDForWindow(HWND hWnd,
              WCHAR** pszAppId,
              void* pUnknown1,
              void* pUnknown2,
              void* pUnknown3) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    GetAppIDForProcess(DWORD dwProcessId,
               WCHAR** pszAppId,
               void* pUnknown1,
               void* pUnknown2,
               void* pUnknown3) = 0;
  };

  HRESULT hr;
  std::wstring result;

  winrt::com_ptr<IAppResolver_8> appResolver;
  hr = CoCreateInstance(CLSID_StartMenuCacheAndAppResolver, nullptr,
              CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
              IID_IAppResolver_8, appResolver.put_void());
  if (SUCCEEDED(hr)) {
    WCHAR* pszAppId;
    hr = appResolver->GetAppIDForWindow(hWnd, &pszAppId, nullptr, nullptr,
                      nullptr);
    if (SUCCEEDED(hr)) {
      result = pszAppId;
      CoTaskMemFree(pszAppId);
    }
  }

  return result;
}

std::vector<const SuffixRule*> GetRulesForWindow(HWND hWnd) {
  std::vector<const SuffixRule*> matchedRules;

  if (g_settings.suffixRules.empty()) {
    return matchedRules;
  }

  // Get process path and convert to uppercase
  WCHAR resolvedWindowProcessPath[MAX_PATH];
  WCHAR resolvedWindowProcessPathUpper[MAX_PATH];
  DWORD resolvedWindowProcessPathLen = 0;

  DWORD dwProcessId = 0;
  if (GetWindowThreadProcessId(hWnd, &dwProcessId)) {
    HANDLE hProcess =
      OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (hProcess) {
      DWORD dwSize = ARRAYSIZE(resolvedWindowProcessPath);
      if (QueryFullProcessImageName(hProcess, 0,
                      resolvedWindowProcessPath, &dwSize)) {
        resolvedWindowProcessPathLen = dwSize;
      }
      CloseHandle(hProcess);
    }
  }

  if (resolvedWindowProcessPathLen > 0) {
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE,
            resolvedWindowProcessPath,
            resolvedWindowProcessPathLen + 1,
            resolvedWindowProcessPathUpper,
            resolvedWindowProcessPathLen + 1, nullptr, nullptr, 0);
  } else {
    resolvedWindowProcessPathUpper[0] = L'\0';
  }

  // Extract process name from path
  PCWSTR programFileNameUpper =
    wcsrchr(resolvedWindowProcessPathUpper, L'\\');
  if (programFileNameUpper) {
    programFileNameUpper++;
  }

  // Get App ID once (expensive operation)
  std::wstring appId;
  bool appIdFetched = false;

  // Check each rule and collect all matches
  for (const auto& rule : g_settings.suffixRules) {
    bool matches = false;

    // Empty process identifier matches all processes
    if (rule.processIdentifier.empty()) {
      matches = true;
    }
    // Check full path match
    else if (wcscmp(resolvedWindowProcessPathUpper,
            rule.processIdentifier.c_str()) == 0) {
      matches = true;
    }
    // Check process name match
    else if (programFileNameUpper && *programFileNameUpper &&
         wcscmp(programFileNameUpper, rule.processIdentifier.c_str()) ==
           0) {
      matches = true;
    }
    // Check App ID match
    else {
      if (!appIdFetched) {
        appId = GetWindowAppId(hWnd);
        if (!appId.empty()) {
          LCMapStringEx(
            LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, appId.data(),
            static_cast<int>(appId.length()), appId.data(),
            static_cast<int>(appId.length()), nullptr, nullptr, 0);
        }
        appIdFetched = true;
      }
      if (!appId.empty() &&
        wcscmp(appId.c_str(), rule.processIdentifier.c_str()) == 0) {
        matches = true;
      }
    }

    if (matches) {
      matchedRules.push_back(&rule);
    }
  }

  return matchedRules;
}

using FindResourceExW_t = decltype(&FindResourceExW);
FindResourceExW_t FindResourceExW_Original;
HRSRC WINAPI FindResourceExW_Hook(HMODULE hModule,
                  LPCWSTR lpType,
                  LPCWSTR lpName,
                  WORD wLanguage) {
  if (g_settings.suffixRemovalMode == SuffixRemovalMode::FileExplorerOnly &&
    hModule && lpType == RT_STRING && lpName == MAKEINTRESOURCE(2195) &&
    hModule == GetModuleHandle(L"explorerframe.dll")) {
    Wh_Log(L">");
    SetLastError(ERROR_RESOURCE_NAME_NOT_FOUND);
    return nullptr;
  }

  return FindResourceExW_Original(hModule, lpType, lpName, wLanguage);
}

using InternalGetWindowText_t = int(WINAPI*)(HWND hWnd,
                       LPWSTR pString,
                       int cchMaxCount);
InternalGetWindowText_t InternalGetWindowText_Original;
int WINAPI InternalGetWindowText_Hook(HWND hWnd,
                    LPWSTR pString,
                    int cchMaxCount) {
  int result = InternalGetWindowText_Original(hWnd, pString, cchMaxCount);
  if (result == 0 || !pString || cchMaxCount == 0) {
    return result;
  }

  if (g_settings.suffixRemovalMode != SuffixRemovalMode::Universal &&
    g_settings.suffixRules.empty()) {
    return result;
  }

  void* retAddress = __builtin_return_address(0);

  HMODULE taskbarModule = GetModuleHandle(L"taskbar.dll");
  if (!taskbarModule) {
    return result;
  }

  HMODULE module;
  if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (PCWSTR)retAddress, &module) ||
    module != taskbarModule) {
    return result;
  }

  Wh_Log(L"Original text: %s", pString);

  std::wstring text = pString;
  bool modified = false;

  // Apply universal mode: keep only text after the last separator
  if (g_settings.suffixRemovalMode == SuffixRemovalMode::Universal &&
    result > 0 && pString) {
    // Find the rightmost separator and keep everything after it.
    // This makes "x - y - z" become "z".
    size_t lastSepPos = std::wstring::npos;
    size_t lastSepLen = 0;

    size_t hyphenPos = text.rfind(L" - ");
    size_t emDashSpacePos = text.rfind(L" — ");
    size_t emDashPos = text.rfind(L"—");
    size_t pipeSpacePos = text.rfind(L" | ");
    size_t pipePos = text.rfind(L"|");
    size_t bulletSpacePos = text.rfind(L" • ");
    size_t bulletPos = text.rfind(L"•");

    auto updateLastSep = [&](size_t pos, size_t len) {
      if (pos != std::wstring::npos &&
        (lastSepPos == std::wstring::npos || pos > lastSepPos ||
         (pos == lastSepPos && len > lastSepLen))) {
        lastSepPos = pos;
        lastSepLen = len;
      }
    };

    updateLastSep(hyphenPos, 3);
    updateLastSep(emDashSpacePos, 3);
    updateLastSep(emDashPos, 1);
    updateLastSep(pipeSpacePos, 3);
    updateLastSep(pipePos, 1);
    updateLastSep(bulletSpacePos, 3);
    updateLastSep(bulletPos, 1);

    if (lastSepPos != std::wstring::npos) {
      size_t valueStart = lastSepPos + lastSepLen;
      while (valueStart < text.length() && text[valueStart] == L' ') {
        valueStart++;
      }

      text = text.substr(valueStart);
      modified = true;
      Wh_Log(L"Universal mode: removed prefix before last separator");
    }
  }

  // Get all matching rules for this window's process
  std::vector<const SuffixRule*> rules = GetRulesForWindow(hWnd);

  if (!rules.empty() && result > 0 && pString) {
    // Apply all matching rules in order
    for (const auto* rule : rules) {
      try {
        std::wstring newText =
          std::regex_replace(text, rule->search, rule->replace);
        if (newText != text) {
          text = newText;
          modified = true;
        }
      } catch (const std::regex_error& ex) {
        Wh_Log(L"Regex replace error %08X: %S",
             static_cast<DWORD>(ex.code()), ex.what());
      }
    }
  }

  // Update the window text if it changed
  if (modified) {
    if (text.length() < static_cast<size_t>(cchMaxCount)) {
      wcscpy_s(pString, cchMaxCount, text.c_str());
      result = static_cast<int>(text.length());
      Wh_Log(L"Modified text: %s", pString);
    } else {
      Wh_Log(L"Result too long (%zu chars), keeping original",
           text.length());
    }
  }

  return result;
}

void ApplySettings() {
  HWND hTaskBandWnd = GetTaskBandWnd();
  if (!hTaskBandWnd) {
    return;
  }

  static const UINT WM_SHELLHOOK = RegisterWindowMessage(L"SHELLHOOK");

  EnumWindows(
    [](HWND hWnd, LPARAM lParam) WINAPI -> BOOL {
      if (IsWindowVisible(hWnd)) {
        PostMessage(reinterpret_cast<HWND>(lParam), WM_SHELLHOOK,
              HSHELL_REDRAW, reinterpret_cast<LPARAM>(hWnd));
      }
      return TRUE;
    },
    reinterpret_cast<LPARAM>(hTaskBandWnd));
}

void LoadSettings() {
  Wh_Log(L"LoadSettings");

  // Load cleanup mode
  PCWSTR mode = Wh_GetStringSetting(L"titleCleanupMode");
  g_settings.suffixRemovalMode = SuffixRemovalMode::FileExplorerOnly;
  if (wcscmp(mode, L"off") == 0) {
    g_settings.suffixRemovalMode = SuffixRemovalMode::Off;
  } else if (wcscmp(mode, L"universal") == 0) {
    g_settings.suffixRemovalMode = SuffixRemovalMode::Universal;
  }
  Wh_FreeStringSetting(mode);

  // Load custom title rules
  g_settings.suffixRules.clear();

  for (int i = 0;; i++) {
    PCWSTR processId =
      Wh_GetStringSetting(L"titleRules[%d].processIdentifier", i);
    PCWSTR search = Wh_GetStringSetting(L"titleRules[%d].search", i);
    PCWSTR replace = Wh_GetStringSetting(L"titleRules[%d].replace", i);

    bool hasRule = *search;

    if (!hasRule) {
      Wh_FreeStringSetting(processId);
      Wh_FreeStringSetting(search);
      Wh_FreeStringSetting(replace);
      break;
    }

    try {
      SuffixRule rule;

      // Convert processIdentifier to uppercase (empty = match all)
      if (*processId) {
        rule.processIdentifier = processId;
        LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE,
                &rule.processIdentifier[0],
                static_cast<int>(rule.processIdentifier.length()),
                &rule.processIdentifier[0],
                static_cast<int>(rule.processIdentifier.length()),
                nullptr, nullptr, 0);
      }

      rule.search = std::wregex(search);
      rule.replace = replace;

      Wh_Log(L"Loaded rule for '%s': '%s' -> '%s'",
           rule.processIdentifier.empty()
             ? L"<all processes>"
             : rule.processIdentifier.c_str(),
           search, replace);

      g_settings.suffixRules.push_back(std::move(rule));
    } catch (const std::regex_error& ex) {
      Wh_Log(L"Invalid regex pattern '%s': %S (code %08X)", search,
           ex.what(), static_cast<DWORD>(ex.code()));
    }

    Wh_FreeStringSetting(processId);
    Wh_FreeStringSetting(search);
    Wh_FreeStringSetting(replace);
  }
}

BOOL Wh_ModInit() {
  Wh_Log(L">");

  LoadSettings();

  HMODULE kernelBaseModule = GetModuleHandle(L"kernelbase.dll");
  HMODULE kernel32Module = GetModuleHandle(L"kernel32.dll");

  auto setKernelFunctionHook = [kernelBaseModule, kernel32Module](
                   PCSTR targetName, void* hookFunction,
                   void** originalFunction) {
    void* targetFunction =
      (void*)GetProcAddress(kernelBaseModule, targetName);
    if (!targetFunction) {
      targetFunction = (void*)GetProcAddress(kernel32Module, targetName);
      if (!targetFunction) {
        return FALSE;
      }
    }

    return Wh_SetFunctionHook(targetFunction, hookFunction,
                  originalFunction);
  };

  setKernelFunctionHook("FindResourceExW", (void*)FindResourceExW_Hook,
              (void**)&FindResourceExW_Original);

  HMODULE user32Module = GetModuleHandle(L"user32.dll");
  if (user32Module) {
    void* pInternalGetWindowText =
      (void*)GetProcAddress(user32Module, "InternalGetWindowText");
    if (pInternalGetWindowText) {
      Wh_SetFunctionHook(pInternalGetWindowText,
                 (void*)InternalGetWindowText_Hook,
                 (void**)&InternalGetWindowText_Original);
    }
  }

  return TRUE;
}

void Wh_ModAfterInit() {
  Wh_Log(L">");

  ApplySettings();
}

void Wh_ModUninit() {
  Wh_Log(L">");

  ApplySettings();
}

void Wh_ModSettingsChanged() {
  Wh_Log(L">");

  LoadSettings();

  ApplySettings();
}