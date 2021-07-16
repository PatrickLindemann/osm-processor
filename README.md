# Warzone OSM Mapmaker

This command line utility creates maps for the online strategy game [Warzone](https://www.warzone.com/) by using OpenStreetMap data.

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
    liblz4-dev \
    libproj-dev \
    libsparsehash-dev \
    libutfcpp-dev \
    zlib1g-dev \
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

## Authors

- **Patrick Lindemann** - Initial work - Github
