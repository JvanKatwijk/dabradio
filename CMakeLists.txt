cmake_minimum_required( VERSION 2.8.11 )
set (objectName dabradio)
set (CMAKE_CXX_FLAGS "${CMAKE_XCC_FLAGS} -Wall -std=c++11 -flto")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

set(CMAKE_AUTORCC ON)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/bin" CACHE PATH "default install path" FORCE )
endif()
#set (CMAKE_INSTALL_PREFIX /usr/local/bin)

#	modify if you want

if(MINGW)
    add_definitions ( -municode)
endif()

IF(EXISTS (".git") AND IS_DIRECTORY (".git"))
   execute_process (
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )
endif ()

if(GIT_COMMIT_HASH)
   add_definitions("-DGITHASH=\"${GIT_COMMIT_HASH}\"")
else ()
   add_definitions ("-DGITHASH=\"       \"")
endif()


########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

### make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

######################################################################
#
#####################################################################
#
#	default value
	set(USE_PORTAUDIO true)

if(DEFINED AIRSPY)
   set(AIRSPY true)
endif ()

if(DEFINED SDRPLAY)
   set(SDRPLAY true)
endif ()

if(DEFINED RTLSDR)
   set(RTLSDR true)
endif ()


#add_definitions (-DTHREADED_DECODING)	# uncomment for use for an RPI
#
########################################################################

	find_package (PkgConfig)
	find_package (LSBId)

	find_package (Qt5Core REQUIRED)
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Network REQUIRED)
	include_directories (
	  ${Qt5Network_INCLUDE_DIRS}
	)

        find_package(FFTW3f)
        if (NOT FFTW3F_FOUND)
            message(FATAL_ERROR "please install FFTW3")
        endif ()

        find_package(Faad)
        if (NOT FAAD_FOUND )
            message(FATAL_ERROR "please install libfaad")
        endif ()

        find_package(zlib)
	if (NOT ZLIB_FOUND)
            message(FATAL_ERROR "please install libz")
        endif ()
	list(APPEND extraLibs ${ZLIB_LIBRARY})

        find_package(LibSndFile)
        if (NOT LIBSNDFILE_FOUND)
            message(FATAL_ERROR "please install libsndfile")
        endif ()
        list(APPEND extraLibs ${LIBSNDFILE_LIBRARY})

        find_package(LibSampleRate)
        if (NOT LIBSAMPLERATE_FOUND)
            message(FATAL_ERROR "please install libsamplerate")
        endif ()
        list(APPEND extraLibs ${LIBSAMPLERATE_LIBRARY})

	find_library (PTHREADS pthread)
	if (NOT(PTHREADS))
	   message (FATAL_ERROR "please install libpthread")
	else (NOT(PTHREADS))
	   set (extraLibs ${extraLibs} ${PTHREADS})
	endif (NOT(PTHREADS))

