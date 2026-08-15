# 3DSync

Homebrew for Nintendo 3DS/2DS console family that allows synchronization of saves and files to a cloud, to another console or to a PC.

The configurator has built-in support for Checkpoint and JKSM folders, but you can add custom paths if you like.

**Currently supports Dropbox for cloud synchronization.**

This repository is based on the original 3DSync project and contains modifications to improve Dropbox synchronization and restore the project's build environment.

## Changes and improvements

### Manual Dropbox synchronization

The application now provides explicit controls for synchronization:

```text
[A] Download from Dropbox
[B] Upload to Dropbox
[START] Exit
```

Download and upload are independent operations.

Pressing `A` downloads the configured files from Dropbox to the Nintendo 3DS.

Pressing `B` uploads the configured local files from the Nintendo 3DS to Dropbox.

The download flow does **not** automatically upload the downloaded files back to Dropbox.

### Recursive Dropbox downloads

Custom Dropbox paths are downloaded recursively.

The application traverses Dropbox folders, downloads files from nested directories and recreates the corresponding directory structure on the Nintendo 3DS.

Missing local directories are created automatically.

### Citra save downloading

The application was modified in April 2024 so that when syncing Citra saves into the 3DS, **all save slots are downloaded instead of only the first one**.

To use this functionality, place your entire Citra `sdmc` directory in your Dropbox's `Apps/3DSyncU` directory.

The final directory structure should look something like:

```text
Apps
└── 3DSyncU
    └── sdmc
        └── Nintendo 3DS
            └── ...
```

### Build tools

The project also includes the build tools that were previously provided through an external submodule.

The original `buildtools` repository became inaccessible after it was archived/deleted, returning a 404 error.

The required build tools and legacy dependencies have therefore been included in the repository, allowing the project to be compiled without depending on the unavailable original repository.

## Usage

