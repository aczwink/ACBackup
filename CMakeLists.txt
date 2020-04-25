cmake_minimum_required(VERSION 3.6)
project(ACBackup)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 17)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

add_executable(ACBackup src/main.cpp src/commands/Commands.hpp src/commands/Init.cpp src/config/CompressionStatistics.hpp src/config/CompressionStatistics.cpp src/config/ConfigManager.hpp src/config/ConfigManager.cpp src/InjectionContainer.hpp src/commands/AddSnapshot.cpp src/indexing/OSFileSystemNodeIndex.hpp src/indexing/OSFileSystemNodeIndex.cpp src/status/StatusTracker.hpp src/status/StatusTracker.cpp src/status/TerminalStatusTracker.hpp src/config/Config.hpp src/status/ProcessStatus.hpp src/config/ConfigException.hpp src/indexing/FileSystemNodeAttributes.hpp src/indexing/FileSystemNodeIndex.hpp src/backup/SnapshotManager.hpp src/backup/SnapshotManager.cpp src/backup/Snapshot.hpp src/backup/Snapshot.cpp src/indexing/FileSystemNodeIndex.cpp src/backup/BackupNodeAttributes.hpp src/backup/BackupNodeAttributes.cpp src/backupfilesystem/FlatVolumesFileSystem.hpp src/backupfilesystem/FlatVolumesFileSystem.cpp src/backupfilesystem/VolumesOutputStream.hpp src/backupfilesystem/VolumesOutputStream.cpp src/backup/BackupNodeIndex.hpp src/backup/BackupNodeIndex.cpp src/Util.hpp src/Util.cpp src/status/TerminalStatusTracker.cpp src/status/ProcessStatus.cpp src/commands/VerifySnapshot.cpp src/backupfilesystem/FlatVolumesFile.hpp src/backupfilesystem/FlatVolumesFile.cpp src/backupfilesystem/FlatVolumesBlockInputStream.hpp src/backupfilesystem/FlatVolumesBlockInputStream.cpp src/backupfilesystem/FlatVolumesDirectory.hpp src/backupfilesystem/FlatVolumesDirectory.cpp src/Serialization.hpp src/status/WebStatusTracker.hpp src/status/WebStatusTracker.cpp src/status/StatusTrackerWebService.hpp src/status/StatusTrackerWebService.cpp src/status/webresources.hpp src/indexing/LinkPointsOutOfIndexDirException.hpp src/backupfilesystem/FlatVolumesLink.hpp src/backupfilesystem/FlatVolumesLink.cpp src/backup/VirtualSnapshotFilesystem.hpp src/backup/VirtualSnapshotFilesystem.cpp src/CompressionSetting.hpp src/commands/Diff.cpp src/NodeIndexDifferenceResolver.hpp src/NodeIndexDifferenceResolver.cpp src/commands/OutputSnapshotStats.cpp src/commands/OutputSnapshotHashValues.cpp)
target_link_libraries(ACBackup Std++ Std++Static)