#######################################################################
#
#	Here we really start

	include_directories (
	           ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	           ${QT5Widgets_INCLUDES}
	           ${QT_QTCORE_INCLUDE_DIR}
	           ${QT_QTGUI_INCLUDE_DIR}
	           .
	           ./
	           ./service-description
	           ./includes
	           ./includes/ofdm
	           ./includes/protection
	           ./includes/backend
	           ./includes/backend/audio
	           ./includes/backend/data
	           ./includes/backend/data/journaline
	           ./includes/backend/data/mot
	           ./includes/support
	           ./includes/support/viterbi_768
	           ./includes/output
	           ./devices
	           /usr/include/
	)

	set (${objectName}_HDRS
	     ./dab-processor.h
	     ./includes/dab-constants.h
	     ./service-description/service-descriptor.h 
	     ./service-description/audio-descriptor.h 
	     ./includes/country-codes.h 
	     ./includes/ofdm/timesyncer.h 
             ./includes/ofdm/sample-reader.h
	     ./includes/ofdm/phasereference.h
	     ./includes/ofdm/phasetable.h
	     ./includes/ofdm/freq-interleaver.h
	     ./includes/ofdm/fic-handler.h 
	     ./includes/ofdm/fib-processor.h  
	     ./includes/protection/protTables.h
             ./includes/protection/protection.h
             ./includes/protection/uep-protection.h
             ./includes/protection/eep-protection.h
	     ./includes/backend/firecode-checker.h
	     ./includes/backend/frame-processor.h
	     ./includes/backend/charsets.h
	     ./includes/backend/galois.h
	     ./incluces/backend/reed-solomon.h
	     ./includes/backend/msc-handler.h
	     ./includes/backend/backend.h
             ./includes/backend/backend-deconvolver.h
             ./includes/backend/backend-driver.h
	     ./includes/backend/audio/faad-decoder.h
	     ./includes/backend/audio/mp4processor.h
	     ./includes/backend/audio/mp2processor.h
	     ./includes/backend/data/virtual-datahandler.h
	     ./includes/backend/data/pad-handler.h
	     ./includes/backend/data/mot/mot-handler.h 
	     ./includes/backend/data/mot/mot-object.h 
	     ./includes/backend/data/mot/mot-dir.h 
	     ./includes/backend/data/data-processor.h
	     ./devices/device-handler.h
	     ./includes/output/fir-filters.h
	     ./includes/output/audio-base.h
	     ./includes/output/newconverter.h
	     ./includes/support/viterbi_768/viterbi-768.h
	     ./includes/support/fft-handler.h
	     ./includes/support/ringbuffer.h
	     ./includes/support/Xtan2.h
	     ./includes/support/dab-params.h
	     ./includes/support/band-handler.h
	     ./includes/support/text-mapper.h
	     ./includes/support/dab_tables.h 

	)

	set (${objectName}_SRCS
	     ${${objectName}_SRCS}
	     ./main.cpp
	     ./dab-processor.cpp
	     ./service-description/service-descriptor.cpp 
	     ./service-description/audio-descriptor.cpp 
	     ./src/ofdm/timesyncer.cpp 
             ./src/ofdm/sample-reader.cpp
	     ./src/ofdm/ofdm-decoder.cpp
	     ./src/ofdm/phasereference.cpp
	     ./src/ofdm/phasetable.cpp
	     ./src/ofdm/freq-interleaver.cpp
	     ./src/ofdm/fic-handler.cpp
	     ./src/ofdm/fib-processor.cpp
	     ./src/protection/protTables.cpp
	     ./src/protection/protection.cpp
	     ./src/protection/eep-protection.cpp
	     ./src/protection/uep-protection.cpp
	     ./src/backend/firecode-checker.cpp
	     ./src/backend/frame-processor.cpp
	     ./src/backend/charsets.cpp
	     ./src/backend/galois.cpp
	     ./src/backend/reed-solomon.cpp
	     ./src/backend/msc-handler.cpp
	     ./src/backend/backend.cpp
	     ./src/backend/backend-deconvolver.cpp
             ./src/backend/backend-driver.cpp
	     ./src/backend/audio/faad-decoder.cpp
	     ./src/backend/audio/mp4processor.cpp
	     ./src/backend/audio/mp2processor.cpp
	     ./src/backend/data/virtual-datahandler.cpp
	     ./src/backend/data/pad-handler.cpp
	     ./src/backend/data/mot/mot-handler.cpp 
	     ./src/backend/data/mot/mot-object.cpp 
	     ./src/backend/data/mot/mot-dir.cpp 
	     ./src/backend/data/data-processor.cpp
	     ./devices/device-handler.cpp
	     ./src/output/audio-base.cpp
	     ./src/output/newconverter.cpp
	     ./src/output/fir-filters.cpp
	     ./src/support/fft-handler.cpp
	     ./src/support/Xtan2.cpp
	     ./src/support/dab-params.cpp
	     ./src/support/band-handler.cpp
	     ./src/support/text-mapper.cpp
	     ./src/support/dab_tables.cpp
	     ./src/support/viterbi_768/viterbi-768.cpp
	     ./src/support/viterbi_768/spiral-no-sse.c
	)

	set (${objectName}_UIS
                ${${objectName}_UIS} ./forms/dabradio.ui
	                             ./forms/audio-description.ui)

	set (${objectName}_MOCS
	     ./dab-processor.h
	     ./service-description/service-descriptor.h 
	     ./service-description/audio-descriptor.h 
	     ./devices/device-handler.h
	     ./includes/output/audio-base.h
	     ./includes/ofdm/sample-reader.h
	     ./includes/ofdm/ofdm-decoder.h
	     ./includes/ofdm/phasereference.h
	     ./includes/ofdm/fic-handler.h
	     ./includes/ofdm/fib-processor.h
	     ./includes/backend/backend.h
	     ./includes/backend/audio/faad-decoder.h
	     ./includes/backend/audio/mp2processor.h
	     ./includes/backend/audio/mp4processor.h
	     ./includes/backend/data/virtual-datahandler.h
	     ./includes/backend/data/pad-handler.h
	     ./includes/backend/data/mot/mot-handler.h 
	     ./includes/backend/data/mot/mot-object.h 
	     ./includes/backend/data/mot/mot-dir.h 
	     ./includes/backend/data/data-processor.h
	)


	set (${objectName}_MOCS
	        ${${objectName}_MOCS} ./radio.h)

	set ($(objectName)_HDRS
	        ${${objectName}_HDRS} ./radio.h)

	set (${objectName}_SRCS
	        ${${objectName}_SRCS} ./radio.cpp)


##########################################################################
#	The devices
#
	if (SDRPLAY)
