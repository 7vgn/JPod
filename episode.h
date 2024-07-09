/**
 * \file episode.h
 * \brief Defines the Episode class
 */

#ifndef EPISODE_H
#define EPISODE_H

#include<string>
#include<filesystem>
#include<ctime>
#include<mrss.h>

class Feed;

/**
 * \brief Represents a single episode within a podcast feed
 */
class Episode
{
private:
	Feed*		feed;
	std::string	title, description, uri;
	std::tm		pubDate;

	static std::tm parseTime(std::string timeStr);
public:
	/**
	 * \brief Creates an Episode instance from an RSS item
	 * \param feed Pointer to the Feed that this episode belongs to.
	 * \param item The RSS `<item>...</item>` containing the information for the
	 * episode.
	 * \throws std::runtime_error If the publication date could not be parsed
	 * or if the episode has no enclosed URI. Some RSS feed items come only
	 * with text which means for the purposes of downloading podcasts, it
	 * should be ignored.
	 */
	Episode(Feed* feed, mrss_item_t* item);

	/**
	 * \brief Returns the title of the episode
	 * \return The episode title.
	 */
	std::string getTitle() const {return title;}

	/**
	 * \brief Returns the description of the episode
	 * \return The episode description.
	 */
	std::string getDescription() const {return description;}

	/**
	 * \brief Returns the URI of the episode
	 * \return The episode URI.
	 */
	std::string getUri() const {return uri;}

	/**
	 * \brief Returns the publication date of the episode
	 * \return The episode's publication date.
	 */
	const std::tm* getPubDate() const {return &pubDate;}

	/**
	 * \brief Fills the placeholders in a string with the episode's metdata
	 * \param pattern A string that may contain certain placeholders:
	 * \%P is replaced with the title of the feed.
	 * \%C is replaced with the description of the feed.
	 * \%T is replaced with the title of the episode.
	 * \%D is replaced with the description of the episode.
	 * \%Y, \%y, \%m, \%b, \%B, \%W, \%j, \%d, \%e, \%a, \%A, \%w, \%u, \%H,
	 * \%I, \%M, \%S, and \%p are replaced with the corresponding part of the
	 * episode's publication date. See std::put_time() for details.
	 * \%\% is replaced with the percent sign.
	 * \return The pattern with all placeholders replaced.
	 */
	std::string fillPlaceholders(std::string pattern) const;

	/**
	 * \brief Downloads an the episode
	 * \param filename The name (including path) of the file where the
	 * downloaded data should be written.
	 * \throws std::runtime_error If anything at all goes wrong. This includes
	 * failure to download and failure to create or write to the file.
	 */
	void download(std::filesystem::path filename) const;
};

#endif //EPISODE_H
