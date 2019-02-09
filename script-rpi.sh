
sudo apt-get update
sudo apt-get install qt4-qmake build-essential g++
sudo apt-get install libsndfile1-dev qt4-default libfftw3-dev portaudio19-dev 
sudo apt-get install libfaad-dev zlib1g-dev libusb-1.0-0-dev mesa-common-dev
sudo apt-get install libgl1-mesa-dev libqt4-opengl-dev libsamplerate-dev libqwt-dev

wget http://sm5bsz.com/linuxdsp/hware/rtlsdr/rtl-sdr-linrad4.tbz
tar xvfj rtl-sdr-linrad4.tbz
cd rtl-sdr-linrad4
mkdir build
cd build
cmake .. -DDETACH_KERNEL_DRIVER=ON  -DINSTALL_UDEV_RULES=ON
make
sudo make install
sudo ldconfig
cd ..
cd ..

qmake-qt4
make

