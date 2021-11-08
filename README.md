# Warzone OSM Mapmaker

![](https://github.com/PatrickLindemann/warzone-osm-mapmaker/blob/master/doc/mapmaker.svg?raw=true)

This command line utility creates maps for the online strategy game [Warzone](https://www.warzone.com/) by using OpenStreetMap (OSM) data.

This tool is part of the Bachelor's Thesis **OSM Risk Maps** which was conducted at the [Institute For Formal Methods of Informatics](https://www.fmi.uni-stuttgart.de/) at the [University of Stuttgart](https://www.uni-stuttgart.de/). For more information about the thesis or this project, feel free to contact the initial author [Patrick Lindemann](https://github.com/PatrickLindemann).

## Contents

* [Map Creation](#map-creation)
    * [Downloading OSM Data](#downloading-osm-data)
    * [Preparing The Extract (Optional)](#preparing-the-extract-optional)
    * [Inspecting The Extract](#inspecting-the-extract)
    * [Creating The Map](#creating-the-map)
    * [Tips for Creators](#tips-for-creators)
* [Map Upload](#map-upload)
    * [Uploading to Warzone](#uploading-to-warzone)
    * [API Setup](#api-setup)
    * [Adding Metadata](#adding-metadata)
* [Building the Project (Ubuntu)](#building-the-project-ubuntu)
    * [Pre-Requisites](#pre-requisites)
    * [Installation](#installation)
* [Built With](#built-with)
* [Authors](#authors)
* [Helpful Links](#helpful-links)

## Map Creation

If you are not familiar with the [OpenStreetMap](https://www.openstreetmap.org/) project or [Warzone](https://www.warzone.com/), we recommend that you find more about them in the [OpenStreetMap Wiki](https://wiki.openstreetmap.org/wiki/Main_Page) or [Warzone Wiki](https://www.warzone.com/wiki/Main_Page), repsectively.

This tool depends on real OpenStreetMap data in order to generate the playable maps. In detail, *administrative* [boundary areas](https://wiki.openstreetmap.org/wiki/Key:boundary) with specified [admin_levels](https://wiki.openstreetmap.org/wiki/Key:admin_level) are extracted from an input file and put together into a [Warzone Map](https://www.warzone.com/wiki/Map). The meaning of administrative levels differs between countries, a mapping can be found in this [table](https://wiki.openstreetmap.org/wiki/Tag:boundary%3Dadministrative#10_admin_level_values_for_specific_countries).

You can control which boundaries are extracted as [territories](https://www.warzone.com/wiki/Territories) and [bonuses](https://www.warzone.com/wiki/Bonus) (and super-bonuses) with the two parameters

* `--territory-level <level>`
* `--bonus-levels <level> ... <level>`

For example, if you want to create a warzone map of [Germany](https://de.wikipedia.org/wiki/Liste_der_Landkreise_in_Deutschland), using `--territory-level 4` would yield a map with its federal state borders (*Bundesländer*), `--territory-level 6` would use the county borders (*Landkreise*). By combining the levels with `--territory-level 6 --bonus-levels 4`, you would create a map with county borders as territories and federal state borders as bonuses.

### Downloading OSM data

You can find and download an extract of your choice from [Geofabrik](https://download.geofabrik.de/), a server that provides regularily updated OSM data files for the whole planet. Please note that such files can reach sizes up to 58.5GB ([planet.osm.pbf](https://wiki.openstreetmap.org/wiki/Planet.osm)), depending on the scale of your extract, which is why we recommend that you download the extracts compressed with the [Protocol Buffer Format](https://wiki.openstreetmap.org/wiki/PBF_Format) (*.pbf*).

If you just want to test the tool, you can use the mapmaker with the included `data/isle-of-man.osm.pbf` extract.

### Preparing the extract (Optional)

We are only interested in the administrative boundaries of OpenStreetMap to create our maps. Yet, map extracts contain lots of other information such as roads, buildings, points of interest, etc. This data is automatically skipped by the mapmaker, but larger files take longer to read, which can be tedious if you adjust the parameters for your map multiple times.

If you work with larger map extracts (> 10MB in compressed size), we strongly recommend to prepare the data by pre-filtering the boundaries of your downloaded map extract. This greatly reduces the file size of your data; for example, applying the pre-filter to a (compressed) map extract of Germany with a file size of 3.56GB reduces its file size to only 33MB.

To prepare your map extract, use the following command:

```
./warzone-osm-mapmaker prepare <input-file> [parameters]
```

This will create a pre-filtered file with the naming schema `<input-file>-prepared.osm.pbf`. Afterwards, you can use this file for the next map creation steps.

#### Parameters

The prepare command accepts the following parameters:

| Parameter | Short | Description | Type | Default |
|-----------|-------|-------------|------|---------|
| --outdir | -o | The output folder for the pre-filtered boundary file. | string | ./data/ |
| --help | -h | Show the help message. | flag ||


### Inspecting the extract

After you have downloaded (and pre-filtered) the map extract, you should inspect it by entering

```
./warzone-osm-mapmaker checkout <path/to/file.osm.pbf>
```

This will display a table of useful information as well as level level distribution for the map extract. You can use this table to decide which levels should be used as territory and bonus levels.

### Creating the map

In this step, we can finally create our map by entering

```
./warzone-osm-mapmaker create <path/to/file.osm.pbf> [parameters]
```

The mapmaker creates two outputs: A `svg` file that contains the geometry of the territories and bonuses, and a `.json` file with the metadata, which consists of

* The name, center point and neighbors for each territory and
* The name, color, army count and subareas for each bonus.

In order to play your map in Warzone, you will need to upload the map, which will be covered in the next section.

#### Parameters

Usually, you want to customize your generated map. For this, we offer a list of additional parameters that you can specify to alter the results:

| Parameter | Short | Description | Type | Default |
|-----------|-------|-------------|------|---------|
| --outdir | -o | The output folder for the generated map files. | string | ./out/ |
| --territory-level | -t | The admin_level of areas that will be used as territories. If not specified, the boundaries with the most common admin_level will be used as territores. | int: [1; 12] ||
| --bonus-levels | -b | The admin_levels of areas that will be used as bonuses. Bonus levels need to be lower than the territory level. If multiple levels are specified, super bonuses will be created. If no levels are specified, no bonuses will be created. | int: [1; 12] ||
| --width || The output map width in pixels. If set to 0, the width will be determined automatically with the height. | int | 1000 |
| --height || The output map height in pixels. If set to 0, the height will be determined automatically with the width. | int | 0 |
| --compression-tolerance | -c | The minimum distance tolerance for the compression algorithm. If set to 0, no compression will be applied. | [0; 1] | 0 |
| --filter-tolerance | -f | The surface area tolerance to filter areas that are too small. The value 0.25 means that all areas with a size of less 25% of the map will be removed. If set to 0, no filter will be applied. | [0; 1] | 0 |
| --verbose | -v | Enable verbose logging. | flag ||
| --help | -h | Show the help message. | flag ||

### Tips for Creators
* The width and height of your map should not exceed 2500x2500 pixels, as Warzone does not accept larger map sizes.
* Your generated map `.svg` should not exceed 2.5MB, as Warzone does not accept larger file sizes. You can reduce the map size by applying a greater compression tolerance.
* Currently, creating super bonuses is not possible through the Warzone API. If you add more than one bonus level, you need to add the super bonus metadata manually through the Warzone Mapmaker.

## Map Upload

After you've generated your map, you can use Warzone to playtest it.

In order to upload maps to the [Warzone Community Map Explorer](https://www.warzone.com/SinglePlayer/CommunityLevels), you will first need to create a [Warzone Account](https://www.warzone.com/SignUp2). Afterwards, you can upload the generated map svg [here](https://www.warzone.com/MultiPlayer?DesignMaps=1).

### Setup

To add the metadata that is generated with your map, we use the Warzone [Set Map Details API](https://www.warzone.com/wiki/Set_map_details_API). Before you can use the upload function, you first need to create a configuration file (named `config.json`), which you can do with the following command:

```
./warzone-osm-mapmaker setup [parameters]
```

During the setup, you will be asked to enter your Warzone user e-Mail and your personal API access token, which is needed to authenticate you in the upload process. You can retrieve your personal token via the [Get Token API](https://www.warzone.com/API/GetAPIToken), which can be accessed directly with [this link](https://www.warzone.com/API/GetAPIToken) after you've logged in. The response will look like this:

```
{
  "APIToken" : "QPxpV5bRa$bBCYVOs3PF#MKgApbWISKW#kR0p8qY",
  "information" : "[...]"
}
```

Now, you can copy the value of the `APIToken` field (without the quotation marks) and pass it to the application when asked. After you finish the setup, your `config.json` should look like this:
```
{
   "email": "email@example.com",
   "api-token": "QPxpV5bRa$bBCYVOs3PF#MKgApbWISKW#kR0p8qY"
}
```

**Important**: Your personal API token is used along with your e-mail address to authenticate your user with Warzone and act on your behalf. You should ***never*** reveal this token to other players - treat it as you would treat your password. If you ever suspect that someone knows your API token, you should immediately change your Warzone account password, which will cause your token to re-randomize and stop the old one from working.

#### Parameters

The setup command accepts the following parameters:

| Parameter | Short | Description | Type | Default |
|-----------|-------|-------------|------|---------|
| --outdir | -o | The output folder for the generated config.json. | string | ./ |
| --email | -e | The Warzone user E-Mail address. | string ||
| --token | -t | The Warzone user API token. | string ||
| --help | -h | Show the help message. | flag ||

### Uploading the Map

As of November 2021, Warzone offers no API endpoint to upload a generated map. Therefore, you need to upload your map manually.

### Uploading the metadata


## Building the Project (Ubuntu)

This section provides an installation guide for Linux Ubuntu systems.

### Pre-Requisites

The project depends on multiple third-party libraries which have to be installed prior to the project setup. Before you start installing packages, be sure to update your current packages by entering

```
sudo apt update
```

#### Git and C++ Tools

You will need to install the build essentials that allow you to build this C++ project. You can install them at once with this command:

```
sudo apt-get install -y -V \
  git \
  g++ \
  make \
  cmake \
  doxygen \
  graphviz \
  libboost-dev \
  libboost-filesystem-dev
```

All other dependencies will be installed automatically by the super build during the installation.

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

If the installation was sucessful, a help message with the available commands will appear.

## Building the Project (Windows)

This section provides an installation guide for 64-Bit Windows systems.

### Pre-Requisites

#### Git

#### MinGW

#### Boost

### Installation

## Built with

- [**Boost**](https://www.boost.org) - Free peer-reviewed portable C++ source libraries - [Github](https://github.com/boostorg/boost)
- [**Libosmium**](https://osmcode.org/libosmium) - A fast and flexible C++ library for working with OpenStreetMap data - [Github](https://github.com/osmcode/libosmium)
- [**nlohmann/json**](https://json.nlohmann.me/) - A modern JSON library for C++ - [Github](https://github.com/nlohmann/json)

## Authors

- **Patrick Lindemann** - Initial work - [Github](https://github.com/PatrickLindemann)