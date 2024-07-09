/**
 * \file jpod.cpp
 * \brief Implements the main() function and serveral helper functions.
 */

#include<string>
#include<vector>
#include<iostream>
#include<stdexcept>
#include<functional>
#include<cstdlib>
#include<nxml.h>
#include"filter.h"
#include"episode.h"
#include"feed.h"

/**
 * \brief Print the help/usage message, then terminate
 */
void printHelp()
{
	std::cout
		<< "JPod - a primitive podcatcher" << std::endl
		<< std::endl
		<< "Usage: jpod [COMMAND]" << std::endl
		<< std::endl
		<< "Where COMMAND can be one of the following:" << std::endl
		<< "  help | --help | -h     Show this information." << std::endl
		<< "  list                   List the uids of all feeds." << std::endl
		<< "  info UID               Show information about the given feed." << std::endl
		<< "  episodes UID           List all the episodes (that get past the filter) of the given feed." << std::endl
		<< "  update [UID]           Update one or all feeds and download new episodes." << std::endl
		<< "                         If no UID is given, all feeds are updated." << std::endl
		<< std::endl
		<< "The feeds are obtained from the .jpodconf file in the current user's home" << std::endl
		<< "directory. If this file does not exist, the program will fail. You can create" << std::endl
		<< "an example configuration file by running make newconf as the user in question." << std::endl
		<< std::endl;
	exit(0);
}

/**
 * \brief Helper class for automatically freeing C-style objects when they go
 * out of scope
 * \details Used for libnxml objects.
 */
class Finalizer
{
private:
	std::function<void()> finalize;
public:
	/**
	 * \brief Constructs a Finalizer
	 * \param finalize Function that is called when this instance does out of
	 * scope.
	 */
	Finalizer(std::function<void()> finalize): finalize(finalize) {}

	/**
	 * \brief Destructor that calls the finalize() function.
	 */
	~Finalizer() {finalize();}
};

/**
 * \brief Parse the configuration file and extract a list of all feeds
 * \param configFile Name of the configuration file.
 * \return List of all the feeds.
 * \throws std::runtime_error If a problem occurs while parsing the config file.
 */
