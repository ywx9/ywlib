#pragma once
#include <ywx/core.h>

namespace yw {

/// MARK: ok/yes

inline bool ok(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_OK | (topmost ? MB_TOPMOST : 0) | (modal ? MB_TASKMODAL : 0);
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDOK;
}

inline bool yes(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_YESNO | (topmost ? MB_TOPMOST : 0) | (modal ? MB_TASKMODAL : 0);
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDYES;
}

/// MARK: open_file_dialog

enum class file_dialog_option {
  none = 0,
  multi_select = 1,
  save_file = 2,
};

inline std::expected<std::vector<file::path_string>, error> open_file_dialog(
  null_terminated<wchar_t> Title = L"Open File", file_dialog_option Option = file_dialog_option::none,
  const wchar_t* Filter = L"All Files\0*.*\0") {
  switch (Option) {
  case file_dialog_option::none: [[fallthrough]];
  case file_dialog_option::multi_select: [[fallthrough]];
  case file_dialog_option::save_file: break;
  default: return std::unexpected(error(errors::invalid_argument, "invalid file_dialog_option"));
  }
  std::vector<wchar_t> buffer(65536);
  OPENFILENAMEW ofn{
    .lStructSize = sizeof(OPENFILENAMEW),
    .lpstrFilter = Filter,
    .lpstrFile = buffer.data(),
    .nMaxFile = static_cast<DWORD>(buffer.size()),
    .lpstrTitle = Title.data(),
    .Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR};
  if (Option == file_dialog_option::save_file) {
    ofn.Flags |= OFN_OVERWRITEPROMPT;
    if (!::GetSaveFileNameW(&ofn)) {
      const auto code = ::CommDlgExtendedError();
      if (code == 0) return std::vector<file::path_string>();
      return std::unexpected(error(errors::operation_failed, "GetSaveFileNameW failed", int32_t(code)));
    }
    return std::vector<file::path_string>{file::path_string(buffer.data())};
  } else {
    ofn.Flags |= OFN_FILEMUSTEXIST;
    if (Option == file_dialog_option::multi_select) ofn.Flags |= OFN_ALLOWMULTISELECT;
    if (!::GetOpenFileNameW(&ofn)) {
      const auto code = ::CommDlgExtendedError();
      if (code == 0) return std::vector<file::path_string>();
      return std::unexpected(error(errors::operation_failed, "GetOpenFileNameW failed", int32_t(code)));
    }
    std::vector<file::path_string> result;
    const wchar_t* first = buffer.data();
    const auto first_len = std::char_traits<wchar_t>::length(first);
    const wchar_t* p = first + first_len + 1;
    if (*p == L'\0') {
      result.emplace_back(first);
      return result;
    }
    const string_view<wchar_t> directory(first, first_len);
    while (*p != L'\0') {
      const auto name_len = std::char_traits<wchar_t>::length(p);
      result.push_back(file::join(directory, string_view<wchar_t>(p, name_len)));
      p += name_len + 1;
    }
    return result;
  }
}

/// MARK: select_folder_dialog

inline std::expected<std::optional<file::path_string>, error> select_folder_dialog(
  null_terminated<wchar_t> Title = L"Select Folder") {
  wchar_t buffer[MAX_PATH]{};
  BROWSEINFOW bi{.lpszTitle = Title.data(), .ulFlags = BIF_RETURNONLYFSDIRS};

  const auto pidl = ::SHBrowseForFolderW(&bi);
  if (!pidl) return std::optional<file::path_string>();

  const bool ok = ::SHGetPathFromIDListW(pidl, buffer);
  ::CoTaskMemFree(pidl);
  if (!ok) return std::unexpected(error(errors::operation_failed, "SHGetPathFromIDListW failed"));
  return file::path_string(buffer);
}

} // namespace yw
