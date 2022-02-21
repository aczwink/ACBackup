cmake_minimum_required(VERSION 3.6)
project(ACBackup)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 20)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

set(SRC_FILES_SHARED
	src/backup/BackupNodeAttributes.cpp
	src/backup/BackupNodeAttributes.hpp
	src/backup/BackupNodeIndex.cpp
	src/backup/BackupNodeIndex.hpp
	src/backup/Snapshot.cpp
	src/backup/Snapshot.hpp
	src/backup/SnapshotManager.cpp
	src/backup/SnapshotManager.hpp
	src/backup/VirtualSnapshotFilesystem.cpp
	src/backup/VirtualSnapshotFilesystem.hpp

	src/backupfilesystem/FlatVolumesBlockInputStream.cpp
	src/backupfilesystem/FlatVolumesBlockInputStream.hpp
	src/backupfilesystem/FlatVolumesFileSystem.cpp
	src/backupfilesystem/FlatVolumesFileSystem.hpp
	src/backupfilesystem/VolumesOutputStream.cpp
	src/backupfilesystem/VolumesOutputStream.hpp

	src/commands/AddSnapshot.cpp
	src/commands/Init.cpp

	src/config/CompressionStatistics.cpp
	src/config/CompressionStatistics.hpp
	src/config/ConfigManager.cpp
	src/config/ConfigManager.hpp

	src/indexing/Filtering/AppleDesktopServicesStoreFilter.cpp
	src/indexing/Filtering/AppleDesktopServicesStoreFilter.hpp
	src/indexing/Filtering/AppleDoubleFilter.cpp
	src/indexing/Filtering/AppleDoubleFilter.hpp
	src/indexing/Filtering/DesktopIniFilter.cpp
	src/indexing/Filtering/DesktopIniFilter.hpp
	src/indexing/Filtering/ThumbsDbFilter.cpp
	src/indexing/Filtering/ThumbsDbFilter.hpp

	src/indexing/FileSystemNodeIndex.cpp
	src/indexing/FileSystemNodeIndex.hpp
	src/indexing/OSFileSystemNodeIndex.cpp
	src/indexing/OSFileSystemNodeIndex.hpp

	src/status/StatusTrackingOutputStream.cpp
	src/status/StatusTrackingOutputStream.hpp

	src/NodeIndexDifferenceResolver.cpp
	src/NodeIndexDifferenceResolver.hpp
	src/Util.cpp
	src/Util.hpp
	)

add_executable(ACBackup ${SRC_FILES_SHARED} src/main.cpp src/commands/Commands.hpp src/InjectionContainer.hpp src/status/StatusTracker.hpp src/status/StatusTracker.cpp src/status/TerminalStatusTracker.hpp src/config/Config.hpp src/status/ProcessStatus.hpp src/config/ConfigException.hpp src/indexing/FileSystemNodeAttributes.hpp src/status/TerminalStatusTracker.cpp src/status/ProcessStatus.cpp src/commands/VerifySnapshot.cpp src/backupfilesystem/FlatVolumesFile.hpp src/backupfilesystem/FlatVolumesFile.cpp src/backupfilesystem/FlatVolumesDirectory.hpp src/backupfilesystem/FlatVolumesDirectory.cpp src/Serialization.hpp src/status/WebStatusTracker.hpp src/status/WebStatusTracker.cpp src/status/StatusTrackerWebService.hpp src/status/StatusTrackerWebService.cpp src/status/webresources.hpp src/indexing/LinkPointsOutOfIndexDirException.hpp src/backupfilesystem/FlatVolumesLink.hpp src/backupfilesystem/FlatVolumesLink.cpp src/CompressionSetting.hpp src/commands/Diff.cpp src/commands/OutputSnapshotStats.cpp src/commands/OutputSnapshotHashValues.cpp src/StreamPipingFailedException.hpp src/indexing/Filtering/FileFilter.hpp)
target_link_libraries(ACBackup Std++ Std++Static)

add_executable(ACBackupViewer ${SRC_FILES_SHARED} src_viewer/main.cpp src_viewer/Nodes.hpp src_viewer/Nodes.cpp src_viewer/DataFileTreeNode.hpp src_viewer/DataFileTreeNode.cpp src_viewer/FileRevisionNode.hpp)
target_link_libraries(ACBackupViewer Std++ Std++Static)

add_executable(tests_ACBackup ${SRC_FILES_SHARED} src_tests/IntegrationTests/SnapshotManagerTests.cpp src_tests/IntegrationTests/TestBackupCreator.hpp src_tests/IntegrationTests/FileFilteringTests.cpp)
target_link_libraries(tests_ACBackup Std++ Std++Static Std++Test)


install (TARGETS ACBackup RUNTIME DESTINATION bin)
