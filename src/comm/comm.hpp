#pragma once

#ifndef _Comm_H
#define _Comm_H

#include <curl/curl.h>

#include "./../json/jFunc.hpp"

#include <cstdio>
#include <cstring>

#include <filesystem>
#include <string>


#define MAL_ID "6714cf62c9e005774888501e8a417384"
#define MAL_BaseAnimeLink  "https://myanimelist.net/anime/"
#define MAL_BaseAPIAnimeLink "https://api.myanimelist.net/v0.2/anime/"
#define MAL_BaseAPISearchLink "https://api.myanimelist.net/v0.2/anime?q="
#define MAL_APIAnimeFields "?fields=main_picture,status,num_episodes,pictures"

#define Gogo_BaseLink "https://gogoanime.tv/"
#define Gogo_BaseAnimeLink "https://gogoanime.tv/category/"
#define Gogo_BaseSearchLink "https://gogoanime.tv/search.html?keyword="

size_t curlToString(void *contents, size_t size, size_t nmemb, std::string *response);
size_t curlToFile(void *contents, size_t size, size_t nmemb, void *fp);
void urlImageToDisk(std::string link, std::string path);
std::string truncateDirectoryName(std::string str);
int getLastDownEp(std::string path);
int getEpNumber(std::string str);

// Provider: gogoanime.tv
int getLastGogoEp(std::string link);
json searchGogo(std::string animeName);
std::string getGogoEpDownLink(std::string link, int id, long int &res_code);

// MAL
int getAnimeDetails(std::string animeId, bool &status, std::string &image_link);
bool getAnimeStatus(std::string animeId);
json searchMAL(std::string animeName);

#endif //_Comm_H
