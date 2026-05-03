#pragma once

// Cross-platform shim for the Linux build.  The original project is Windows/MSVC
// first; keep Windows behaviour untouched and provide small substitutes only on
// non-Windows platforms.
#ifndef _WIN32
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <ctime>
#include <cwctype>
#include <chrono>
#include <string>
#include <filesystem>
#include <cstdio>
#include <locale>
#include <codecvt>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cerrno>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cfloat>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <sys/wait.h>
#include <wx/defs.h>
#include <wx/window.h>
#include <wx/panel.h>

#ifndef __linux__
#define __linux__ 1
#endif

#ifndef __WXGTK__
#define __WXGTK__ 1
#endif

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif

#ifndef __stdcall
#define __stdcall
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

using BYTE = unsigned char;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using UINT = unsigned int;
using ULONG = unsigned long;
using LONG = long;
using BOOL = int;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
using BOOLEAN = int;
using VOID = void;
using HRESULT = long;
using WCHAR = wchar_t;
using LPCWSTR = const wchar_t*;
using LPWSTR = wchar_t*;
using LPWORD = WORD*;
using LPCSTR = const char*;
using HWND = void*;
using HDC = void*;
using HMODULE = void*;
using FARPROC = void*;
using HGLOBAL = void*;
using HRSRC = void*;
struct ITEMIDLIST {};
using HGDIOBJ = void*;
using PVOID = void*;
using ULONGLONG = unsigned long long;
using HICON = void*;
using HANDLE = void*;
using HMONITOR = void*;
using HKEY = void*;
using LPBYTE = BYTE*;
struct LOGFONTW { wchar_t lfFaceName[128]{}; long lfHeight{}; long lfWeight{}; unsigned char lfItalic{}; unsigned char lfUnderline{}; unsigned char lfStrikeOut{}; unsigned char lfCharSet{}; unsigned char lfPitchAndFamily{}; unsigned char lfOutPrecision{}; unsigned char lfClipPrecision{}; unsigned char lfQuality{}; };
using LOGFONT = LOGFONTW;
using LPLOGFONT = LOGFONTW*;
using HFONT = HGDIOBJ;
struct TEXTMETRIC { int tmDescent{}; int tmExternalLeading{}; };
struct SIZE { long cx{}; long cy{}; };
using FONTENUMPROC = int (__stdcall *)(const LOGFONT*, const TEXTMETRIC*, DWORD, std::intptr_t);
using FONTENUMPROCW = FONTENUMPROC;
constexpr int LF_FACESIZE = 32;
struct SYSTEMTIME { WORD wYear{}, wMonth{}, wDayOfWeek{}, wDay{}, wHour{}, wMinute{}, wSecond{}, wMilliseconds{}; };
struct FILETIME { DWORD dwLowDateTime{}, dwHighDateTime{}; };
struct WIN32_FIND_DATAW { DWORD nFileSizeLow{}; FILETIME ftLastWriteTime{}; wchar_t cFileName[MAX_PATH]{}; };
using WIN32_FIND_DATA = WIN32_FIND_DATAW;
enum FINDEX_INFO_LEVELS { FindExInfoStandard = 0, FindExInfoBasic = 1 };
constexpr int FindExSearchNameMatch = 0;
struct OSVERSIONINFO { DWORD dwOSVersionInfoSize{}, dwMajorVersion{10}, dwMinorVersion{}; };
inline BOOL GetVersionExW(OSVERSIONINFO*) { return TRUE; }
using byte = unsigned char;
using WPARAM = std::uintptr_t;
using LPARAM = std::intptr_t;
using LRESULT = std::intptr_t;
using HHOOK = void*;
#ifndef WXLRESULT
using WXLRESULT = long;
#endif
#ifndef WXUINT
using WXUINT = unsigned int;
#endif
#ifndef WXWPARAM
using WXWPARAM = std::uintptr_t;
#endif
#ifndef WXLPARAM
using WXLPARAM = std::intptr_t;
#endif
using WXHDC = void*;
struct POINT { long x{}; long y{}; POINT(long _x = 0, long _y = 0) : x(_x), y(_y) {} };
struct MSG { HWND hwnd{}; UINT message{}; WPARAM wParam{}; LPARAM lParam{}; DWORD time{}; POINT pt{}; };
using LPMSG = MSG*;
constexpr WPARAM VK_MENU = 0x12;
constexpr WPARAM VK_LMENU = 0xA4;
constexpr WPARAM VK_LSHIFT = 0xA0;
constexpr WPARAM VK_RSHIFT = 0xA1;
constexpr WPARAM VK_LCONTROL = 0xA2;
constexpr WPARAM VK_RCONTROL = 0xA3;
constexpr WPARAM VK_DOWN = 0x28;
constexpr WPARAM VK_UP = 0x26;
constexpr WPARAM VK_LEFT = 0x25;
constexpr WPARAM VK_RIGHT = 0x27;
constexpr WPARAM VK_ESCAPE = 0x1B;
constexpr WPARAM VK_RETURN = 0x0D;
constexpr UINT WM_MOUSEWHEEL = 0x020A;
constexpr UINT WM_LBUTTONDOWN = 0x0201;
constexpr UINT WM_NCLBUTTONDOWN = 0x00A1;
constexpr UINT WM_MBUTTONUP = 0x0208;
constexpr UINT WM_NCMBUTTONUP = 0x00A8;
inline BOOL GetKeyboardState(BYTE state[256]) { if (state) std::memset(state, 0, 256); return TRUE; }
inline BOOL GetCursorPos(POINT* p) {
    if (!p) return FALSE;
    wxPoint pos = wxGetMousePosition();
    p->x = pos.x;
    p->y = pos.y;
    return TRUE;
}
inline HWND WindowFromPoint(POINT pt) {
    wxWindow* win = wxFindWindowAtPoint(wxPoint(pt.x, pt.y));
    return win ? reinterpret_cast<HWND>(win) : nullptr;
}
inline DWORD GetCurrentThreadId() {
    static std::atomic<DWORD> next{1};
    thread_local DWORD id = next.fetch_add(1);
    return id;
}
inline DWORD GetCurrentProcessId() { return static_cast<DWORD>(::getpid()); }
inline DWORD GetWindowThreadProcessId(HWND, DWORD* pid) { if (pid) *pid = GetCurrentProcessId(); return GetCurrentThreadId(); }
inline wxWindow* wxGetWindowFromHWND(HWND hwnd) { return reinterpret_cast<wxWindow*>(hwnd); }