1. Follow the steps on the [configurator page](https://concreted.github.io/3DSync/) to obtain the configuration file.
2. Place the configurator file in the following folder of the console SD card:

```text
/3ds/3DSync/3DSync.ini
```

3. Download and install the `.cia` file, or run the `.3dsx` from the Homebrew Launcher.
4. Use `A` to download from Dropbox or `B` to upload to Dropbox.

## Dropbox authentication

3DSync uses a Dropbox OAuth refresh token.

The configuration contains the refresh token under the `[Dropbox]` section:

```ini
[Dropbox]
RefreshToken=YOUR_REFRESH_TOKEN
```

**Never publish your real refresh token or commit it to Git.**

When 3DSync starts, the refresh token is exchanged for a temporary Dropbox access token. This access token is then used for the Dropbox API operations during the session.

## Development

Follow the steps below to install build dependencies and compile on Ubuntu.

### Basic dependencies

```bash
sudo apt update
sudo apt install zip unzip wget git gdebi-core -y
```

### Install devkitPro with 3DS support

```bash
sudo apt-get install gdebi-core

wget https://github.com/devkitPro/pacman/releases/download/v1.0.2/devkitpro-pacman.amd64.deb
```

> The `1.0.2` package is deprecated and may no longer be included in the official releases section. A copy is included in the `compilation_tools_and_dependencies` folder.

Install it with:

```bash
sudo gdebi devkitpro-pacman.amd64.deb
```

Then install the Nintendo 3DS development packages:

```bash
sudo dkp-pacman -S 3ds-dev
```

Load the devkitPro environment:

```bash
source /etc/profile.d/devkit-env.sh
```

### Install additional 3DS dependencies

```bash
sudo dkp-pacman -S 3ds-curl 3ds-mbedtls 3ds-libjson-c
```

### bannertool

The project requires `bannertool`.

The version originally used by the project may no longer be available from its original download location. A copy is included in the `compilation_tools_and_dependencies` folder.

If required, extract it and install it:

```bash
unzip bannertool.zip -d bannertool
sudo cp bannertool/linux-x86_64/bannertool /usr/local/bin
sudo chmod +x /usr/local/bin/bannertool
```

### makerom

The project also requires `makerom`.

A copy is included in `compilation_tools_and_dependencies` for convenience:

```bash
unzip makerom-v0.18.3-ubuntu_x86_64.zip -d makerom
sudo cp makerom/makerom /usr/local/bin
sudo chmod +x /usr/local/bin/makerom
```

The original download location is:

https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.18.3/makerom-v0.18.3-ubuntu_x86_64.zip

### Build the project

Clone the repository:

```bash
mkdir -p ~/src
cd ~/src

git clone https://github.com/PedroPerez14/3DSync-multisave.git
cd 3DSync-multisave
```

Then build the Nintendo 3DS binaries:

```bash
make
```

The project uses the build system included in:

```text
buildtools/
```

## Compilation dependencies

Some dependencies in the original compilation instructions are no longer available or may return an error when downloaded from their original locations.

The missing packages and tools have been added to:

```text
compilation_tools_and_dependencies/
```

This folder is intended to save time when setting up the build environment and allows the project to be compiled despite the original dependencies becoming unavailable.

The repository currently includes, among others:

```text
compilation_tools_and_dependencies/
├── bannertool.zip
├── devkitpro-pacman-1.0.2.amd64.deb
└── makerom-v0.18.3-ubuntu_x86_64.zip
```

The compilation environment was tested on **Ubuntu 20.04**.

The Nintendo 3DS application has been tested on real hardware. The PC testing binary described below is a separate test and is not intended to reproduce the complete Nintendo 3DS application.

## Local Dropbox testing

A testing binary can be built to test Dropbox functionality locally.

Before building, you may have to comment out the `VERSION_STRING` reference in:

```text
source/utils/curl.cpp
```

Build the test binary:

```bash
g++ -o test test.cpp source/modules/dropbox.cpp source/utils/curl.cpp -lcurl -ljson-c
```

Set your Dropbox refresh token as an environment variable:

```bash
export DROPBOX_REFRESH_TOKEN=<your Dropbox refresh token>
```

Run the test binary:

```bash
./test
```

The refresh token should never be committed to the repository.

## Dropbox implementation

Dropbox communication uses the Dropbox HTTP API through `libcurl`.

The application uses the Dropbox API for:

* OAuth token exchange.
* Folder listing.
* File uploads.
* File downloads.

Downloads are performed recursively for configured custom paths.

During recursive downloads, separate Dropbox/CURL instances are used for folder listing and file downloads. This avoids stale CURL request state between `list_folder` and subsequent file download requests.

## Project structure

```text
3DSync-multisave/
├── buildtools/
├── compilation_tools_and_dependencies/
├── meta/
├── source/
│   ├── libs/
│   │   └── inih/
│   ├── modules/
│   │   ├── citra.h
│   │   ├── dropbox.cpp
│   │   ├── dropbox.h
│   │   └── time.h
│   ├── utils/
│   │   ├── curl.cpp
│   │   └── curl.h
│   └── main.cpp
├── Makefile
├── test.cpp
├── test_download.cpp
├── wsl_sync.sh
└── README.md
```

## Testing

The modified Dropbox synchronization functionality has been tested on **real Nintendo 3DS hardware**.

The following operations have been successfully verified:

* Dropbox authentication.
* Dropbox → Nintendo 3DS download.
* Nintendo 3DS → Dropbox upload.
* Manual `A` download operation.
* Manual `B` upload operation.
* Recursive Dropbox directory traversal.
* Automatic creation of missing local directories.
* Save file synchronization.

## Original project and credits

This project is based on the original 3DSync project:

https://github.com/PedroPerez14/3DSync-multisave

Original author:

**Kyraminol**

The project also uses:

* devkitPro / devkitARM
* libctru
* bannertool
* makerom
* libcurl
* mbedTLS
* json-c
* Dropbox API
* Checkpoint
* JKSM
* Citra

Please respect the licenses of the original project and all third-party components.

## Disclaimer

Always keep an independent backup of important save data before synchronizing or replacing files.

Cloud synchronization should not be considered a replacement for independent backups.

Use this software at your own risk.
