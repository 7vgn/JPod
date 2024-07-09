/**
 * \file filter.cpp
 * \brief Implementation for filter.h
 */

#include"filter.h"

Filter::Filter(FilterType type, std::regex regex, std::string match)
: type(type), regex(regex), match(match)
{
}

FilterResult Filter::apply(const Episode& episode)
{
	bool matched = regex_match(episode.fillPlaceholders(match), regex);
	if((matched && type == FilterType::EXCLUDE_IF_MATCH) || (!matched && type == FilterType::EXCLUDE_IF_NOT_MATCH))
		return FilterResult::EXCLUDE;
	else if((matched && type == FilterType::INCLUDE_IF_MATCH) || (!matched && type == FilterType::INCLUDE_IF_NOT_MATCH))
		return FilterResult::INCLUDE;
	else
		return FilterResult::INCONCLUSIVE;
}

