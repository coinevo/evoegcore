#------------------------------------------------------------------------------
# CMake helper for the majority of the cpp-ethereum modules.
#
# This module defines
#     Coinevo_XXX_LIBRARIES, the libraries needed to use ethereum.
#     Coinevo_FOUND, If false, do not try to use ethereum.
#
# File addetped from cpp-ethereum
#
# The documentation for cpp-ethereum is hosted at http://cpp-ethereum.org
#
# ------------------------------------------------------------------------------
# This file is part of cpp-ethereum.
#
# cpp-ethereum is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cpp-ethereum is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2014-2016 cpp-ethereum contributors.
#------------------------------------------------------------------------------

#set(LIBS        common;blocks;cryptonote_basic;cryptonote_core;
#		cryptonote_protocol;daemonizer;mnemonics;epee;lmdb;device;
#                blockchain_db;ringct;wallet;cncrypto;easylogging;version;checkpoints;
#                ringct_basic;randomx;hardforks)


if (NOT COINEVO_DIR)
    set(COINEVO_DIR ~/coinevo)
endif()

message(STATUS COINEVO_DIR ": ${COINEVO_DIR}")

set(COINEVO_SOURCE_DIR ${COINEVO_DIR}
        CACHE PATH "Path to the root directory for Coinevo")

# set location of coinevo build tree
set(COINEVO_BUILD_DIR ${COINEVO_SOURCE_DIR}/build/release/
        CACHE PATH "Path to the build directory for Coinevo")


if (NOT EXISTS ${COINEVO_BUILD_DIR})   
    # try different location   
    message(STATUS "Trying different folder for coinevo libraries")
    set(COINEVO_BUILD_DIR ${COINEVO_SOURCE_DIR}/build/Linux/master/release/
        CACHE PATH "Path to the build directory for Coinevo" FORCE)
endif()


if (NOT EXISTS ${COINEVO_BUILD_DIR})   
  message(FATAL_ERROR "Coinevo libraries not found in: ${COINEVO_BUILD_DIR}")
endif()


set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${COINEVO_BUILD_DIR}"
        CACHE PATH "Add Coinevo directory for library searching")


set(LIBS  cryptonote_core
          blockchain_db
          #cryptonote_protocol
          cryptonote_basic
          #daemonizer
          blocks
          lmdb
          ringct
          ringct_basic
          common
          #mnemonics
          easylogging
          device
          epee
          checkpoints
          version
          cncrypto
          randomx
          hardforks)

set(Evo_INCLUDE_DIRS "${CPP_COINEVO_DIR}")

# if the project is a subset of main cpp-ethereum project
# use same pattern for variables as Boost uses

set(Coinevo_LIBRARIES "")

foreach (l ${LIBS})

	string(TOUPPER ${l} L)

	find_library(Evo_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
                        PATH_SUFFIXES "/src/${l}"
                                      "/src/"
                                      "/external/db_drivers/lib${l}"
                                      "/lib" "/src/crypto"
                                      "/contrib/epee/src"
                                      "/external/easylogging++/"
                                      "/src/ringct/"
                                      "/external/${l}"
			NO_DEFAULT_PATH
			)

	set(Evo_${L}_LIBRARIES ${Evo_${L}_LIBRARY})

	message(STATUS FindCoinevo " Evo_${L}_LIBRARIES ${Evo_${L}_LIBRARY}")

    add_library(${l} STATIC IMPORTED)
	set_property(TARGET ${l} PROPERTY IMPORTED_LOCATION ${Evo_${L}_LIBRARIES})

    set(Coinevo_LIBRARIES ${Coinevo_LIBRARIES} ${l} CACHE INTERNAL "Coinevo LIBRARIES")

endforeach()


message("FOUND Coinevo_LIBRARIES: ${Coinevo_LIBRARIES}")

message(STATUS ${COINEVO_SOURCE_DIR}/build)

#macro(target_include_coinevo_directories target_name)

    #target_include_directories(${target_name}
        #PRIVATE
        #${COINEVO_SOURCE_DIR}/src
        #${COINEVO_SOURCE_DIR}/external
        #${COINEVO_SOURCE_DIR}/build
        #${COINEVO_SOURCE_DIR}/external/easylogging++
        #${COINEVO_SOURCE_DIR}/contrib/epee/include
        #${COINEVO_SOURCE_DIR}/external/db_drivers/liblmdb)

#endmacro(target_include_coinevo_directories)


add_library(Coinevo::Coinevo INTERFACE IMPORTED GLOBAL)

# Requires to new cmake
#target_include_directories(Coinevo::Coinevo INTERFACE        
    #${COINEVO_SOURCE_DIR}/src
    #${COINEVO_SOURCE_DIR}/external
    #${COINEVO_SOURCE_DIR}/build
    #${COINEVO_SOURCE_DIR}/external/easylogging++
    #${COINEVO_SOURCE_DIR}/contrib/epee/include
    #${COINEVO_SOURCE_DIR}/external/db_drivers/liblmdb)

set_target_properties(Coinevo::Coinevo PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES 
            "${COINEVO_SOURCE_DIR}/src;${COINEVO_SOURCE_DIR}/external;${COINEVO_SOURCE_DIR}/build;${COINEVO_SOURCE_DIR}/external/easylogging++;${COINEVO_SOURCE_DIR}/contrib/epee/include;${COINEVO_SOURCE_DIR}/external/db_drivers/liblmdb")


target_link_libraries(Coinevo::Coinevo INTERFACE
    ${Coinevo_LIBRARIES})
