# dabradio [![Build Status](https://travis-ci.org/JvanKatwijk/qt-dab.svg?branch=master)](https://travis-ci.org/JvanKatwijk/dabradio)

dabradio is a Software for Windows, Linux and Raspberry Pi for listening to terrestrial Digital Audio Broadcasting (DAB and DAB+). It is the  smaller brother of Qt-DAB, the successor of both DAB-rpi and sdr-j-DAB, two former programs by the same author.

------------------------------------------------------------------
Table of Contents
------------------------------------------------------------------

* [Introduction](#introduction)
* [Features](#features)
* [Installation](#features)
  * [Windows](#windows)
  * [Ubuntu Linux](#ubuntu-linux)
  	- [Configuring using the dabradio.pro file](#configuring-using-the-dabradio-file)
   	- [Configuring using CMake](#configuring-using-cmake)
   	- [Qt](#qt)
  * [Raspberry PI](#raspberry-pi)
  * [appImage for x64 Linux systems](#appimage-for-x64-linux-systems)
  * [Comment on some settings](#comment-on-some-settings)
  * [A note on intermittent sound](#a-note-on-intermittent-sound)
* [Copyright](#copyright)
 
------------------------------------------------------------------
Features
------------------------------------------------------------------

  * DAB (mp2) and DAB+ (HE-AAC v1, HE-AAC v2 and LC-AAC) decoding
  * MOT SlideShow (SLS)
  * Dynamic Label (DLS) 
  * Both DAB bands supported: 
  	* VHF Band III
   	* L-Band (only used in Czech Republic and Vatican)
  * Scanning function (scanning over all channels in a given band and collecting
all services)
  * Detailed information for selected service (SNR, bitrate, frequency, ensemble name, ensemble ID, subchannel ID, used CUs, protection level, CPU usage, program type, language, 4 quality bars)
  * Supports various inputs from 
  	- SDRplay (both RSP I and RSP II),
  	- Airspy, including Airspy mini,
   	- SDR DAB sticks (RTL2838U or similar), and
   	- prerecorded dump (*.sdr, and *.iq) 
 
Data services are not  implemented, although data as subservice are - limited - implemented.

 ------------------------------------------------------------------
Introduction
------------------------------------------------------------------

![dabradio with input](/screenshot_dabradio.png?raw=true)

**dabradio** is the little brother of Qt-DAB. The latter is kind of a research vehicle, with lost of options, used by only a few. The need arose to have
a smaller brother, just for listening to DAB services.

**dabradio** and Qt-DAB share a lot of functionality, obviously, nevertheless
to avoid even more "ifdef"s in the code, it was decided to
maintain a GitHub repository for both of them.

The Qt-free version, the "command line only" version, is named dab-cmdline, and is built around a library that does the DAB decoding. It has its own repository on Github.

Next to these C++ based versions, a version in Java is being developed, it has its own repository on Github.

**dabradio** dynamically selects the input device, there is no input device selector on the GUI.
If an input device (one of SDRplay, AIRspy or RTLSDR stick) is attached, the software will detect and
use that device (if more than one device is connected, the software will select one of them).
If no external device is detected, the software will present a menu to select a file for file input.

For further information please visit http://www.sdr-j.tk

Some settings are preserved between program invocations, they are stored in a file `.dabradio.ini`, to be found in the home directory. See [Comment on some settings](#comment-on-some-settings) for more details.

------------------------------------------------------------------
Windows
------------------------------------------------------------------

Windows releases can be found at https://github.com/JvanKatwijk/dabradio/releases . Please copy them into the same directory you've unzipped http://www.sdr-j.tk/windows-bin.zip as it uses the same libraries.

If you want to compile it by yourself, please install Qt through its online installer, see https://www.qt.io/ 

------------------------------------------------------------------
Ubuntu Linux
------------------------------------------------------------------

If you are not familar with compiling then please continue reading by jumping to chapter [appImage](#appimage-for-x64-linux-systems) which is much easier for Linux beginners.

Ubuntu 16.04 (and on) have good support for Qt5 and qwt (compiled for Qt5).
For generating an executable under Ubuntu (16.04 or newer), you can put the following commands into a script. 
(For Ubuntu 14.04 look into the package manager for Qt4 packages)

1. Fetch the required components
   ```
   sudo apt-get update
   sudo apt-get install qt5-qmake build-essential g++
   sudo apt-get install libsndfile1-dev qt5-default libfftw3-dev portaudio19-dev 
   sudo apt-get install libfaad-dev zlib1g-dev rtl-sdr libusb-1.0-0-dev mesa-common-dev
   sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev libsamplerate0-dev libqwt-qt5-dev
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

5. If DAB spectrum and the constellation diagram should be displayed, check the installation path to qwt. If you were downloading it from http://qwt.sourceforge.net/qwtinstall.html please mention the correct path in `dabradio.pro` file (for other installation change it accordingly): 
  ```
  INCLUDEPATH += /usr/local/include  /usr/local/qwt-6.1.3
  ```
	
6. Build and make
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

Comment the lines out by prefixing the line with a `#` in the `qt-dab.pro` file (section "unix") for the device(s) you want to exclude in the configuration. In the example below, rtl_tcp (i.e. the connection to the rtlsdr server) won't be used.
```
CONFIG          += dabstick
CONFIG          += sdrplay
CONFIG          += airspy
```

Remark: Input from pre-recorded files (8 bit unsigned `*.raw` and `*.iq' as well as 16-bit "wav" `*.sdr` files) is configured by default.

Audio samples are - by default - sent to an audio device using the portaudio library.

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

Important: Note that CMakeLists.txt file expects the appropriate Qt version (and - if configured - the qwt library) to be installed.

-----------------------------------------------------------------
SDRplay
-----------------------------------------------------------------

The current set of sources provides support for the RSP-I and the RSP-II and
the new RSP-1a, it is assumed that at least library version 2.09 is installed.

------------------------------------------------------------------
Qt
------------------------------------------------------------------

The software uses the Qt library and - for the spectrum and the constellation diagram - the qwt library.

The `CMakeLists.txt` assumes Qt5, if you want to use Qt4, and you want to have the spectrum in the configuration, be aware of the binding of the qwt library (i.e. Qt4 and a qwt that uses Qt5 does not work well).  

-----------------------------------------------------------------
Raspberry PI
------------------------------------------------------------------

The dabradio software runs pretty well on the author's RPI-2. The average load on the 4 cores is somewhere between 50 and 60 percent.

One remark: getting "sound" is not always easy. Be certain that you have installed the alsa-utils, and that you are - as non-root user - able to see devices with `aplay -L`

In arch, it was essential to add the username to the group "audio".

The most recent distribution of Raspbian Stretch (i.e. august 2017) supports both Qt5 and a qwt compiled against Qt5.

IMPORTANT NOTE:
Since I was studying the (potential) difference in behaviour between a version with and a version without concurrency in the front end, there is a setting in the ".pro" file for selecting this.
Use for the dabradio the concurrency option.

For the ".pro" file uncomment 

	#DEFINES += __THREADED_DECODING.

For the CMakeLists.txt file, uncomment 

	#add_definitions (-D__THREADED_DECODING) #uncomment for the RPI


---------------------------------------------------------------------------
appImage for x64 Linux systems
---------------------------------------------------------------------------

https://github.com/JvanKatwijk/dabradio/releases contains a generated appImage which is created on Ubuntu 14.04 (Trusty), and uses Qt4 (so it basically should run on any x-64 based linux system that isn't too old.). It assumes that you have installed a device, either a dabstick (i.e. rtlsdr), an Airspy or a SDRplay. All further dependencies are included. There is only one file which you have to make executable in order to run.

Note that on start up the appImage will try to set the udev settings for the airspy and dabstick right. Libraries for the dabstick (i.e. rtlsdr) and airspy are part of the appImage. Note that while the SDRplay is selectable, the library for the device should be installed from the supplier, i.e. "www.sdrplay.com".

All further dependencies are included

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

