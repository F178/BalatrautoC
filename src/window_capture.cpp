#include "window_capture.hpp"

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
struct WindowCandidate {
    HWND handle = nullptr;
    int score = 0;
};

std::wstring lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t character) {
        return static_cast<wchar_t>(std::towlower(character));
    });
    return value;
}

std::wstring processName(HWND window) {
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!process) return {};
    std::vector<wchar_t> path(32768);
    DWORD length = static_cast<DWORD>(path.size());
    std::wstring result;
    if (QueryFullProcessImageNameW(process, 0, path.data(), &length)) {
        result.assign(path.data(), length);
        const std::size_t separator = result.find_last_of(L"\\/");
        if (separator != std::wstring::npos) result.erase(0, separator + 1);
    }
    CloseHandle(process);
    return lower(std::move(result));
}

BOOL CALLBACK inspectWindow(HWND window, LPARAM parameter) {
    if (!IsWindowVisible(window) || IsIconic(window)) return TRUE;
    RECT client{};
    if (!GetClientRect(window, &client) || client.right - client.left < 160 || client.bottom - client.top < 120) {
        return TRUE;
    }

    int score = 0;
    const std::wstring executable = processName(window);
    if (executable == L"balatro.exe") score = 100;

    const int titleLength = GetWindowTextLengthW(window);
    std::wstring title(static_cast<std::size_t>(std::max(0, titleLength)) + 1U, L'\0');
    if (titleLength > 0) {
        GetWindowTextW(window, title.data(), titleLength + 1);
        title.resize(static_cast<std::size_t>(titleLength));
        const std::wstring normalized = lower(title);
        if (normalized == L"balatro") score = std::max(score, 80);
        else if (normalized.find(L"balatro") != std::wstring::npos &&
                 normalized.find(L"balatrauto") == std::wstring::npos) {
            score = std::max(score, 50);
        }
    }

    auto* candidate = reinterpret_cast<WindowCandidate*>(parameter);
    if (score > candidate->score) {
        candidate->handle = window;
        candidate->score = score;
    }
    return TRUE;
}

bool captureClient(HWND window, std::vector<unsigned char>& rgba, int& width, int& height, std::string* error) {
    RECT client{};
    if (!GetClientRect(window, &client)) {
        if (error) *error = "Could not read the Balatro window bounds";
        return false;
    }
    POINT origin{client.left, client.top};
    if (!ClientToScreen(window, &origin)) {
        if (error) *error = "Could not locate the Balatro window on screen";
        return false;
    }
    width = client.right - client.left;
    height = client.bottom - client.top;
    if (width <= 0 || height <= 0) {
        if (error) *error = "The Balatro window has no visible client area";
        return false;
    }

    HDC screen = GetDC(nullptr);
    HDC memory = screen ? CreateCompatibleDC(screen) : nullptr;
    if (!screen || !memory) {
        if (memory) DeleteDC(memory);
        if (screen) ReleaseDC(nullptr, screen);
        if (error) *error = "Could not access the desktop image";
        return false;
    }

    BITMAPINFO bitmap{};
    bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth = width;
    bitmap.bmiHeader.biHeight = -height;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    bitmap.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP image = CreateDIBSection(screen, &bitmap, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!image || !bits) {
        if (image) DeleteObject(image);
        DeleteDC(memory);
        ReleaseDC(nullptr, screen);
        if (error) *error = "Could not allocate the window capture";
        return false;
    }

    HGDIOBJ previous = SelectObject(memory, image);
    const BOOL copied = BitBlt(
        memory,
        0,
        0,
        width,
        height,
        screen,
        origin.x,
        origin.y,
        SRCCOPY | CAPTUREBLT);
    rgba.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U);
    if (copied) {
        const auto* bgra = static_cast<const unsigned char*>(bits);
        for (std::size_t index = 0; index < rgba.size(); index += 4U) {
            rgba[index] = bgra[index + 2U];
            rgba[index + 1U] = bgra[index + 1U];
            rgba[index + 2U] = bgra[index];
            rgba[index + 3U] = 255;
        }
    }
    SelectObject(memory, previous);
    DeleteObject(image);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);

    if (!copied) {
        rgba.clear();
        if (error) *error = "Windows could not capture the visible Balatro window";
        return false;
    }
    return true;
}

bool saveBitmap(
    const std::filesystem::path& outputPath,
    const std::vector<unsigned char>& rgba,
    int width,
    int height,
    std::string* error) {
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        if (error) *error = "Could not create the Balatro capture file";
        return false;
    }

    BITMAPFILEHEADER fileHeader{};
    BITMAPINFOHEADER infoHeader{};
    const DWORD pixelBytes = static_cast<DWORD>(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U);
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + pixelBytes;
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = -height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = pixelBytes;

    std::vector<unsigned char> bgra(rgba.size());
    for (std::size_t index = 0; index < rgba.size(); index += 4U) {
        bgra[index] = rgba[index + 2U];
        bgra[index + 1U] = rgba[index + 1U];
        bgra[index + 2U] = rgba[index];
        bgra[index + 3U] = rgba[index + 3U];
    }
    output.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    output.write(reinterpret_cast<const char*>(bgra.data()), static_cast<std::streamsize>(bgra.size()));
    if (!output.good()) {
        if (error) *error = "Could not finish writing the Balatro capture";
        return false;
    }
    return true;
}
}
#endif

bool WindowCapture::captureBalatro(const std::filesystem::path& outputPath, std::string* error) {
#ifdef _WIN32
    WindowCandidate candidate;
    EnumWindows(inspectWindow, reinterpret_cast<LPARAM>(&candidate));
    if (!candidate.handle) {
        if (error) *error = "No visible Balatro window was found";
        return false;
    }

    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    if (!captureClient(candidate.handle, pixels, width, height, error)) return false;

    std::error_code directoryError;
    std::filesystem::create_directories(outputPath.parent_path(), directoryError);
    if (directoryError) {
        if (error) *error = "Could not create the capture folder";
        return false;
    }
    if (!saveBitmap(outputPath, pixels, width, height, error)) return false;
    if (error) error->clear();
    return true;
#else
    static_cast<void>(outputPath);
    if (error) *error = "Window capture is available on Windows only";
    return false;
#endif
}