inline DWORD timeGetTime() {
    using namespace std::chrono;
    return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

#ifndef _SPACE
#define _SPACE 0x0008
#endif
#ifndef _PUNCT
#define _PUNCT 0x0010
#endif
inline int kainote_iswctype(wint_t ch, int mask) {
    return ((mask & _SPACE) && std::iswspace(ch)) || ((mask & _PUNCT) && std::iswpunct(ch));
}
#define iswctype(ch, mask) kainote_iswctype((ch), (mask))

constexpr DWORD GENERIC_READ = 0x80000000u;
constexpr DWORD GENERIC_WRITE = 0x40000000u;
constexpr DWORD FILE_SHARE_READ = 0x00000001u;
constexpr DWORD FILE_SHARE_WRITE = 0x00000002u;
constexpr DWORD FILE_SHARE_DELETE = 0x00000004u;
constexpr DWORD OPEN_EXISTING = 3;
constexpr DWORD OPEN_ALWAYS = 4;
constexpr DWORD CREATE_ALWAYS = 2;
constexpr DWORD FILE_ATTRIBUTE_NORMAL = 0x80;
constexpr DWORD FILE_ATTRIBUTE_READONLY = 0x1;
constexpr DWORD INVALID_FILE_ATTRIBUTES = 0xffffffffu;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

inline void kainote_time_t_to_system(std::time_t t, SYSTEMTIME* st) {
    std::tm tm{}; gmtime_r(&t, &tm);
    st->wYear = tm.tm_year + 1900; st->wMonth = tm.tm_mon + 1; st->wDay = tm.tm_mday;
    st->wDayOfWeek = tm.tm_wday; st->wHour = tm.tm_hour; st->wMinute = tm.tm_min; st->wSecond = tm.tm_sec; st->wMilliseconds = 0;
}
inline void GetSystemTime(SYSTEMTIME* st) { kainote_time_t_to_system(std::time(nullptr), st); }
inline void GetLocalTime(SYSTEMTIME* st) {
    std::time_t t=std::time(nullptr); std::tm tm{}; localtime_r(&t,&tm);
    st->wYear=tm.tm_year+1900; st->wMonth=tm.tm_mon+1; st->wDay=tm.tm_mday; st->wDayOfWeek=tm.tm_wday; st->wHour=tm.tm_hour; st->wMinute=tm.tm_min; st->wSecond=tm.tm_sec; st->wMilliseconds=0;
}
inline bool SystemTimeToFileTime(const SYSTEMTIME* st, FILETIME* ft) {
    std::tm tm{}; tm.tm_year=st->wYear-1900; tm.tm_mon=st->wMonth-1; tm.tm_mday=st->wDay; tm.tm_hour=st->wHour; tm.tm_min=st->wMinute; tm.tm_sec=st->wSecond;
    auto t = timegm(&tm); auto v=static_cast<std::uint64_t>(t);
    ft->dwLowDateTime=static_cast<DWORD>(v); ft->dwHighDateTime=static_cast<DWORD>(v>>32); return true;
}
inline bool FileTimeToSystemTime(const FILETIME* ft, SYSTEMTIME* st) {
    std::uint64_t v=((std::uint64_t)ft->dwHighDateTime<<32)|ft->dwLowDateTime; kainote_time_t_to_system((std::time_t)v, st); return true;
}
struct KainoteHandleBase {
    enum class Kind { File, Event, Thread, Timer } kind;
    explicit KainoteHandleBase(Kind k) : kind(k) {}
    virtual ~KainoteHandleBase() = default;
};
struct KainoteFileHandle : KainoteHandleBase {
    FILE* file{};
    explicit KainoteFileHandle(FILE* f) : KainoteHandleBase(Kind::File), file(f) {}
    ~KainoteFileHandle() override { if (file) std::fclose(file); }
};
struct KainoteEventHandle : KainoteHandleBase {
    std::mutex mutex;
    std::condition_variable cv;
    bool manualReset{};
    bool signaled{};
    KainoteEventHandle(bool manual, bool initial) : KainoteHandleBase(Kind::Event), manualReset(manual), signaled(initial) {}
};
struct KainoteThreadHandle : KainoteHandleBase {
    std::thread thread;
    std::atomic<bool> finished{false};
    KainoteThreadHandle() : KainoteHandleBase(Kind::Thread) {}
    explicit KainoteThreadHandle(std::thread&& t) : KainoteHandleBase(Kind::Thread), thread(std::move(t)) {}
    ~KainoteThreadHandle() override { if (thread.joinable()) thread.detach(); }
};
struct KainoteTimerHandle : KainoteHandleBase {
    std::thread thread;
    std::atomic<bool> cancelled{false};
    explicit KainoteTimerHandle(std::thread&& t) : KainoteHandleBase(Kind::Timer), thread(std::move(t)) {}
    ~KainoteTimerHandle() override { cancelled = true; if (thread.joinable()) thread.detach(); }
};

inline HANDLE CreateFileW(const wchar_t* path, DWORD access, DWORD, void*, DWORD creation, DWORD, HANDLE) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; std::string p=conv.to_bytes(path ? std::wstring(path) : std::wstring()); std::replace(p.begin(), p.end(), '\\', '/');
    const char* mode = (access & GENERIC_WRITE) ? "ab+" : "rb"; if (creation == CREATE_ALWAYS) mode = "wb+";
    FILE* f = std::fopen(p.c_str(), mode); return f ? reinterpret_cast<HANDLE>(new KainoteFileHandle(f)) : INVALID_HANDLE_VALUE;
}
inline HANDLE CreateFileA(const char* path, DWORD access, DWORD, void*, DWORD creation, DWORD, HANDLE) {
    std::string p = path ? std::string(path) : std::string(); std::replace(p.begin(), p.end(), '\\', '/');
    const char* mode = (access & GENERIC_WRITE) ? "ab+" : "rb"; if (creation == CREATE_ALWAYS) mode = "wb+";
    FILE* f = std::fopen(p.c_str(), mode); return f ? reinterpret_cast<HANDLE>(new KainoteFileHandle(f)) : INVALID_HANDLE_VALUE;
}
inline HANDLE CreateFile(const wchar_t* path, DWORD access, DWORD share, void* sec, DWORD creation, DWORD flags, HANDLE tmpl) { return CreateFileW(path, access, share, sec, creation, flags, tmpl); }
inline HANDLE CreateFile(const char* path, DWORD access, DWORD share, void* sec, DWORD creation, DWORD flags, HANDLE tmpl) { return CreateFileA(path, access, share, sec, creation, flags, tmpl); }
inline KainoteHandleBase* kainote_handle(HANDLE h) { return (!h || h == INVALID_HANDLE_VALUE) ? nullptr : reinterpret_cast<KainoteHandleBase*>(h); }
inline bool GetFileTime(HANDLE h, FILETIME*, FILETIME*, FILETIME* write) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::File || !write) return false;
    FILE* f = static_cast<KainoteFileHandle*>(base)->file;
    int fd=fileno(f); struct stat st{}; if (fstat(fd,&st)!=0) return false;
    std::uint64_t v=(std::uint64_t)st.st_mtime; write->dwLowDateTime=(DWORD)v; write->dwHighDateTime=(DWORD)(v>>32); return true;
}
inline bool CloseHandle(HANDLE h) { auto* base = kainote_handle(h); if (!base) return false; delete base; return true; }
inline DWORD GetLastError() { return errno; }
inline unsigned short GetSystemDefaultUILanguage() { return 0x409; }