std::vector<Feed> readConfigFile(std::string configFile)
{
	std::filesystem::path homeDir(getenv("HOME"));

	nxml_t* xmlData;
	nxml_error_t rc;

	// Initlialize nxml library
	rc = nxml_new(&xmlData);
	if(rc != NXML_OK)
		throw std::runtime_error("Error initializing libnxml.");
	Finalizer f1([xmlData]{nxml_free(xmlData);});

	// Parse config file
	rc = nxml_parse_file(xmlData, configFile.data());
	if(rc != NXML_OK)
		throw std::runtime_error("Error reading config file " + configFile + ": " + nxml_strerror(xmlData, rc));

	// Get <podlist>...</podlist> root element
	nxml_data_t* xmlPodlist;
	nxml_root_element(xmlData, &xmlPodlist);
	if(xmlPodlist->type != NXML_TYPE_ELEMENT || std::string(xmlPodlist->value) != "podlist")
		throw std::runtime_error("Invalid config file. Root node is not <podlist>...</podlist>");

	// Go through <feed>...</feed> elements
	std::vector<Feed> feedList;
	nxml_data_t* xmlFeed = xmlPodlist->children;
	while(xmlFeed)
	{
		if(xmlFeed->type != NXML_TYPE_COMMENT)
		{
			if(xmlFeed->type != NXML_TYPE_ELEMENT || std::string(xmlFeed->value) != "feed")
				throw std::runtime_error("Invalid config file. Podlist contains something other than <feed>...</feed>");

			// Get the uid attribute
			nxml_attr_t* xmlUid;
			rc = nxml_find_attribute(xmlFeed, std::string("uid").data(), &xmlUid);
			if(rc != NXML_OK || xmlUid == NULL)
				throw std::runtime_error("Invalid feed in config file. Attribute uid is missing.");
			std::string uid(xmlUid->value);
			if(uid.empty() || uid.find_first_of(" \t\r\n") != std::string::npos)
				throw std::runtime_error("Invalid feed in config file. Attribute uid must not be empty or contain whitespaces.");

			// Get the uri attribute
			nxml_attr_t* xmlUri;
			rc = nxml_find_attribute(xmlFeed, std::string("uri").data(), &xmlUri);
			if(rc != NXML_OK || xmlUri == NULL)
				throw std::runtime_error("Invalid feed in config file. Attribute uri is missing in the feed with uid \"" + uid + "\".");
			std::string uri(xmlUri->value);
			if(uri.empty())
				throw std::runtime_error("Invalid feed in config file. Attribute uri is empty in the feed with uid \"" + uid + "\".");

			// Get the basedir
			nxml_attr_t* xmlBasedir;
			rc = nxml_find_attribute(xmlFeed, std::string("basedir").data(), &xmlBasedir);
			if(rc != NXML_OK || xmlBasedir == NULL)
				throw std::runtime_error("Invalid feed in config file. Attribute basedir is missing in the feed with uid \"" + uid + "\".");
			std::string basedir(xmlBasedir->value);

			// Get the filename pattern
			nxml_attr_t* xmlFilename;
			rc = nxml_find_attribute(xmlFeed, std::string("filename").data(), &xmlFilename);
			std::string filename = "%Y-%m-%d_%T";
			if(rc == NXML_OK && xmlFilename != NULL)
			{
				filename = xmlFilename->value;
				if(filename.empty())
					throw std::runtime_error("Invalid feed in config file. Attribute filename is invalid in the feed with uid \"" + uid + "\".");
			}

			// Get the filters
			std::vector<Filter> filterList;
			nxml_data_t* xmlFilter = xmlFeed->children;
			while(xmlFilter)
			{
				if(xmlFilter->type != NXML_TYPE_COMMENT)
				{
					if(xmlFilter->type != NXML_TYPE_ELEMENT || std::string(xmlFilter->value) != "filter")
						throw std::runtime_error("Invalid config file. <feed>...</feed> contains something other than <filter ... />");

					// Get the type attribute
					nxml_attr_t* xmlType;
					rc = nxml_find_attribute(xmlFilter, std::string("type").data(), &xmlType);
					if(rc != NXML_OK || xmlType == NULL)
						throw std::runtime_error("Invalid filter in feed with uid \"" + uid + "\". Attribute type is missing.");
					std::string strType(xmlType->value);
					FilterType type;
					if(strType == "include-if-match")
						type = FilterType::INCLUDE_IF_MATCH;
					else if(strType == "include-if-not-match")
						type = FilterType::INCLUDE_IF_NOT_MATCH;
					else if(strType == "exclude-if-match")
						type = FilterType::EXCLUDE_IF_MATCH;
					else if(strType == "exclude-if-not-match")
						type = FilterType::EXCLUDE_IF_NOT_MATCH;
					else
						throw std::runtime_error("Invalid filter in feed with uid \"" + uid + "\". Attribute type must be one of \"include-if-match\", \"include-if-not-match\", \"exclude-if-match\", \"exclude-if-not-match\".");

					// Get the regex attribute
					nxml_attr_t* xmlRegex;
					rc = nxml_find_attribute(xmlFilter, std::string("regex").data(), &xmlRegex);
					if(rc != NXML_OK || xmlRegex == NULL)
						throw std::runtime_error("Invalid filter in feed with uid \"" + uid + "\". Attribute regex is missing.");
					std::regex regex;
					try {regex = xmlRegex->value;}
					catch(std::regex_error& e) {throw std::runtime_error("Invalid filter in feed with uid \"" + uid + "\". Attribute regex is not a valid regular expression: " + e.what());}

					// Get the match attribute
					nxml_attr_t* xmlMatch;
					rc = nxml_find_attribute(xmlFilter, std::string("match").data(), &xmlMatch);
					if(rc != NXML_OK || xmlMatch == NULL)
						throw std::runtime_error("Invalid filter in feed with uid \"" + uid + "\". Attribute match is missing.");
					std::string match(xmlMatch->value);
						
					// Add filter to list
					filterList.push_back(Filter(type, regex, match));
				}
				xmlFilter = xmlFilter->next;
			}

			// Add Feed to list
			feedList.push_back(Feed(uid, uri, homeDir / basedir, filename, filterList));
		}
		xmlFeed = xmlFeed->next;
	}

	return feedList;
}

