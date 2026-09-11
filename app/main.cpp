#include <iostream>
#include <string_view>

int wmain(int argc, wchar_t* argv[]) {
    if (argc > 1 && std::wstring_view{argv[1]} == L"--version") {
        std::cout << "SmartShell " << SMARTSHELL_VERSION << '\n';
        return 0;
    }

    std::cout << "SmartShell bootstrap " << SMARTSHELL_VERSION << '\n';
    return 0;
}
