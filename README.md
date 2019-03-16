# ACBackup
Incremental backup solution with snapshots/compression/encryption

## Usage

### Creating a snapshot
Issue the command "add-snapshot sourceDir", which will read in the directory identified by "sourceDir" and backup all files that have changed or are new since the last snapshot.
The newest snapshot contains only the data of the newly added or changed files. Files that haven't changed are referenced to an older snapshot.
Files that were deleted in the source directory, are unlinked. I.e. their data remains in an older snapshot, but the newly created snapshot has no reference to that file.

ACBackup uses LZMA compression to shrink the files, which can reduce the size of the backup significantly.
However, LZMA is computationally heavy and thus increases compression time and memory requirements drastically.
ACBackup tries to "learn" how good your files compress and uses that to decide whether it is worthy to actually compress them and with which compression level.
You can specify the maximum compression level that should be used with the "-c 0...9" switch - the default value is 3.
A higher number means more compression rate but also higher computation cost.
I don't recommend using anything higher than 5, because the compression throughput will vanish while not actually achieving much more compression rate (law of diminishing returns).
Check for instance https://www.rootusers.com/gzip-vs-bzip2-vs-xz-performance-comparison/ on performance of the levels (the xz column is for LZMA).
You can also disable compression totally with the "-no-c" switch.

When you have a multicore machine, several files can be processed in parallel.
However, only one file can by design be written to the snapshot container at a time. Still other files can be processed in memory and then be written all at once to the container.
Obviously, this does only work if the filtered data actually fits in memory.
You can specify the maximum file size in MiB to be handled in memory with the "-l size" switch.
If a files raw size (unfiltered i.e. uncompressed) is less then the specified size, the file is processed in memory and then written to the container all at once.
If it is bigger, then the file can not be processed in memory and thus must be streamed to the snapshot container (i.e. chunk by chunk).
Be careful that ACBackup uses as much threads as your processor has logical cores.
Thus if you have 8 (logical) cores and specified "-l 1024", ACBackup may take up to 8 GiB (actually 7) of memory for prefiltering.
Don't specify too high numbers here and always leave some space for the rest of the machine. Note that ACBackup obviously also needs some other memory apart for filtering.

#### Recommended settings
Here are settings that I recommend for certain hardware. I assume here that ACBackup is virtually the only (heavy) process running on the machine.
| Machine | -c | -l |
| ------- | ------------- |
| Raspberry Pi 3 Model B+ (4 cores, 1 GiB RAM) | 3 | 160 |