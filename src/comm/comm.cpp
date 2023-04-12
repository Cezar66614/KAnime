#include "./comm.hpp"

size_t curlToString(void *contents, size_t size, size_t nmemb, std::string *response) {
  size_t newLength = size*nmemb;
  response->append((char*)contents, newLength);

  return newLength;
}
size_t curlToFile(void *contents, size_t size, size_t nmemb, void *fp) {
  FILE *file = (FILE *)fp;
  if (!file) { std::fprintf(stderr, "No file!\n"); return 0; }

  size_t written = std::fwrite((FILE *)contents, size, nmemb, file);
  return written;
}
void urlImageToDisk(std::string link, std::string path) { path += "/image.jpg";
  FILE* fp = fopen(path.c_str(), "wb");
  if (!fp) { std::fprintf(stderr, "Failed to create image on the disk!\n"); return; }

  CURL *curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 
    
    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      long int res_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
      if (!((res_code == 200 || res_code == 201) && res != CURLE_ABORTED_BY_CALLBACK)) { std::fprintf(stderr, "Response code: %ld\n", res_code); return; }

    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    std::fclose(fp);

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
}
std::string truncateDirectoryName(std::string str) { size_t i, siz;
  do {
    i = str.find('[');
    if (i != std::string::npos) str.erase(i, str.find(']', i) - i + 1);
    else break;
  } while (true);

  siz = str.size();
  for (i = 0; i < siz && str[i] == ' '; ++i);
  str.erase(0, i);

  siz = str.size() - 1;
  for (i = siz; i >= 0 && str[i] == ' '; --i);
  str.erase(i+1, siz-i);

  return str;
}

int getLastDownEp(std::string path) { int epNr, lepNr = -1, i, j; std::string filename;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (!entry.is_directory()) {
      filename = entry.path().filename().string();
      epNr = getEpNumber(filename);

      if (epNr > lepNr) lepNr = epNr;
    }
  }

  return lepNr;
}
int getEpNumber(std::string str) { int i, siz = str.size(), nr = 0; bool bracket = 0, number = 0;
  for (--siz; siz >= 0 && str[siz] != '.'; --siz);
  for (i = 0; i < siz; ++i) {
    if (str[i] >= '0' && str[i] <= '9' && bracket == 0) {
      if (number == 0) nr = 0, number = 1;
      nr *= 10, nr += str[i] - '0';
    } else {
      number = 0;
      if (str[i] == '(' || str[i] == '[' || str[i] == '{') bracket = 1;
      else if (str[i] == ')' || str[i] == ']' || str[i] == '}') { if (bracket == 0) nr = 0; bracket = 0; }
    }
  }

  return nr ? nr : -1;
}

// Provider: gogoanime.tv
int getLastGogoEp(std::string link) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, (Gogo_BaseAnimeLink + link).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 

    res = curl_easy_perform(curl); // get the ep number
    if (res == CURLE_OK) {
      size_t lepPos1 = response.find("ep_end");
      if (lepPos1 != std::string::npos) { lepPos1 += 6;
        size_t lepPos2 = response.find('>', lepPos1);
        if (lepPos2 != std::string::npos) {
          int i, lepNr = 0;
          for (i = lepPos1; i < lepPos2; ++i) if (response[i] - '0' >= 0 && response[i] - '0' <= 9) lepNr *= 10, lepNr += response[i] - '0';
          return lepNr;
        } else std::fprintf(stderr, "Error when trying to find the last episode string pos 2!\n");
      } else std::fprintf(stderr, "Error when trying to find the last episode string pos 1!\n");
    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return -1;
}
json searchGogo(std::string animeName) {
  CURL *curl;
  CURLcode res;

  json animeDetails;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    size_t i, j = animeName.size();
    for (i = 0; i < j; ++i) if (animeName[i] == ' ') animeName[i] = '+';

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (Gogo_BaseSearchLink + animeName).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      size_t rPos1 = response.find("<ul class=\"items\">") + 18;
      size_t rPos2 = response.find("</ul>", rPos1);
      j = 0;
      std::string str;
      
      while (rPos1 != std::string::npos) {
        str.clear(); rPos1 = response.find("/category/", rPos1); if (rPos1 == std::string::npos) break; rPos1 += 10;
        i = rPos1; while (response[i] != '"') str.push_back(response[i++]);
        animeDetails[j]["link"] = str;

        str.clear(); rPos1 = response.find("title=\"", i); if (rPos1 == std::string::npos) break; rPos1 += 7;
        i = rPos1; while (response[i] != '"') str.push_back(response[i++]);
        animeDetails[j++]["title"] = str;

        rPos1 = response.find("</li>", i); if (rPos1 == std::string::npos) break; rPos1 += 5;
        i = response.find("</li>", rPos1);
        if (rPos1 > i || i > rPos2) break;
      }

      return animeDetails;

    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return animeDetails;
}
std::string getGogoEpDownLink(std::string link, int id, long int &res_code) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, (Gogo_BaseLink + link + "-episode-" + std::to_string(id)).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 

    res = curl_easy_perform(curl); // get the ep number
    if (res == CURLE_OK) {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
      if (!((res_code == 200 || res_code == 201) && res != CURLE_ABORTED_BY_CALLBACK)) return std::string("Response code: " + std::to_string(res_code));
      res_code = 0;

      size_t downPos = response.find("<li class=\"dowloads\"><a href=\"");
      if (downPos != std::string::npos) { downPos += 30; link.erase();
        for (; response[downPos] != '\"'; ++downPos) link.push_back(response[downPos]);
        return link;
      } else std::fprintf(stderr, "Error when trying to find the download string pos!\n");
    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return "Error when trying to find the download link!";
}

// MAL
int getAnimeDetails(std::string animeId, bool &status, std::string &image_link) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, (std::string("Authorization: ") + MAL_ID).c_str());

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (MAL_BaseAPIAnimeLink + animeId + MAL_APIAnimeFields).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      json resp;
      resp = json::parse(response);
      
      int epNr = resp["num_episodes"].get<int>();
      status = std::strcmp(resp["status"].get<std::string>().c_str(), "finished_airing");
      image_link = resp["main_picture"]["medium"].get<std::string>();

      return epNr;

    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();

  return -1;
}
bool getAnimeStatus(std::string animeId) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, (std::string("Authorization: ") + MAL_ID).c_str());

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (MAL_BaseAPIAnimeLink + animeId + MAL_APIAnimeFields).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      json resp;
      resp = json::parse(response);
      
      return (std::strcmp(resp["status"].get<std::string>().c_str(), "finished_airing"));
    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();

  return 0;
}
json searchMAL(std::string animeName) {
  CURL *curl;
  CURLcode res;

  json animeDetails;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, (std::string("Authorization: ") + MAL_ID).c_str());

    std::string response; int i, siz = animeName.size();
    for (i = 0; i < siz; ++i) if (animeName[i] == ' ') animeName[i] = '_';
    curl_easy_setopt(curl, CURLOPT_URL, (MAL_BaseAPISearchLink + animeName).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      long res_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
      if (!((res_code == 200 || res_code == 201) && res != CURLE_ABORTED_BY_CALLBACK)) { std::fprintf(stderr, "Response code: %ld\n", res_code); return animeDetails; }

      json resp;
      resp = json::parse(response);

      siz = resp["data"].size();
      for (i = 0; i < siz; ++i) animeDetails[i]["link"] = resp["data"][i]["node"]["id"], animeDetails[i]["title"] = resp["data"][i]["node"]["title"];

      return animeDetails;

    } else std::fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();

  return animeDetails;
}
