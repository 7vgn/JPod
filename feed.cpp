/**
 * \file feed.cpp
 * \brief Implementation for feed.h
 */

#include<stdexcept>
#include<iostream>
#include<algorithm>
#include<mrss.h>
#include"feed.h"

Feed::Feed(std::string uid, std::string uri, std::filesystem::path basePath, std::string filenamePattern, std::vector<Filter> filters)
: uid(uid), uri(uri), basePath(basePath), filenamePattern(filenamePattern), filters(filters), updated(false)
{
	// Make sure basePath exists and is accessible
	if(!std::filesystem::exists(basePath))
	{
		try
		{
			std::filesystem::create_directories(basePath);
		}
		catch(std::filesystem::filesystem_error& e)
		{
			throw std::runtime_error(std::string("Base path for RSS feed does not exist and could not be created: ") + basePath.string());
		}
	}
	else if(!std::filesystem::is_directory(basePath))
		throw std::runtime_error(std::string("Base path for RSS feed is not a directory: ") + basePath.string());
}

void Feed::update()
{
	// Retrieve feed
	mrss_t* mrss;
	mrss_error_t err = mrss_parse_url(&uri[0], &mrss);
	if(err != MRSS_OK)
		throw std::runtime_error(std::string("Error parsing podcast RSS feed: ") + mrss_strerror(err));

	// Extract title & description
	title = mrss->title;
	description = mrss->description;

	// Get Episodes
	episodes.clear();
	mrss_item_t* item = mrss->item;
	while(item)
	{
		try
		{
			// Create an Episode object
			Episode episode(this, item);

			// Include this episode unless it gets filtered out
			FilterResult filterResult = FilterResult::INCONCLUSIVE;
			for(Filter filter : filters)
			{
				filterResult = filter.apply(episode);
				if(filterResult != FilterResult::INCONCLUSIVE)
					break;
			}
			if(filterResult != FilterResult::EXCLUDE) // Include by default
				episodes.push_back(episode);
		}
		catch(std::runtime_error& e) {} // If an error occurs, ignore this episode and continue with the next one. 

		// Move on to next episode
		item = item->next;
	}

	// Free resources
	mrss_free(mrss);

	updated = true;
}

std::string Feed::getTitle() const
{
	if(!updated)
		throw std::runtime_error("Feed must be updated before its title is available");
	return title;
}

std::string Feed::getDescription() const
{
	if(!updated)
		throw std::runtime_error("Feed must be updated before its description is available");
	return description;
}

const std::vector<Episode>& Feed::getEpisodes() const
{
	if(!updated)
		throw std::runtime_error("Feed must be updated before its episode list is available");
	return episodes;
}

void Feed::download()
{
	if(!updated)
		throw std::runtime_error("Feed must be updated before its episode list is available");
	for(const Episode& ep : getEpisodes())
	{
		// Create a filename for the episode
		std::string filename = cleanupFilename(ep.fillPlaceholders(filenamePattern));

		// Find out if the episode is already downloaded (ignore file extension since we don't know that without downloading)
		bool found = false;
		for(std::filesystem::directory_iterator iter(basePath); iter != std::filesystem::directory_iterator(); iter++)
		{
			if(std::filesystem::is_regular_file(*iter) && iter->path().stem() == filename)
				found = true;
		}
		if(found) continue;

		// Download the episode
		std::filesystem::path episodePath = basePath;
		episodePath.append(filename);
		try
		{
			ep.download(episodePath);
		}
		catch(std::runtime_error& e)
		{
			std::cerr << "The episode \"" << ep.getTitle() << "\" from the feed \"" << getTitle() << "\" could not be downloaded: " << e.what() << std::endl;
		}
	}
}

std::string Feed::cleanupFilename(std::string filename)
{
	// Remove all characters that might be problematic in a filename
	std::string result;
	std::remove_copy_if(filename.begin(), filename.end(), std::back_inserter(result), [](char c) {return std::string("!#$^&=+*{}:;\"'<>?|/\\").find(c) != std::string::npos;});

	// Cut filename to 250 characters maximum (255 is the limit, leave some space for extension)
	if(result.length() > 250)
		result = result.substr(0, 250);

	return result;
}

