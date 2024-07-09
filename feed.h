/**
 * \file feed.h
 * \brief Defines the Feed class
 */

#ifndef FEED_H
#define FEED_H

#include<string>
#include<vector>
#include<filesystem>
#include"episode.h"
#include"filter.h"

/**
 * \brief Represents a podcast feed
 */
class Feed
{
private:
	std::string uid, uri, filenamePattern;
	std::filesystem::path basePath;
	std::string title, description;
	std::vector<Episode> episodes;
	std::vector<Filter> filters;
	bool updated;

	static std::string cleanupFilename(std::string filename);
public:
	/**
	 * \brief Constructs a Feed object
	 * \details No internet connectivity is needed at this point. The URI is
	 * not accessed until update() is called.
	 * \param uid Unique ID for this podcast feed. This string is mainly used
	 * to identify the podcast from the command line.
	 * \param uri The URI of the RSS feed.
	 * \param basePath Path to the directory where downloaded episodes should
	 * be placed. If this path does not exist, it is created.
	 * \param filenamePattern Used to create filenames for downloaded episodes.
	 * See Episode#fillPlaceholders() for details.
	 * \param filters A list of filters that are applied to each episode in
	 * this feed.
	 * \throws std::runtime_error If the base path could not be accessed or
	 * created.
	 */
	Feed(std::string uid, std::string uri, std::filesystem::path basePath, std::string filenamePattern, std::vector<Filter> filters = std::vector<Filter>());

	/**
	 * \brief Returns the feed's unique id
	 * \return The unique identifier string for this feed.
	 */
	std::string getUid() const {return uid;}

	/**
	 * \brief Returns the feed's URI
	 * \return The URI of the RSS feed.
	 */
	std::string getUri() const {return uri;}

	/**
	 * \brief Returns the feed's base path
	 * \return The base path where downloaded episodes are stored.
	 */
	std::filesystem::path getBasePath() const {return basePath;}

	/**
	 * \brief Returns the feed's filename pattern
	 * \return The pattern used to generate filenames for downloaded episodes.
	 */
	std::string getFilenamePattern() const {return filenamePattern;}

	/**
	 * \brief Updates the feed from the URI
	 * \details This retrieves the feed's title, description and episode list.
	 * If obtaining the information for an episode fails (i.e. the `<item>...
	 * </item>` section contains invalid data) but the rest of the RSS feed is
	 * still readable, the episode is ignored and the method continues with the
	 * next one.
	 * \throws std::runtime_error If an error occurs while downloading or
	 * parsing the RSS feed.
	 */
	void update();

	/**
	 * \brief Returns the feed's title
	 * \return The tite of the feed.
	 * \throws std::runtime_error If update() has not been called before.
	 */
	std::string getTitle() const;

	/**
	 * \brief Returns the feed's description
	 * \return The description of the feed.
	 * \throws std::runtime_error If update() has not been called before.
	 */
	std::string getDescription() const;

	/**
	 * \brief Returns the feed's episode list
	 * \return The episode list of the feed.
	 * \throws std::runtime_error If update() has not been called before.
	 */
	const std::vector<Episode>& getEpisodes() const;

	/**
	 * \brief Downloads all missing episodes
	 * \details To determine whether an episode has already been downloaded,
	 * the method looks for a file with the same name (but not necessarily file
	 * extension). If this file exists, regardless of its contents, that
	 * episode is not (re)downloaded.
	 * If the download of an episode fails, an error is printed to stderr but
	 * the method continues with the next episode.
	 * \throws std::runtime_error If update() has not been called before.
	 */
	void download();
};

#endif //FEED_H
