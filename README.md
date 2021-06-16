# Warzone OSM Processor

This command line utility pre-processes OSM data by filtering areas, calculating and building geometries and serializing the results into [Apache Parquet](https://github.com/apache/parquet-format) files for use in the [Warzone OSM Mapmaker](https://github.com/PatrickLindemann/warzone-osm-map-maker/).

## Contents

* [Usage](#usage)
* [Getting Started](#getting-started)
    * [Pre-Requisites](#pre-requisites)
    * [Dependencies](#dependencies)
        * [Git and C++ Tools](#git-and-c++-tools)
        * [Libosmium](#libosmium)
        * [Apache Arrow](#apache-arrow)
        * [Osmium Tool](#osmium-tool)
* [Common Problems](#common-problems)
* [Built With](#built-with)
* [Authors](#authors)

## Usage

text

## Getting Started

Currently, the installation and usage of the OSM Processor is supported on Unix systems only. This section provides an installation guide for Linux Ubuntu systems.

### Dependencies

This project depends on multiple third-party libraries which have to be installed prior to the project installation. Before you start installing packages, be sure to update your current packages by entering

```
sudo apt update
```

Now you can proceed by installing following packages:

#### Git and C++ Tools

```
apt-get install -y -V \
  git \
  g++ \
  make \
  cmake \
  doxygen \
  graphviz \
  libboost-dev
```

#### Libosmium

```
apt-get install -y -V \
    libbz2-dev \
    libexpat1-dev \
    libgdal-dev \
    libgeos++-dev \
    liblz4-dev \
    libproj-dev \
    libsparsehash-dev \
    libprotobuf-dev \
    libosmpbf-dev \
    libutfcpp-dev \
    zlib1g-dev \
```

#### Apache Arrow

First, you need to add the Apache JFrog repositories to your system:

```
sudo apt install ca-certificates lsb-release wget -y -V
wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
```

Now, you can install the required Apache Arrow and Parquet libraries by entering:

```
apt-get install -y -V \
    libarrow-dev
    libparquet-dev
```

#### Osmium-Tool (Optional)

Todo

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
./osm-processor -h
```

### Common Problems

## Built with

- [**Boost**](https://www.boost.org/) - Free peer-reviewed portable C++ source libraries - [Github](https://github.com/boostorg/boost)
- [**Libosmium**](https://osmcode.org/libosmium/) - A fast and flexible C++ library for working with OpenStreetMap data - [Github](https://github.com/osmcode/libosmium)
- [**Apache Arrow**](https://arrow.apache.org/) - A language-independent columnar memory format for flat and hierarchical data - [Github](https://github.com/apache/arrow)

## Authors

- **Patrick Lindemann** - Initial work - Github
