# dabradio [![Build Status](https://travis-ci.org/JvanKatwijk/dabradio.svg?branch=master)](https://travis-ci.org/JvanKatwijk/dabradio)


dabradio is a Software for Windows, Linux and Raspberry Pi for listening to terrestrial Digital Audio Broadcasting (DAB and DAB+). It is the little brother of Qt-DAB.

------------------------------------------------------------------
Table of Contents
------------------------------------------------------------------

* [Introduction](#introduction)
* [Features](#features)
* [Installation](#features)
  * [Windows](#windows)
  * [Linux: Ubuntu/x64 and Stretch RPI2/3](#ubuntu-linux)
  	- [Configuring using the dabradio.pro file](#configuring-using-the-dabradio-file)
   	- [Configuring using CMake](#configuring-using-cmake)
   	- [Qt](#qt)
  * [Raspberry PI](#raspberry-pi)
  * [appImage for x64 Linux systems](#appimage-for-x64-linux-systems)
  * [Comment on some settings](#comment-on-some-settings)
  * [A note on intermittent sound](#a-note-on-intermittent-sound)
* [Copyright](#copyright)

-------------------------------------------------------------------
Introduction
-------------------------------------------------------------------

![dabradio with input](/screenshot_dabradio.png?raw=true)

**dabradio** is the little brother of Qt-DAB. The latter is kind of a research vehicle, with lots of options, most of them used by only a few. The need arose to have a smaller brother, just for tuning to and listening to DAB services.

**dabradio** and Qt-DAB share a lot of functionality, obviously. Nevertheless
to avoid even more "ifdef"s in the code, it was decided to
maintain a GitHub repository for both of them.

The Qt-free version, the "command line only" version, is named dab-cmdline, and is built around a library that does the DAB decoding. It has its own repository on Github.

Next to these C++ based versions, a version in Java is being developed, it has its own repository on Github and a GUI that is similar to the one for
**dabradio**.

A new feature is that
rather than selecting a channel, the software maintains a list of
channels that can be received. The list is maintained
between program invocations,
on program start up, these channels are scanned for services.

The GUI does not provide buttons to select the Mode or the Band. Defaults are Mode 1 and the VHF Band III. In the ".ini" file
(a file .dabradio.ini in the home directory of the user) the Mode can be set as well as the band.

The first time the program is started, all channels are scanned,
as is the case whenever the "reset" button is touched.

The services are presented in a separate widget, for each service the
widget contains some additional information.

Device selection is automated, if a device is connected, the software
will detect that and connect to that device. If more than one
device is connected, one will be selected.

------------------------------------------------------------------------
Features
------------------------------------------------------------------------

  * DAB (mp2) and DAB+ (HE-AAC v1, HE-AAC v2 and LC-AAC) decoding
  * MOT SlideShow (SLS)
  * Dynamic Label (DLS) 
  * Both DAB bands supported (default VHF Band III, can be set in the ini file):
  	* VHF Band III
   	* L-Band (only used in Czech Republic and Vatican)
  * Scanning function (scanning over all channels in a given band and collecting
all services)
  * Supports input from various devices: 
  	- SDRplay (both RSP I and RSP II),
  	- Airspy, including Airspy mini,
   	- SDR DAB sticks (RTL2838U or similar), and
 
Data services are not visible to the use, although data as subservice are - limited - implemented.

------------------------------------------------------------------
Windows
------------------------------------------------------------------

Windows releases can be found at https://github.com/JvanKatwijk/dabradio/releases . Please copy them into the same directory you've unzipped http://www.sdr-j.tk/windows-bin.zip as it uses the same libraries.

If you want to compile it by yourself, please install Qt
through its online installer, see https://www.qt.io/ 

------------------------------------------------------------------
Linux: Ubuntu/x64 and Stretch RPI2/3
------------------------------------------------------------------

If you are not familar with compiling then please continue reading by jumping to chapter [appImage](#appimage-for-x64-linux-systems) which is much easier for Linux beginners.

Ubuntu 16.04 (and on) as well as Debian/Stretch on the RPI2 and 3 have good support for Qt5 (note that contrary to Qt-DAB
no use is made of the qwt library). 
For generating an executable under Ubuntu (16.04 or newer) or on
the RPI 2/3 running under Stretch, THE FOLLOWING COMMANDS ARE IN A SCRIPT: build-script. It was tested on RPI2/3 running stretch. 
(For Ubuntu 14.04 look into the package manager for Qt4 packages).

1. Fetch the required components
   ```
   sudo apt-get update
   sudo apt-get install qt5-qmake build-essential g++ git cmake
   sudo apt-get install libsndfile1-dev qt5-default libfftw3-dev portaudio19-dev 
   sudo apt-get install libfaad-dev zlib1g-dev rtl-sdr libusb-1.0-0-dev mesa-common-dev
   sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev libsamplerate0-dev 
   sudo apt-get install qtbase5-dev

   ```
   
2. Fetch the required libraries 

  a) Assuming you want to use a dabstick (also known as rtlsdr) as device, fetch a version of the library for the dabstick
  ```
  wget http://sm5bsz.com/linuxdsp/hware/rtlsdr/rtl-sdr-linrad4.tbz
  tar xvfj rtl-sdr-linrad4.tbz 
  cd rtl-sdr-linrad4
  sudo autoconf
  sudo autoreconf -i
  ./configure --enable-driver-detach
  make
  sudo make install
  sudo ldconfig
  cd
  ```
   	
   b) Assuming you want to use an Airspy as device, fetch a version of the library for the Airspy	
  ```
  sudo apt-get install build-essential cmake libusb-1.0-0-dev pkg-config
  wget https://github.com/airspy/host/archive/master.zip
  unzip master.zip
  cd airspyone_host-master
  mkdir build
  cd build
  cmake ../ -DINSTALL_UDEV_RULES=ON
  make
  sudo make install
  sudo ldconfig
  ```   
	
  Clean CMake temporary files/dirs:
  ```
  cd host-master/build
  rm -rf *
  ```
	
3. Get a copy of the dabradio sources
  ```
  git clone https://github.com/JvanKatwijk/dabradio.git
  cd dabradio
  ```
	
4. Edit the `dabradio.pro` file for configuring the supported devices and other options. Comment the respective lines out if you don't own an Airspy (mini) or an SDRplay.

5. Build and make
  ```
  qmake dabradio.pro
  make
  ```

  You could also use QtCreator, load the `dabradio.pro` file and build the executable.
  
  Remark: The executable file can be found in the sub-directory linux-bin. A make install command is not implemented.


------------------------------------------------------------------
Configuring using the dabradio.pro file
------------------------------------------------------------------

Options in the configuration is to select or unselect devices.

Comment the lines out by prefixing the line with a `#` in the `qt-dab.pro` file (section "unix") for the device(s) you want to exclude in the configuration.
```
CONFIG          += dabstick
CONFIG          += sdrplay
CONFIG          += airspy
```

Audio samples are sent to an audio device using the portaudio library.

If you are compiling/running for an x64 based PC with SSE, then
you could set
```
#CONFIG          += NEON_RPI2
#CONFIG          += NEON_RPI3
CONFIG          += SSE
#CONFIG          += NO_SSE
```

If you are compiling/running for an RPI2, and want to check whether or
not NEON instructions can be used, you could set
```
CONFIG          += NEON_RPI2
#CONFIG          += NEON_RPI3
#CONFIG          += SSE
#CONFIG          += NO_SSE
```

If you are compiling/running for an RPI3, and want to check whether or
not NEON instructions can be used, you could set
```
#CONFIG          += NEON_RPI2
CONFIG          += NEON_RPI3
#CONFIG          += SSE
#CONFIG          += NO_SSE
```

The safest way - always - is to set
```
#CONFIG          += NEON
#CONFIG          += SSE
CONFIG          += NO_SSE
```

Slightly slower since no the other two use specialized instructions
in the viterbi decoding (which is quite heavy in DAB decoding)


Further in the ".pro" file, in the section labeled NEON, you could
choose between compiler flags set for optimizing for the RPI2 or the RPI3
by commenting (or uncommenting) some lines

The default setting in the ".pro" file is NO_SSE, as is the case in
the CMakeLists.txt file.

------------------------------------------------------------------
Configuring using CMake
------------------------------------------------------------------

The `CMakeLists.txt` file has all devices and the spectrum switched off as default. You can select a device (or more devices) without altering the `CMakeLists.txt` file, but by passing on definitions to the command line.

An example:
```
cmake .. -DSDRPLAY=ON -DRTLSDR=ON -DAIRSPY=ON
```
	
will generate a makefile with support for three supported devices,  the SDRplay device the AIRSPY device and the RTLSDR based dabsticks.

The default location for installation depends on your system, mostly `/usr/local/bin` or something like that. Set your own location by adding
```
-DCMAKE_INSTALL_PREFIX=your installation prefix
```

For other options, see the `CMakeLists.txt` file.

-----------------------------------------------------------------
SDRplay
-----------------------------------------------------------------

The current set of sources provides support for the RSP-I and the RSP-II and
the new RSP-1a, it is assumed that at least library version 2.09 is installed.

------------------------------------------------------------------
Qt
------------------------------------------------------------------

The software uses the Qt library, 
the `CMakeLists.txt` assumes Qt5, the dabradio.pro file can
easily be changed to use Qt4.

-----------------------------------------------------------------
Raspberry PI
------------------------------------------------------------------

The dabradio software runs pretty well on the author's RPI-2 ans 3 The average load on the 4 cores on the RPI2
is somewhere between 50 and 60 percent, on the RPI3B+ it is well below 50 percent.

One remark: getting "sound" is not always easy. Be certain that you have installed the alsa-utils, and that you are - as non-root user - able to see devices with `aplay -L`

In arch, it was essential to add the username to the group "audio".

The releases section contains an AppImage developed under and for Raspbian Stretch on an RPI2 and RPI3. Note that libraries for
the rtlsdr DABstick and/or the Airspy can be obtained from repositories for Raspbian Stretch. An API library for the SDRplay can be downloaded from  sdrplay.com

Use is simple, install the libraries as needed, download the AppImage file, chmod 777 dabradio-ARM.AppImage to set the
exec bit and run the program. Running is possible therefore withour compiling anything.

If you want to create your own executable, pls note that an optimal use of the 4 cores of the CPU can be makde
by uncommenting (in the ".pro" file)

	#DEFINES += __THREADED_DECODING.
	#DEFINES += __THREADED_BACKEND

For the CMakeLists.txt file, uncomment 

	#add_definitions (-D__THREADED_DECODING -D__THREADED_BACKEND) #uncomment for the RPI


---------------------------------------------------------------------------
appImages for x64 Linux systems and RPI2/3
---------------------------------------------------------------------------

https://github.com/JvanKatwijk/dabradio/releases contains a generated appImage, dabradio-x64.Appimage, which is created on Ubuntu 14.04 (Trusty), and uses Qt4 (so it basically should run on any x-64 based linux system that isn't too old.).
It assumes that you have installed an appropriate usb library,
libraries to support either a dabstick (i.e. rtlsdr) or an Airspy are
included in the appImage (the appropriate udev rules, i.e. rules to
allow a non-root user to use the device through USB, will be installed
by the execution of the appImage, that is why it will ask for your password. If you have installed the device of your choice, you can just cancel this request).
If you want to run
with an SDRplay, follow the installation instructions for the library from 
"www.sdrplay.com". All further dependencies are included.
The appImage is just a self-contained single file which you have to make executable in order to run.
It furthermore contains an -experimental- appImage for use under Stretch on an RPI2/3, dabradio-ARM.AppImage.

For more information see http://appimage.org/

--------------------------------------------------------------------------------
Comment on some settings
-------------------------------------------------------------------------------

Some values of settings are maintained between program invocations. This is done in the (hidden) file `.dabradio.ini` in the user's home directory.

Some settings are not influenced by buttons or sliders of the GUI, they will only change by editing the .ini file.

Typical examples are

`saveSlides=1` 
when set to 0 the slides that are attached to audio programs will not be saved. If set to 1 the slides will be saved in a directory `/tmp/qt-pictures` (Linux) or in `%tmp%\qt-pictures` (Windows).

`picturesPath` 
defines the directory where the slides (MOT slideshow) should be stored. Default is the home directory.

`showSlides=1` 
when set to 0 the slides will not be shown.

--------------------------------------------------------------------------------
A note on intermittent sound 
-------------------------------------------------------------------------------

In some cases, in some periods of listening, the sound is (or at least seems)
interrupted. There are two different causes for this

First of all the incoming signal is weak and audio packages do not pass the
many controls that are executed. This shows in the widget
"technical data", not all the colored bars at the bottom are 100 percent green. 
An audio package represents 24 milliseconds of audio, loss of a few packages
leads to an interruption of the sound.

A second reason has to do with system parameters. Too small a buffersize
in the audio driver causes too high a frequency of calls to a callback
function. In Linux this shows by an underrun reported by the alsa sound system.
The buffer size can be set (in multiples of 256 audio samples)
by the value of "latency" in the ".ini" file. The default value is 1.

On my RPI 2 - with Stretch - latency=2 works best.

# Copyright


	Copyright (C)  2013, 2014, 2015, 2016, 2017
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

	The Qt-DAB software is made available under the GPL-2.0.
	The SDR-J software, of which the Qt-DAB software is a part, 
	is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

