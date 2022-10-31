#include "pch.h"
#include "ProcessHelper.h"

CString ProcessHelper::GetProcessName(DWORD pid) {
    static std::unordered_map<DWORD, CString> processes;
    CString name;
    HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        WCHAR path[MAX_PATH];
        DWORD size = _countof(path);
        if (::QueryFullProcessImageName(hProcess, 0, path, &size))
            name = wcsrchr(path, L'\\') + 1;
        ::CloseHandle(hProcess);
    }
    if(name.IsEmpty()) {
        if (processes.empty()) {
            auto hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            ATLASSERT(hSnap != INVALID_HANDLE_VALUE);
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            ::Process32First(hSnap, &pe);
            while (::Process32Next(hSnap, &pe)) {
                if (wcsrchr(pe.szExeFile, L'\\') == nullptr)
                    processes.insert({ pe.th32ProcessID, pe.szExeFile });
            }
            ::CloseHandle(hSnap);
        }
        name = processes.at(pid);
    }
    return name;
}