#	   find_path (SDRPLAYLIB_INCLUDE_DIR
#                      NAMES mirsdrapi-rsp.h
#                      PATHS
#                      /usr/local/include/
#	   )
#	   include_directories (${SDRPLAYLIB_INCLUDE_DIR})

	   include_directories (
	     ./devices/sdrplay-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	        ./devices/sdrplay-handler/mirsdrapi-rsp.h
           )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler/sdrplay-handler.cpp
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	   )

	   add_definitions (-DHAVE_SDRPLAY)
	endif (SDRPLAY)

	if (AIRSPY)
#           find_package(LibAIRSPY)
#           if (NOT LIBAIRSPY_FOUND)
#               message(FATAL_ERROR "please install airspy library")
#           endif ()
#	   ## include_directories (${AIRSPYLIB_INCLUDE_DIR})

	   include_directories (
	     ./devices/airspy-handler
	     ./devices/airspy-handler/libairspy
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/airspy-handler/airspy-handler.h
	        ./devices/airspy-handler/airspyfilter.h
	        ./devices/airspy-handler/libairspy/airspy.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/airspy-handler/airspy-handler.cpp
	        ./devices/airspy-handler/airspyfilter.cpp
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/airspy-handler/airspy-handler.h
	   )

	   add_definitions (-DHAVE_AIRSPY)
	endif (AIRSPY)

#
	if (RTLSDR)
           find_package(LibRTLSDR)
           if (NOT LIBRTLSDR_FOUND)
               message(FATAL_ERROR "please install librtlsdr")
           endif ()
	   ###include_directories (${RTLSDR_INCLUDE_DIR})

	   include_directories (
	        ./devices/rtlsdr-handler/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.cpp
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )
	   add_definitions (-DHAVE_RTLSDR)
	endif()
#
	if (TCP_STREAMER)
	   add_definitions (-DTCP_STREAMER)
	   find_package (Qt5Network REQUIRED)
	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./includes/output/tcp-streamer.h
	   )
	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./includes/output/tcp-streamer.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/output/tcp-streamer.cpp
	   )
	endif (TCP_STREAMER)

	if (QT_AUDIO)
	   add_definitions (-DQT_AUDIO)
	   find_package (Qt5Multimedia REQUIRED)
	   include_directories (
	      ${Qt5Multimedia_INCLUDE_DIRS}
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./includes/output/Qt-audio.h
	        ./includes/output/Qt-audiodevice.h
	   )
	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./includes/output/Qt-audio.h
	        ./includes/output/Qt-audiodevice.h
	   )
	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/output/Qt-audio.cpp
	        ./src/output/Qt-audiodevice.cpp
	   )
	endif (QT_AUDIO)

	if (USE_PORTAUDIO)
           find_package(Portaudio)
           if (NOT PORTAUDIO_FOUND)
               message(FATAL_ERROR "please install portaudio V19")
           endif ()
           list(APPEND extraLibs ${PORTAUDIO_LIBRARIES})
	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	       ./includes/output/audiosink.h
	   )

	   set (${objectName}_MOCS
	       ${${objectName}_MOCS}
	      ./includes/output/audiosink.h
	   )

	   set (${objectName}_SRCS
	      ${${objectName}_SRCS}
	      ./src/output/audiosink.cpp
	   )
	endif (USE_PORTAUDIO)

	QT5_WRAP_UI (UIS ${${objectName}_UIS})

	include_directories (
	          ${SDRPLAY_INCLUDES}
	          ${QT5Widgets_INCLUDES}
	          ${QT_QTCORE_INCLUDE_DIR}
	          ${QT_QTGUI_INCLUDE_DIR}
	          ${FFTW_INCLUDE_DIRS}
	          ${PORTAUDIO_INCLUDE_DIRS}
	          ${FAAD_INCLUDE_DIRS}
	          ${SNDFILES_INCLUDE_DIRS}
	)

	QT5_WRAP_CPP (MOCS ${${objectName}_MOCS})

	add_executable (${objectName}
	                ${${objectName}_SRCS}
	                ${UIS}
	                ${RSCS}
	                ${TRS}
	                ${MOCS}
	                resources.qrc
	)

	target_link_libraries (${objectName}
	                       Qt5::Widgets
	                       Qt5::Network
	                       ${RTLTCP_lib}
	                       ${FFTW3F_LIBRARIES}
	                       ${extraLibs}
	                       ${FAAD_LIBRARIES}
	                       ${CMAKE_DL_LIBS}
	)

	INSTALL (TARGETS ${objectName} DESTINATION  ${CMAKE_INSTALL_PREFIX})
#	INSTALL (TARGETS ${objectName} DESTINATION ./linux-bin)

########################################################################
# Create uninstall target
########################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
