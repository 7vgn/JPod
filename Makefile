#------------------------------------------------------------------------------
# Main targets

all: jpod
.PHONY: clean install uninstall newconf doc

#------------------------------------------------------------------------------
# Generate jpod binary

# The libraries curl, mrss, libnxml, pkg-config must be installed, e.g. via
#  sudo apt-get install libcurl4 libmrss0-dev pkg-config
# (libmrss will install libnxml as a dependency)

# Compile and link flags
GCCFLAGS = -std=gnu++17 -O3

INCLUDES = $(shell pkg-config --cflags libcurl mrss)
LDFLAGS = $(shell pkg-config --libs libcurl mrss)

OBJS = jpod.o feed.o episode.o filter.o

# Link everything together
jpod: $(OBJS)
	g++ $(GCCFLAGS) -o jpod $(OBJS) $(LDFLAGS)

# Read dependency information
-include $(OBJS:.o=.d)

# Compile sources and generate dependency information
%.o: %.cpp
	g++ -c $(GCCFLAGS) $(INCLUDES) $*.cpp -o $*.o
	g++ -MM $(GCCFLAGS) $(INCLUDES) $*.cpp > $*.d

#------------------------------------------------------------------------------
# Generate doxygen documentation
doc:
	doxygen

#------------------------------------------------------------------------------
# Cleanup

clean:
	rm -rf *.o *.d jpod doc

#------------------------------------------------------------------------------
# Install and uninstall (both need root)

install: jpod
# Create program directory and copy binary there
	mkdir -p /opt/jpod/bin
	cp jpod /opt/jpod/bin/.
# Set ownership and permissions for all directories and files
	chown -R root:root /opt/jpod
	chmod -R 0755 /opt/jpod

uninstall:
# Remove directories
	rm -rf /opt/jpod

newconf:
# Creates a default configuration file in the current user's home directory. 
# Therefore, unlike install, this should NOT be run as root (unless root wants podcasts)
	cp jpodconf.default ~/.jpodconf

