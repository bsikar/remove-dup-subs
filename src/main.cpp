#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

std::set<std::string> get_sub_dirs(const fs::path &path) {
  std::set<std::string> sub_dirs;
  if (fs::exists(path) && fs::is_directory(path)) {
    for (const auto &entry : fs::directory_iterator(path)) {
      if (fs::is_directory(entry.status())) {
        sub_dirs.insert(entry.path().filename().string());
      }
    }
  }
  return sub_dirs;
}

std::map<std::string, std::string>
get_files_matching_pattern(const fs::path &path) {
  std::map<std::string, std::string> matching_files;
  std::regex pattern(".*-\\s*(S\\d+E\\d+).*", std::regex_constants::icase);

  for (const auto &entry : fs::directory_iterator(path)) {
    if (fs::is_regular_file(entry.status())) {
      std::string filename = entry.path().filename().string();

      if (entry.path().extension() == ".srt") {
        continue;
      }

      std::smatch matches;
      if (std::regex_match(filename, matches, pattern)) {
        if (matches.size() > 1) {
          matching_files[matches[1].str()] = filename;
        }
      }
    }
  }
  return matching_files;
}

bool contains_english(const std::string &filename) {
  std::regex pattern(".*\\[.*EN.*\\].*");
  return std::regex_search(filename, pattern);
}

std::string extract_show_name(const std::string &filename) {
  std::regex  pattern("^(.*?) - S\\d+E\\d+ - .*");
  std::smatch matches;
  if (std::regex_match(filename, matches, pattern) && matches.size() > 1) {
    return matches[1].str();
  }
  return "Unknown Show";
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <path (dubs & subs)> <path2 (subs only)>\n"
              << "\tPlease Note that the files in path2 will get deleted "
                 "when prompted\n";
    return 1;
  }

  fs::path path1 = argv[1];
  fs::path path2 = argv[2];

  auto sub_dirs1 = get_sub_dirs(path1);
  auto sub_dirs2 = get_sub_dirs(path2);

  std::map<std::string, std::vector<std::pair<std::string, std::string>>>
      confirmed_matches;

  for (const auto &dir : sub_dirs1) {
    if (sub_dirs2.find(dir) != sub_dirs2.end()) {
      auto sub_sub_dirs1 = get_sub_dirs(path1 / dir);
      auto sub_sub_dirs2 = get_sub_dirs(path2 / dir);

      for (const auto &sub_dir : sub_sub_dirs1) {
        if (sub_sub_dirs2.find(sub_dir) != sub_sub_dirs2.end()) {

          auto files1 = get_files_matching_pattern(path1 / dir / sub_dir);
          auto files2 = get_files_matching_pattern(path2 / dir / sub_dir);

          for (const auto &[match, file1] : files1) {
            auto it = files2.find(match);
            if (it != files2.end()) {
              std::string file_path1 = (path1 / dir / sub_dir / file1).string();
              std::string file_path2 =
                  (path2 / dir / sub_dir / it->second).string();

              if (contains_english(file_path1)) {
                std::string show_name = extract_show_name(file_path1);
                confirmed_matches[show_name].push_back(
                    std::make_pair(file_path1, file_path2));
              } else {
                std::cout << "Does not contain English. Skipping.\n";
              }
            }
          }
        }
      }
    }
  }

  std::cout << "\nConfirmed Matches (Files to be removed):\n";
  for (const auto &[show, path_pairs] : confirmed_matches) {
    std::cout << show << ":\n";
    for (const auto &path_pair : path_pairs) {
      std::cout << "Do you want to remove \"" << path_pair.second
                << "\"? (y/n): ";
      char response;
      std::cin >> response;
      if (response == 'y' || response == 'Y') {
        try {
          if (fs::remove(path_pair.second)) {
            std::cout << "Removed: " << path_pair.second << "\n";
          } else {
            std::cerr << "Failed to remove (file may not exist): "
                      << path_pair.second << "\n";
          }
        } catch (const fs::filesystem_error &e) {
          std::cerr << "Error: " << e.what() << "\n";
        }
      } else {
        std::cout << "Skipping removal of: " << path_pair.second << "\n";
      }
    }
  }

  return 0;
}
