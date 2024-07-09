/**
 * \file episode.cpp
 * \brief Implementation for episode.h
 */

#include<stdexcept>
#include<sstream>
#include<fstream>
#include<iomanip>
#include<curl/curl.h>
#include"feed.h"
#include"episode.h"

Episode::Episode(Feed* feed, mrss_item_t* item)
: feed(feed)
{
	title = item->title;
	description = item->description;
	if(!item->enclosure_url)
		throw std::runtime_error("Episode has no enclosed url, should be ignored");
	uri = item->enclosure_url;
	pubDate = parseTime(item->pubDate);
}

std::string Episode::fillPlaceholders(std::string pattern) const
{
	std::string result;
	for(int i = 0; i < pattern.length(); i++)
	{
		if(pattern[i] == '%')
		{
			if(i + 1 == pattern.length() || pattern[i + 1] == '%')
				result += '%';
			else if(pattern[i + 1] == 'P')
				result += feed->getTitle();
			else if(pattern[i + 1] == 'C')
				result += feed->getDescription();
			else if(pattern[i + 1] == 'T')
				result += getTitle();
			else if(pattern[i + 1] == 'D')
				result += getDescription();
			else if(std::string("YymbBWjdeaAwuHIMSp").find(pattern[i + 1]) != std::string::npos)
			{
				std::ostringstream oss;
				oss << std::put_time(getPubDate(), pattern.substr(i, 2).c_str());
				result += oss.str();
			}
			i++;
		}
		else
			result += pattern[i];
	}
	return result;
}

// Callback function for CURL to write data
static size_t curlWrite(void* ptr, size_t size, size_t nmemb, std::string* data)
{
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

void Episode::download(std::filesystem::path filename) const
{
	std::string responseData;
	long responseCode;

	// Initialise CURL
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(!curl)
	{
		curl_global_cleanup();
		throw std::runtime_error("Unable to initialize CURL");
	}

	// Perform HTTP GET request
	curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/4");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		throw std::runtime_error("Unable to connect to server");
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	if(responseCode != 200)
	{
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		throw std::runtime_error("Unable to download the episode from \"" + getUri() + "\", got response code " + std::to_string(responseCode));
	}
	char* ct = NULL;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
	std::string contentType(ct);

	// CURL clean up
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	// Try to determine file extension
	if(contentType.compare("audio/mpeg") == 0) filename += ".mp3";
	else if(contentType.compare("audio/mp4") == 0) filename += ".mp4";
	else if(contentType.compare("audio/ogg") == 0) filename += ".ogg";
	else if(contentType.compare("audio/wav") == 0) filename += ".wav";

	// Write data into file
	std::ofstream ofs(filename.c_str(), std::ios::binary);
	if(!ofs.is_open())
		throw std::runtime_error("Unable to store episode in \"" + filename.string() + "\".");
	ofs << responseData;
	ofs.close();
}

std::tm Episode::parseTime(std::string timeStr)
{
	std::tm tm;
	std::stringstream ss(timeStr);
	ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
	if(ss.fail())
		throw std::runtime_error("Podcast episode has invalid publication date");
	return tm;
}

