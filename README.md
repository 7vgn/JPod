# JPod - A Primitive Podcatcher
## What is this?
JPod is a very simple podcast downloader written in C++. It is not intended to
replace sophisticated podcast management software but rather to quickly archive
podcasts with user-friendly filenames. It requires no database on its back-end
and has very few dependencies.

While JPod should compile on most platforms with little or no modifications, it
is mainly targeted at Linux. The installation script is Linux-only and the main
use is to call it via cronjob.

JPod is configured with a single, easy to understand XML file containing a list
of podcast URIs, where to download them to, and how to name downloaded files.
It is also possible to filter podcast episodes by certain keywords (for example
to exclude teaser episodes of partially free/paid podcasts).

## Building and Installing
### Dependencies
JPod requires the [curl](https://curl.se/),
[mRSS](https://github.com/bakulf/libmrss), and
[nXML](https://github.com/bakulf/libnxml) libraries, as well as
[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/).
You will also need a somewhat recent GCC to compile the program.
On Debian-based systems (Ubuntu, Mint etc.), you can install everything via

```
sudo apt-get install libcurl4 libmrss0-dev pkg-config build-essential
```
(libmrss will install libnxml as a dependency)

### Download JPod and Build it
```
git clone https://github.com/7vgn/JPod.git
cd JPod
make
```
If you wish, and have doxygen installed, you can also create the code
documentation with

```
make doc
```

### Installation
To install JPod, use

```
sudo make install
```
This copies the binary into /opt/jpod/bin.

To uninstall JPod, run

```
sudo make uninstall
```

## The Configuration File
JPod comes with a template for the configuration file. Running

```
make newconf
```
will place a copy into the home directory of the current user. JPod will
always look for the configuration file at ~/.jpodconf

The configuration file is self-explanatory and contains some example podcasts
as well as a demonstration of how to use filters. Each podcast feed needs a
unique identifier string (chosen by you), the URI of the RSS feed, a directory
where to place the downloaded episodes, and a pattern for creating episode
filenames.

## Using JPod
If you run JPod on the command line without parameters or with --help, it will
show a list of all possible arguments. The most important ones are

```
jpod update <unique identifier string of the podcast>
```
to update a single podcast and

```
jpod update
```
to update all podcasts.

## Automating Podcast Downloads
To automate podcast downloading, simply add call JPod to your crontab. Type

```
crontab -e
```
add the line

```
45 1 * * * /opt/jpod/bin/jpod update
```
then save and exit. This will run JPod every night at a quarter to two.
