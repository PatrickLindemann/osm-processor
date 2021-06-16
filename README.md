
### Structure


### Packages

For runtime:
protozero (libprotozero-dev)
osmium (libosmium-dev)

For serialization:
libarrow-dev

Optional:
osmium-tool


libprotobuf-dev
libosmpbf-dev
libutfcpp-dev

### Installation

```
sudo apt-get install libosmium-dev libprotozero-dev
```

sudo apt-get install osmium-tool

Arrow / Parquet:
sudo apt update
sudo apt install -y -V ca-certificates lsb-release wget
wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
sudo apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
sudo apt update
sudo apt install -y -V libarrow-dev # For C++
sudo apt install -y -V libarrow-glib-dev # For GLib (C)
sudo apt install -y -V libarrow-dataset-dev # For Apache Arrow Dataset C++
sudo apt install -y -V libarrow-flight-dev # For Apache Arrow Flight C++
# Notes for Plasma related packages:
#   * You need to enable "non-free" component on Debian GNU/Linux
#   * You need to enable "multiverse" component on Ubuntu
#   * You can use Plasma related packages only on amd64
sudo apt install -y -V libplasma-dev # For Plasma C++
sudo apt install -y -V libplasma-glib-dev # For Plasma GLib (C)
sudo apt install -y -V libgandiva-dev # For Gandiva C++
sudo apt install -y -V libgandiva-glib-dev # For Gandiva GLib (C)
sudo apt install -y -V libparquet-dev # For Apache Parquet C++
sudo apt install -y -V libparquet-glib-dev # For Apache Parquet GLib (C)

Programmweise:

    Programm checkt input file ab. Wenn diese input file bereits pre-processed wurde, nimmt sie entsprechende datei aus cache ordner. Wenn nicht, wird die Datei
    pre-processed (Alle relations mit type=boundary sowie alle zugehörigen nodes und ways rausgezogen) udn in den cache geschrieben. Anschließend werden alle boundaries mit
    boundary=boundary_type und ggf. admin_level rausgezogen. (-> Extra pre-processing für admin_levels? Ist ja häufigster Fall) Danach werden entsprechende Ergebnisse in spezielles
    Format gebracht (-> Polygone mit Punkten, Center point und weiteren infos wie name usw.) und mit Arrow serialisiert (Im Pandas format!!!) in output file, damit sie von Pandas gelesen werden können.

Parameter:
    - Input File (.osm / .pbf)
    - Output File (.parquet) -> All inclusive
    - Boundary Type: "administrative" "historic" etc.
    - Admin Level: If Boundary type == administrative

