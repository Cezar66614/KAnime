#include "./comm.hpp"

int main() {
  for (const auto &entry : std::filesystem::directory_iterator("/mnt/HDD/Anime/English Subs")) {
    if (entry.is_directory())
      std::printf("%s: %d\n", truncateDirectoryName(entry.path().filename().generic_string()).c_str(), getLastDownEp(entry.path().generic_string()));
  }

  return 0;
}
