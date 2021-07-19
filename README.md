# Warzone OSM Mapmaker

This command line utility creates maps for the online strategy game [Warzone](https://www.warzone.com/) by using OpenStreetMap (OSM) data.

## Contents

* [Description](#description)
* [Getting Started](#getting-started)
    * [Pre-Requisites](#pre-requisites)
    * [Dependencies](#dependencies)
        * [Git and C++ Tools](#git-and-c++-tools)
        * [Libosmium](#libosmium)
        * [Osmium Tool](#osmium-tool)
    * [Installation]
* [Usage](#usage)
    * [Program Options](#program-options)
* [Built With](#built-with)
* [Authors](#authors)
* [Helpful Links](#helpful-links)

## Description

This tool is part of the Bachelor's Thesis **OSM Risk Maps** which was conducted at the [Institute For Formal Methods of Informatics](https://www.fmi.uni-stuttgart.de/) at the [University of Stuttgart](https://www.uni-stuttgart.de/). For more information about the thesis or this project, feel free to contact the initial author [Patrick Lindemann](https://github.com/PatrickLindemann).

## Getting Started

Currently, the installation and usage of the Mapmaker is supported on Unix systems only. This section provides an installation guide for Linux Ubuntu systems.

### Dependencies

The project depends on multiple third-party libraries which have to be installed prior to the project setup. Before you start installing packages, be sure to update your current packages by entering

```
sudo apt update
```

#### Git and C++ Tools

First, you will need to install the build essentials that allow you to build this C++ project. You can install them at once with this command:

```
sudo apt-get install -y -V \
  git \
  g++ \
  make \
  cmake \
  doxygen \
  graphviz \
  libboost-dev
```

#### Libosmium

The second step is installing [libosmium](https://osmcode.org/libosmium/) and all its needed dependencies that allow the program to work with OSM files. Like before, these can be installed at once with this command:

```
sudo apt-get install -y -V \
    libbz2-dev \
    libexpat1-dev \
    liblz4-dev \
    libproj-dev \
    libsparsehash-dev \
    libutfcpp-dev \
    zlib1g-dev \
```

#### Osmium-Tool (Optional)

If you want to work with the OSM data before using the mapmaker, e.g. for creating or merging extracts, [osmium-tool](https://osmcode.org/osmium-tool/) is a helpful command line utility by the same creators as libosium. You can install it with this command:

```
sudo apt-get install osmium-tool
```

Afterwards, you can explore the utility and its options by entering

```
osmium -h
```

More information and the documentation of the tool can be found in the [Osmium Tool Manual](https://osmcode.org/osmium-tool/manual.html).

### Installation

1. Clone this repository

```
git clone https://github.com/PatrickLindemann/warzone-osm-processor.git
```

2. Build the project

```
mkdir build
cd build
cmake ..
make
```

3. Test the program by entering
```
./warzone-osm-mapmaker -h
```

## Usage

If you are not familiar with the [OpenStreetMap](https://www.openstreetmap.org/) project or [Warzone](https://www.warzone.com/), we recommend that you find more about them in the [OpenStreetMap Wiki](https://wiki.openstreetmap.org/wiki/Main_Page) or [Warzone Wiki](https://www.warzone.com/wiki/Main_Page), repsectively.

This tool depends on real OpenStreetMap data in order to generate the playable maps. In detail, *administrative* [Boundary](https://wiki.openstreetmap.org/wiki/Key:boundary) areas with specified [admin_levels](https://wiki.openstreetmap.org/wiki/Key:admin_level) are extracted from an input file and put together into a [Warzone Map](https://www.warzone.com/wiki/Map). After the map was created, it can be uploaded by the user via the 

### Getting OSM data extracts

You can find and download an extract of your choice from [Geofabrik](https://download.geofabrik.de/), a server that provides regularily updated OSM data files for the whole planet. Please note that such files can reach sizes up to 58.5GB ([planet.osm.pbf](https://wiki.openstreetmap.org/wiki/Planet.osm)) depending on the scale of your extract.

### Creating Warzone maps

After you have downloaded the data extract, you should  https://wiki.openstreetmap.org/wiki/Tag:boundary%3Dadministrative#10_admin_level_values_for_specific_countries

```
warzone-osm-mapmaker <input-file>
```

To customize which boundaries will be used as [territories](https://www.warzone.com/wiki/Territories) and which as [bonus links](https://www.warzone.com/wiki/Bonus), the program provides two different parameters:

* `--territory--level <LEVEL>`
* `--bonus-level <LEVEL>`

As different countries have different identifiers for and subdivions of counties, you will need to determine the according admin_level for the country or continent that your OSM extract is from. You can find out more about country-specific admin levels [here](https://wiki.openstreetmap.org/wiki/Tag:boundary%3Dadministrative#10_admin_level_values_for_specific_countries).

For example, if you want to create a warzone map of [Germany](https://de.wikipedia.org/wiki/Liste_der_Landkreise_in_Deutschland), using `--territory-level 4` would yield a map with its federal state borders (Bundesländer), `--territory-level 6` would use the county borders (Landkreise). By combining the levels with `--territory-level 6 --bonus-level 4`, one would create a map with county borders as territories and federal state borders as bonus links.

For more command line options, please refer to the section [Program Options](#program-options).

### Uploading your map

In order to upload maps to the [Warzone Community Map Explorer](https://www.warzone.com/SinglePlayer/CommunityLevels), you will first need to create a [Warzone Account](https://www.warzone.com/SignUp2). Afterwards, you can upload the generated map svg [here](https://www.warzone.com/MultiPlayer?DesignMaps=1).

**Note**: Currently, Warzone provides no API that allows you to upload maps programmatically.

### Adding meta information

/todo

### Program Options

The program takes multiple options that allow you to fine-tune the generated map output:

| Parameter | Shorthand | Description | Type | Default | Annotation(s) |
|-----------|-----------|-------------|------|---------|---------------|
| --input || The input file path. Allowed file formats: .osm, .pbf | string | - ||
| --output | -o | The output file path. Allowed file formats: .svg | string | <BUILD_DIR>/../out/<INPUT_NAME>.svg ||
| --territory-level | -t | The admin_level of boundaries that will be used as territories. | int: [1; 12] | 6 ||
| --bonus-level | -b | The admin_level of boundaries that will be used as bonus links. If set to 0, no bonus links will be created. | int: [1; 12] | 0 ||
| --width || The output map width in pixels. If set to 0, the width will be determined automatically with the height. | int | 1000 ||
| --height || The output map height in pixels. If set to 0, the height will be determined automatically with the width. | int | 0 ||
| --epsilon || The minimum distance threshold between points for the Douglas-Peucker compression algorithm. If set to 0, no compression will be applied. | double | 0 ||
| --cache || Enables caching of already read input files. | flag | false | This should not be enabled when working with large files (> 5GB). |
| --verbose | -v | Enable verbose logging | flag | false ||

## Built with

- [**Boost**](https://www.boost.org/) - Free peer-reviewed portable C++ source libraries - [Github](https://github.com/boostorg/boost)
- [**Libosmium**](https://osmcode.org/libosmium/) - A fast and flexible C++ library for working with OpenStreetMap data - [Github](https://github.com/osmcode/libosmium)

## Authors

- **Patrick Lindemann** - Initial work - [Github](https://github.com/PatrickLindemann)

## Helpful Links

/todo

* https://wiki.openstreetmap.org/wiki/Tag:boundary%3Dadministrative#10_admin_level_values_for_specific_countries