/**
 * \brief Searches for a feed by UID
 * \details If the feed cannot be found, a message is written to stderr and the
 * program terminates with exit code 1.
 * \param feedList List of all the feeds (obtained from readConfigFile()).
 * \param uid The unique identifier string of a podcast feed.
 * \return The Feed with the UID from feedList.
 */
Feed findFeed(std::vector<Feed> feedList, std::string uid)
{
	for(Feed feed : feedList)
		if(feed.getUid() == uid)
			return feed;
	std::cout << "No feed with UID \"" << uid << "\" exists. Use \"jpod list\" for a list f all UIDs." << std::endl;
	exit(1);
}

/**
 * \brief Main function
 * \param argc Number of command line arguments.
 * \param argv Array of command line arguments.
 * \return Returns 0 if successful.
 */
int main(int argc, char** argv)
{
	// Read command line parameters
	std::vector<std::string> args;
	for(int i = 1; i <= argc && argv[i] != NULL; i++)
		args.push_back(argv[i]);

	// Show help
	if(args.size() == 0 || args[0] == "help" || args[0] == "--help" || args[0] == "-h")
		printHelp();

	// Read the feed list from the configuration file
	std::vector<Feed> feedList;
	try
	{
		feedList = readConfigFile(std::string(getenv("HOME")) + "/.jpodconf");
	}
	catch(std::runtime_error& e) {std::cout << e.what() << std::endl; exit(1);}

	// List all uids
	if(args[0] == "list")
	{
		for(Feed f : feedList)
			std::cout << f.getUid() << " ";
		std::cout << std::endl;
		exit(0);
	}

	// Show info about a given feed
	if(args[0] == "info" || args[0] == "episodes")
	{
		// Check parameter
		if(args.size() < 2)
		{
			std::cout << "Missing UID of the feed. Use \"jpod help\" for more information." << std::endl;
			exit(1);
		}

		// Find feed
		Feed feed = findFeed(feedList, args[1]);
		// Update feed
		feed.update();

		// Show requested information
		if(args[0] == "info")
			std::cout
				<< "UID:\t" << feed.getUid() << std::endl
				<< "URI:\t" << feed.getUri() << std::endl
				<< "Download directory:\t" << feed.getBasePath().string() << std::endl
				<< "Filename pattern:\t" << feed.getFilenamePattern() << std::endl
				<< "Title:\t" << feed.getTitle() << std::endl
				<< "Description:\t" << feed.getDescription() << std::endl;
		else
			for(Episode ep : feed.getEpisodes())
				std::cout
					<< "Title:\t" << ep.getTitle() << std::endl
					<< "URI:\t" << ep.getUri() << std::endl
					<< "Published:\t" << std::put_time(ep.getPubDate(), "%c") << std::endl
					<< "Description:\t" << ep.getDescription() << std::endl
					<< std::endl;
		exit(0);
	}

	// Download missing episodes
	if(args[0] == "update")
	{
		// Only feeds that should be updated remain in feedList
		if(args.size() >= 2)
		{
			Feed feed = findFeed(feedList, args[1]);
			feedList = std::vector<Feed>(1, feed);
		}

		// Go over feeds
		for(Feed feed : feedList)
		{
			try
			{
				// Update feed
				feed.update();
				// Download new episodes
				feed.download();
			}
			catch(std::runtime_error& e)
			{
				// If one fails, continue with the next one
				std::cerr << "A problem ocurred when updating the feed with UID \"" << feed.getUid() << "\": " << e.what() << std::endl;
			}
		}
		exit(0);
	}

	std::cout << "Unknown command \"" << args[0] << "\". Use \"jpod help\" for more information." << std::endl;
	return 1;
}

