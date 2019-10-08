cmake_minimum_required(VERSION 3.6)
project(ACBackup)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 17)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

add_executable(ACBackup src/main.cpp src/commands/Commands.hpp src/commands/Init.cpp src/Config.hpp src/Config.cpp src/commands/AddSnapshot.cpp src/status/StatusTracker.hpp src/backup/SnapshotManager.hpp src/indexing/OSFileSystemNodeIndex.hpp src/indexing/FileSystemNodeIndex.hpp src/status/StatusTracker.cpp src/status/StatusTrackerWebService.hpp src/backup/SnapshotManager.cpp src/indexing/OSFileSystemNodeIndex.cpp src/status/StatusTrackerWebService.cpp src/backup/Snapshot.hpp src/backup/Snapshot.cpp src/status/ProcessStatus.hpp src/indexing/FileSystemNodeAttributes.hpp)
target_link_libraries(ACBackup Std++ Std++Static)