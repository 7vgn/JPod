/**
 * \file filter.h
 * \brief Defines the Filter class and related enumeration types.
 */

#ifndef FILTER_H
#define FILTER_H

#include<string>
#include<regex>
#include"episode.h"

/**
 * \brief Determines the type of a filter
 */
enum class FilterType
{
	/// Exclude the episode if the regular expression matches
	EXCLUDE_IF_MATCH,
	/// Exclude the episode if the regular expression doesn't match
	EXCLUDE_IF_NOT_MATCH,
	/// Include the episode if the regular expression matches
	INCLUDE_IF_MATCH,
	/// Include the episode if the regular expression doesn't match
	INCLUDE_IF_NOT_MATCH
};

/**
 * \brief The result of applying a Filter to an episode
 */
enum class FilterResult
{
	/// The episode should definitely be included (ignore remaining filters)
	EXCLUDE,
	/// The episode should definitely be excluded (ignore remaining filters)
	INCLUDE,
	/// This filter cannot decide whether to include or exclude the episode (look at remaining filters for answer)
	INCONCLUSIVE
};

/**
 * \brief Filters decide whether an episode is to be downloaded or ignored
 * \details A Filter consists of a regular expression that gets matched against
 * a string which is constructed from the metadata of an episode.
 */
class Filter
{
private:
	FilterType type;
	std::regex regex;
	std::string match;
public:
	/**
	 * \brief Creates a filter
	 * \param type The type of the filter. Determines what action is to take
	 * place if (or if not) the filter matches.
	 * \param regex A regular expression conforming to the std::regex format.
	 * See https://cplusplus.com/reference/regex/ECMAScript/ for details.
	 * \param match The string that is matched against the regular expression.
	 * May contain certain placeholders that get replaced with the
	 * corresponding metadata entries of the episode. See
	 * Episode#fillPlaceholders() for details.
	 */
	Filter(FilterType type, std::regex regex, std::string match);

	/**
	 * \brief Applies the filter to an episode
	 * \param episode Reference to an episode. It is used to fill the
	 * placeholders in the string that is matched against the regular
	 * expression.
	 * \return Returns whether - according to this filter - the episode
	 * should definitely be included, definitely be excluded, or neither.
	 */
	FilterResult apply(const Episode& episode);
};

#endif //FILTER_H