struct KainoteFindHandle { std::vector<std::filesystem::directory_entry> entries; std::size_t index{}; };
inline std::wstring kainote_utf8_to_wstring(const std::string& s) { std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; return conv.from_bytes(s); }
inline std::string kainote_wstring_to_utf8(const std::wstring& s) { std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; return conv.to_bytes(s); }
inline std::string kainote_normalize_path(const wchar_t* path) { std::string p = kainote_wstring_to_utf8(path ? std::wstring(path) : std::wstring()); std::replace(p.begin(), p.end(), '\\', '/'); return p; }
inline std::string kainote_normalize_path(const char* path) { std::string p = path ? std::string(path) : std::string(); std::replace(p.begin(), p.end(), '\\', '/'); return p; }
inline void kainote_fill_find_data(const std::filesystem::directory_entry& e, WIN32_FIND_DATAW* data) {
    if (!data) return; *data = WIN32_FIND_DATAW{};
    auto name = kainote_utf8_to_wstring(e.path().filename().string());
    std::wcsncpy(data->cFileName, name.c_str(), MAX_PATH - 1);
    std::error_code ec; auto sz = e.is_regular_file(ec) ? e.file_size(ec) : 0; data->nFileSizeLow = static_cast<DWORD>(sz & 0xffffffffu);
    auto ft = e.last_write_time(ec); if (!ec) { auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ft - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()); auto tt = std::chrono::system_clock::to_time_t(sctp); std::uint64_t v = static_cast<std::uint64_t>(tt); data->ftLastWriteTime.dwLowDateTime = static_cast<DWORD>(v); data->ftLastWriteTime.dwHighDateTime = static_cast<DWORD>(v >> 32); }
}
inline HANDLE FindFirstFileW(const wchar_t* pattern, WIN32_FIND_DATAW* data) {
    std::string pat = kainote_normalize_path(pattern); auto star = pat.find('*'); std::filesystem::path dir = star == std::string::npos ? std::filesystem::path(pat).parent_path() : std::filesystem::path(pat.substr(0, star)).parent_path(); std::string prefix = star == std::string::npos ? std::filesystem::path(pat).filename().string() : std::filesystem::path(pat.substr(0, star)).filename().string();
    if (dir.empty()) dir = "."; std::error_code ec; if (!std::filesystem::exists(dir, ec)) return INVALID_HANDLE_VALUE;
    auto* h = new KainoteFindHandle();
    for (auto& e : std::filesystem::directory_iterator(dir, ec)) { if (ec) break; auto n = e.path().filename().string(); if (prefix.empty() || n.rfind(prefix, 0) == 0) h->entries.push_back(e); }
    if (h->entries.empty()) { delete h; return INVALID_HANDLE_VALUE; }
    kainote_fill_find_data(h->entries[0], data); return static_cast<HANDLE>(h);
}
inline HANDLE FindFirstFileA(const char* pattern, WIN32_FIND_DATA* data) { auto w = kainote_utf8_to_wstring(kainote_normalize_path(pattern)); return FindFirstFileW(w.c_str(), data); }
inline HANDLE FindFirstFileEx(const char* pattern, FINDEX_INFO_LEVELS, WIN32_FIND_DATA* data, int, void*, DWORD) { return FindFirstFileA(pattern, data); }
inline HANDLE FindFirstFileEx(const wchar_t* pattern, FINDEX_INFO_LEVELS, WIN32_FIND_DATA* data, int, void*, DWORD) { return FindFirstFileW(pattern, data); }
inline BOOL FindNextFile(HANDLE handle, WIN32_FIND_DATAW* data) { auto* h = static_cast<KainoteFindHandle*>(handle); if (!h) return 0; if (++h->index >= h->entries.size()) return 0; kainote_fill_find_data(h->entries[h->index], data); return 1; }
inline BOOL FindNextFileW(HANDLE handle, WIN32_FIND_DATAW* data) { return FindNextFile(handle, data); }
inline BOOL FindClose(HANDLE handle) { auto* h = static_cast<KainoteFindHandle*>(handle); delete h; return 1; }

