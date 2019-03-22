cmake_minimum_required(VERSION 3.6)
project(ACBackup)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 17)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

add_executable(ACBackup src/main.cpp src/FileIndex.hpp src/FileSystemIndex.hpp src/FileSystemIndex.cpp src/BackupManager.hpp src/BackupManager.cpp src/FlatContainerIndex.hpp src/FlatContainerIndex.cpp src/BackupContainerIndex.hpp src/BackupContainerIndex.cpp src/FlatContainerFileSystem.hpp src/FlatContainerFileSystem.cpp src/StatusTracker.hpp src/StatusTracker.cpp src/resources.hpp src/StatusTrackerWebService.hpp src/StatusTrackerWebService.cpp src/ProcessStatus.hpp src/ProcessStatus.cpp src/KeyDerivation.hpp src/KeyDerivation.cpp src/KeyVerificationFailedException.hpp src/IndexingReadException.hpp)
target_link_libraries(ACBackup Std++ Std++Static)