constexpr DWORD ERROR_NO_MORE_FILES = 18;
constexpr DWORD ERROR_FILE_NOT_FOUND = 2;
constexpr DWORD ERROR_ACCESS_DENIED = 5;
constexpr DWORD FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x100;
constexpr DWORD FORMAT_MESSAGE_FROM_SYSTEM = 0x1000;
constexpr DWORD FORMAT_MESSAGE_IGNORE_INSERTS = 0x200;
inline DWORD FormatMessage(DWORD, void*, DWORD, DWORD, LPWSTR buffer, DWORD, void*) { static wchar_t msg[] = L"System error"; if (buffer) *reinterpret_cast<LPWSTR*>(buffer) = msg; return 12; }
inline DWORD FormatMessageA(DWORD, void*, DWORD error, DWORD, char* buffer, DWORD size, void*) {
    if (!buffer || size == 0) return 0;
    const char* msg = dlerror();
    if (!msg) msg = std::strerror(error ? static_cast<int>(error) : errno);
    if (!msg) msg = "System error";
    std::snprintf(buffer, size, "%s", msg);
    return static_cast<DWORD>(std::strlen(buffer));
}
inline void* LocalFree(void*) { return nullptr; }
inline BOOL SetFileTime(HANDLE, const FILETIME*, const FILETIME*, const FILETIME*) { return TRUE; }
inline BOOL CopyFile(const wchar_t* from, const wchar_t* to, BOOL failIfExists) { std::error_code ec; auto src = std::filesystem::path(kainote_normalize_path(from)); auto dst = std::filesystem::path(kainote_normalize_path(to)); if (failIfExists && std::filesystem::exists(dst, ec)) return FALSE; std::filesystem::create_directories(dst.parent_path(), ec); return std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec); }
inline FILE* kainote_wfopen(const wchar_t* path, const wchar_t* mode) { std::string p = kainote_normalize_path(path); std::string m = kainote_wstring_to_utf8(mode ? std::wstring(mode) : std::wstring(L"rb")); return std::fopen(p.c_str(), m.c_str()); }
inline int kainote_wremove(const wchar_t* path) { std::string p = kainote_normalize_path(path); return std::remove(p.c_str()); }
struct TIME_ZONE_INFORMATION { LONG Bias{}; };
inline DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION*) { return 0; }
inline BOOL SystemTimeToTzSpecificLocalTime(const TIME_ZONE_INFORMATION*, const SYSTEMTIME* universal, SYSTEMTIME* local) { if (!universal || !local) return FALSE; *local = *universal; return TRUE; }
#define _wfopen kainote_wfopen
#define _wremove kainote_wremove
inline int kainote_wrename(const wchar_t* oldp, const wchar_t* newp) { return std::rename(kainote_normalize_path(oldp).c_str(), kainote_normalize_path(newp).c_str()); }
#define _wrename kainote_wrename
#define _fseeki64 fseeko
#define _int64 long long
#define __int64 long long
#define _strdup strdup
#define vsprintf_s(buffer, size, fmt, args) vsnprintf((buffer), (size), (fmt), (args))
#define _snwprintf swprintf
#define _tcsncpy wcsncpy
#ifndef __stdcall
#define __stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
constexpr int WH_CBT = 5;
constexpr int HCBT_MINMAX = 1;
using TIMERPROC = void (*)();
constexpr int WH_KEYBOARD = 2;
constexpr int WH_GETMESSAGE = 3;
inline HHOOK SetWindowsHookEx(int, LRESULT (__stdcall *)(int, WPARAM, LPARAM), void*, DWORD) { return nullptr; }
inline BOOL UnhookWindowsHookEx(HHOOK) { return 1; }
inline LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM) { return 0; }
inline UINT SetTimer(HWND, UINT id, UINT, TIMERPROC proc) { if (proc) proc(); return id; }
inline BOOL KillTimer(HWND, UINT) { return 1; }
#ifndef MAXINT
#define MAXINT INT32_MAX
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef swscanf_s
#define swscanf_s swscanf
#endif
#ifndef TEXT
#define TEXT(x) L##x
#endif
constexpr int FW_NORMAL = 400;
constexpr int FW_BOLD = 700;
constexpr int DEFAULT_CHARSET = 1;
constexpr int OUT_TT_ONLY_PRECIS = 7;
constexpr int OUT_DEFAULT_PRECIS = 0;
constexpr int OUT_TT_PRECIS = 4;
constexpr int CLIP_DEFAULT_PRECIS = 0;
constexpr int ANTIALIASED_QUALITY = 4;
constexpr int MM_TEXT = 1;
constexpr int DEFAULT_PITCH = 0;
constexpr int FF_DONTCARE = 0;
constexpr int CLEARTYPE_QUALITY = 5;
struct SYSTEM_INFO { DWORD dwNumberOfProcessors{1}; };
inline void GetSystemInfo(SYSTEM_INFO* si) { if (si) si->dwNumberOfProcessors = 1; }
constexpr int SM_CXICON = 11;
constexpr int SM_CYICON = 12;
constexpr DWORD WAIT_OBJECT_0 = 0;
constexpr DWORD WAIT_TIMEOUT = 258;
constexpr DWORD WAIT_FAILED = 0xffffffffu;
constexpr DWORD INFINITE = 0xffffffffu;
inline HANDLE CreateEvent(void*, BOOL manualReset, BOOL initialState, const wchar_t*) { return reinterpret_cast<HANDLE>(new KainoteEventHandle(manualReset != FALSE, initialState != FALSE)); }
inline BOOL SetEvent(HANDLE h) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::Event) return FALSE;
    auto* ev = static_cast<KainoteEventHandle*>(base);
    { std::lock_guard<std::mutex> lock(ev->mutex); ev->signaled = true; }
    ev->cv.notify_all();
    return TRUE;
}
inline BOOL ResetEvent(HANDLE h) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::Event) return FALSE;
    auto* ev = static_cast<KainoteEventHandle*>(base);
    std::lock_guard<std::mutex> lock(ev->mutex);
    ev->signaled = false;
    return TRUE;
}
inline DWORD WaitForSingleObject(HANDLE h, DWORD ms) {
    auto* base = kainote_handle(h);
    if (!base) return WAIT_FAILED;
    if (base->kind == KainoteHandleBase::Kind::Thread) {
        auto* th = static_cast<KainoteThreadHandle*>(base);
        if (ms == INFINITE) {
            if (th->thread.joinable()) th->thread.join();
            th->finished = true;
            return WAIT_OBJECT_0;
        }
        auto start = std::chrono::steady_clock::now();
        while (!th->finished.load()) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= ms) return WAIT_TIMEOUT;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (th->thread.joinable()) th->thread.join();
        return WAIT_OBJECT_0;
    }
    if (base->kind == KainoteHandleBase::Kind::Event) {
        auto* ev = static_cast<KainoteEventHandle*>(base);
        std::unique_lock<std::mutex> lock(ev->mutex);
        bool ok = false;
        if (ms == INFINITE) { ev->cv.wait(lock, [&]{ return ev->signaled; }); ok = true; }
        else ok = ev->cv.wait_for(lock, std::chrono::milliseconds(ms), [&]{ return ev->signaled; });
        if (!ok) return WAIT_TIMEOUT;
        if (!ev->manualReset) ev->signaled = false;
        return WAIT_OBJECT_0;
    }
    return WAIT_FAILED;
}
inline DWORD WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll, DWORD ms) {
    if (!handles || !count) return WAIT_FAILED;
    if (waitAll) {
        for (DWORD i = 0; i < count; ++i) {
            DWORD r = WaitForSingleObject(handles[i], ms);
            if (r != WAIT_OBJECT_0) return r;
        }
        return WAIT_OBJECT_0;
    }
    auto start = std::chrono::steady_clock::now();
    while (true) {
        for (DWORD i = 0; i < count; ++i) {
            DWORD r = WaitForSingleObject(handles[i], 0);
            if (r == WAIT_OBJECT_0) return WAIT_OBJECT_0 + i;
        }
        if (ms != INFINITE && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= ms) return WAIT_TIMEOUT;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
constexpr int THREAD_PRIORITY_TIME_CRITICAL = 15;
constexpr int THREAD_PRIORITY_LOWEST = -2;
inline BOOL SetThreadPriority(HANDLE, int) { return TRUE; }
using KainoteThreadProc = unsigned int (__stdcall *)(void*);
inline uintptr_t _beginthreadex(void*, unsigned, KainoteThreadProc proc, void* data, unsigned, unsigned int* threadid) {
    if (!proc) return 0;
    if (threadid) *threadid = GetCurrentThreadId();
    auto* handle = new KainoteThreadHandle();
    handle->thread = std::thread([proc, data, handle]() { proc(data); handle->finished = true; });
    return reinterpret_cast<uintptr_t>(handle);
}
inline void Sleep(DWORD ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
using WAITORTIMERCALLBACK = void (CALLBACK *)(PVOID, BOOLEAN);
using LPTHREAD_START_ROUTINE = DWORD (*)(void*);
inline HANDLE CreateThread(void*, size_t, LPTHREAD_START_ROUTINE proc, void* data, DWORD, DWORD* threadid) {
    if (!proc) return nullptr;
    unsigned int tid{};
    auto h = _beginthreadex(nullptr, 0, reinterpret_cast<KainoteThreadProc>(proc), data, 0, &tid);
    if (threadid) *threadid = tid;
    return reinterpret_cast<HANDLE>(h);
}
inline BOOL CreateTimerQueueTimer(HANDLE* out, HANDLE, WAITORTIMERCALLBACK cb, PVOID param, DWORD due, DWORD period, ULONG) {
    if (!cb) return FALSE;
    auto* timer = new KainoteTimerHandle(std::thread());
    timer->thread = std::thread([timer, cb, param, due, period]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(due));
        do {
            if (timer->cancelled) break;
            cb(param, TRUE);
            if (!period) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(period));
        } while (!timer->cancelled);
    });
    if (out) *out = reinterpret_cast<HANDLE>(timer);
    return TRUE;
}
inline BOOL DeleteTimerQueueTimer(HANDLE, HANDLE timer, HANDLE) { return CloseHandle(timer); }
constexpr int WM_SETICON = 0x0080;
constexpr unsigned ES_DISPLAY_REQUIRED = 0x2;
constexpr unsigned ES_CONTINUOUS = 0x80000000u;
inline unsigned SetThreadExecutionState(unsigned state) { return state; }
constexpr int ICON_BIG = 1;
inline int GetSystemMetrics(int metric) { return (metric == SM_CXICON || metric == SM_CYICON) ? 32 : 0; }
constexpr int LOGPIXELSY = 90;
constexpr DWORD FILE_NOTIFY_CHANGE_FILE_NAME = 0x1;
constexpr DWORD FR_PRIVATE = 0x10;
constexpr int CSIDL_LOCAL_APPDATA = 0x001c;
constexpr int CSIDL_FLAG_CREATE = 0x8000;
constexpr DWORD GGI_MARK_NONEXISTING_GLYPHS = 0x1;
constexpr DWORD GDI_ERROR = 0xffffffffu;
inline HRESULT SHGetFolderPath(HWND, int, HANDLE, DWORD, LPWSTR path) { if (!path) return -1; const char* home = std::getenv("HOME"); std::wstring w = kainote_utf8_to_wstring(std::string(home ? home : "/tmp") + "/.local/share"); std::wcsncpy(path, w.c_str(), MAX_PATH - 1); path[MAX_PATH - 1] = L'\0'; return 0; }
inline DWORD GetGlyphIndicesW(HDC, LPCWSTR text, int count, LPWORD indices, DWORD) { if (!text || !indices) return GDI_ERROR; if (count < 0) count = std::wcslen(text); for (int i = 0; i < count; ++i) indices[i] = text[i] ? static_cast<WORD>(text[i]) : 0xffff; return static_cast<DWORD>(count); }
inline HDC GetDC(HWND) { return nullptr; }
inline HDC CreateCompatibleDC(HDC) { return reinterpret_cast<HDC>(new int(0)); }
inline BOOL DeleteDC(HDC dc) { delete reinterpret_cast<int*>(dc); return TRUE; }
inline int SetMapMode(HDC, int mode) { return mode; }
inline int GetDeviceCaps(HDC, int) { return 96; }
inline int ReleaseDC(HWND, HDC) { return 1; }
inline HGDIOBJ CreateFontIndirectW(const LOGFONTW*) { return nullptr; }
#define CreateFontIndirect CreateFontIndirectW
inline HGDIOBJ SelectObject(HDC, HGDIOBJ obj) { return obj; }
inline BOOL DeleteObject(HGDIOBJ) { return TRUE; }
inline BOOL GetTextExtentPoint32(HDC, const wchar_t* text, int len, SIZE* size) { if (size) { size->cx = std::max(0, len) * 8 * 64; size->cy = 16 * 64; } return TRUE; }
inline BOOL GetTextMetrics(HDC, TEXTMETRIC* tm) { if (tm) { tm->tmDescent = 3 * 64; tm->tmExternalLeading = 0; } return TRUE; }
inline int EnumFontFamiliesEx(HDC, LOGFONTW*, FONTENUMPROC proc, LPARAM lParam, DWORD) { if (proc) { LOGFONTW lf{}; std::wcscpy(lf.lfFaceName, L"Sans"); TEXTMETRIC tm{}; proc(&lf, &tm, 0, lParam); } return 1; }
inline DWORD GetFontData(HDC, DWORD, DWORD, void*, DWORD) { return GDI_ERROR; }
inline HANDLE FindFirstChangeNotification(const wchar_t*, BOOL, DWORD) { return INVALID_HANDLE_VALUE; }
inline BOOL FindNextChangeNotification(HANDLE) { return FALSE; }
inline BOOL FindCloseChangeNotification(HANDLE) { return TRUE; }
inline int AddFontResourceExW(const wchar_t*, DWORD, void*) { return 0; }
inline BOOL RemoveFontResourceExW(const wchar_t*, DWORD, void*) { return TRUE; }
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline HICON GetHiconOf(...) { return nullptr; }
inline HRESULT CoInitialize(void*) { return 0; }
inline void CoUninitialize() {}
inline ITEMIDLIST* ILCreateFromPathW(const wchar_t*) { return nullptr; }
inline void ILFree(ITEMIDLIST*) {}
inline HRESULT SHOpenFolderAndSelectItems(ITEMIDLIST*, unsigned, void*, DWORD) { return 0; }
struct SHELLEXECUTEINFO { DWORD cbSize{}; const wchar_t* lpFile{}; const wchar_t* lpVerb{}; int nShow{}; DWORD fMask{}; };
template <typename T> struct WinStruct : public T { WinStruct() { std::memset(static_cast<T*>(this), 0, sizeof(T)); this->cbSize = sizeof(T); } };
constexpr int SW_RESTORE = 9;
constexpr int SW_SHOWMINNOACTIVE = 7;
constexpr int SW_MINIMIZE = 6;
constexpr DWORD SEE_MASK_FLAG_NO_UI = 0x400;
inline BOOL ShellExecuteEx(SHELLEXECUTEINFO* sei) {
    if (!sei || !sei->lpFile) return FALSE;
    std::string target = kainote_wstring_to_utf8(sei->lpFile);
    std::string command = "xdg-open \"";
    for (char ch : target) { if (ch == '"' || ch == '\\') command.push_back('\\'); command.push_back(ch); }
    command += "\" >/dev/null 2>&1 &";
    return std::system(command.c_str()) == 0;
}
struct SHFILEOPSTRUCT { HWND hwnd{}; UINT wFunc{}; LPCWSTR pFrom{}; LPCWSTR pTo{}; unsigned short fFlags{}; };
constexpr UINT FO_DELETE = 0x0003;
constexpr unsigned short FOF_SILENT = 0x0004;
constexpr unsigned short FOF_NOERRORUI = 0x0400;
constexpr unsigned short FOF_NOCONFIRMATION = 0x0010;
constexpr unsigned short FOF_ALLOWUNDO = 0x0040;
inline int SHFileOperation(SHFILEOPSTRUCT* op) {
    if (!op || op->wFunc != FO_DELETE || !op->pFrom) return 1;
    std::error_code ec;
    std::filesystem::path p(kainote_normalize_path(op->pFrom));
    if (std::filesystem::is_directory(p, ec)) std::filesystem::remove_all(p, ec);
    else std::filesystem::remove(p, ec);
    return ec ? 1 : 0;
}
inline BOOL ShowWindow(HWND, int) { return TRUE; }
inline HKEY HKEY_CURRENT_USER = reinterpret_cast<HKEY>(static_cast<std::uintptr_t>(1));
constexpr DWORD KEY_ALL_ACCESS = 0xf003f;
constexpr DWORD REG_OPTION_NON_VOLATILE = 0;
constexpr DWORD REG_SZ = 1;
inline long RegOpenKeyEx(HKEY, LPCWSTR, DWORD, DWORD, HKEY*) { return 1; }
inline long RegCreateKeyEx(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, DWORD, void*, HKEY* out, DWORD*) { if (out) *out = reinterpret_cast<HKEY>(new int(0)); return 0; }
inline long RegCloseKey(HKEY h) { delete reinterpret_cast<int*>(h); return 0; }
inline long RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, const BYTE*, DWORD) { return 0; }
template <typename TypePtr, typename SizePtr>
inline long RegQueryValueExW(HKEY, LPCWSTR, DWORD*, TypePtr type, LPBYTE, SizePtr) { if (type) *type = REG_SZ; return 1; }
inline long RegDeleteTree(HKEY, LPCWSTR) { return 0; }
constexpr long SHCNE_ASSOCCHANGED = 0x08000000L;
constexpr unsigned SHCNF_IDLIST = 0;
inline void SHChangeNotify(long, unsigned, const void*, const void*) {}
inline UINT RegisterWindowMessage(LPCWSTR) { static UINT next = 0xC000; return next++; }
struct STARTUPINFO { DWORD cb{}; };
struct PROCESS_INFORMATION { HANDLE hProcess{}; HANDLE hThread{}; DWORD dwProcessId{}; DWORD dwThreadId{}; };
inline BOOL CreateProcessW(LPCWSTR, LPWSTR commandLine, void*, void*, BOOL, DWORD, void*, LPCWSTR, STARTUPINFO*, PROCESS_INFORMATION*) {
    if (!commandLine) return FALSE;
    std::string command = kainote_wstring_to_utf8(commandLine);
    return std::system(command.c_str()) == 0;
}
constexpr const wchar_t* RT_RCDATA = L"RCDATA";
inline HRSRC FindResource(void*, const wchar_t*, const wchar_t*) { return nullptr; }
inline HGLOBAL LoadResource(void*, HRSRC) { return nullptr; }
inline void* LockResource(HGLOBAL) { return nullptr; }
inline DWORD SizeofResource(void*, HRSRC) { return 0; }
#define GetHwnd GetHandle
constexpr DWORD FILE_ATTRIBUTE_DIRECTORY = 0x10;
constexpr DWORD LOAD_WITH_ALTERED_SEARCH_PATH = 0x00000008;
inline HMODULE LoadLibraryExW(const wchar_t* filename, void*, DWORD) {
    std::string path = kainote_normalize_path(filename);
    return dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
}
inline FARPROC GetProcAddress(HMODULE module, const char* name) { return module ? dlsym(module, name) : nullptr; }
inline BOOL FreeLibrary(HMODULE module) { return module ? (dlclose(module) == 0) : FALSE; }
inline DWORD GetModuleFileNameW(HMODULE, wchar_t* buffer, DWORD size) {
    if (!buffer || size == 0) return 0;
    char path[MAX_PATH]{};
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n < 0) return 0;
    path[n] = '\0';
    auto w = kainote_utf8_to_wstring(path);
    std::wcsncpy(buffer, w.c_str(), size - 1);
    buffer[size - 1] = L'\0';
    return static_cast<DWORD>(std::wcslen(buffer));
}
inline DWORD GetFileAttributesW(const wchar_t* path) { std::string p = kainote_normalize_path(path); struct stat st{}; if (stat(p.c_str(), &st) != 0) return INVALID_FILE_ATTRIBUTES; DWORD attr = FILE_ATTRIBUTE_NORMAL; if (S_ISDIR(st.st_mode)) attr |= FILE_ATTRIBUTE_DIRECTORY; if (access(p.c_str(), W_OK) != 0) attr |= FILE_ATTRIBUTE_READONLY; return attr; }
#ifndef GetHWND
#define GetHWND GetHandle
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef S_OK
#define S_OK 0
#define E_FAIL 0x80004005L
#endif

#ifndef __stdcall
#define __stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)

struct tagRECT { long left, top, right, bottom; };
using RECT = tagRECT;
using LPRECT = RECT*;
struct MONITORINFO { DWORD cbSize{sizeof(MONITORINFO)}; RECT rcMonitor{0,0,1920,1080}; RECT rcWork{0,0,1920,1040}; DWORD dwFlags{1}; };
constexpr DWORD MONITORINFOF_PRIMARY = 1;
using MONITORENUMPROC = int (CALLBACK *)(HMONITOR, HDC, LPRECT, LPARAM);
inline BOOL GetMonitorInfo(HMONITOR, MONITORINFO* info) { if (info) { info->rcMonitor = {0,0,1920,1080}; info->rcWork = {0,0,1920,1040}; info->dwFlags = MONITORINFOF_PRIMARY; } return TRUE; }
inline BOOL EnumDisplayMonitors(HDC, const RECT*, MONITORENUMPROC proc, LPARAM data) { if (!proc) return FALSE; RECT r{0,0,1920,1080}; return proc(nullptr, nullptr, &r, data); }

#endif // !_WIN32
