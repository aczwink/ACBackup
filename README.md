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
| ---------- | -- | -- |
| Raspberry Pi 3 Model B+ (4 cores, 1 GiB RAM) | 3 | 160 |

## Cryptographic analysis
ACBackup uses three types of keys: a master key, snapshot keys and an application key.
As ACBackup uses AES-256 for encryption, all keys are 32 bytes long.

The master key is the central and most important key and is unique per backup directory.
It is generated from a user-supplied password, 192 bytes of random salt and 128 bytes of pepper that are static to the application.
The password is something only the user knows, and the master salt is something that only the user owns.
The master key is derived with the scrypt algorithm with a cost factor of 20.
As salt to scrpyt the master salt is taken and concatenated with the master pepper.
The master password is encoded as UTF-8.

The snapshot keys are derived from the master key and are used for encrypting the snapshot files.
The keys for index and data files are not the same.
They are derived with HKDF-SHA512, a 128 bytes random salt (static for the backup directory) and the snapshot file name as info.
The index file is encoded in CBC mode, while the data file is encoded in CTR mode.
I.e. all source files (data) are encrypted with the same key within a snapshot but the initial value (16 bytes) is chosen randomly for each file. 

The application key is static for the whole application (and will never change).
It is only there to protect the master salt - i.e. even if the master salt is stolen, you still have to know the application key and the master pepper in order to derive the master key.
This is actually not really a protection since the information for deriving the application key can be obtained from the source code. 
The only advantage is that the software is quite unknown and thus also the application key.

When initializing an encrypted backup dir a file called "master_key" is created.
It contains the master salt encrypted with the application key, the snapshot key salt encrypted with the master key and a random verification message also encrypted with the master key.
The master salt is the information with high entropy in comparison with your password.
If your password is compromised, thats bad, but worse when your salt is stolen.
Never keep the master_key file inside the backup directory, move it in when you need to encrypt/decrypt something and when you're done move it out again.
And move it to a safe location like a USB stick or so.
Never keep it somewhere where you have a network connection with an unsafe network (i.e. Internet).
Be also sure to keep your master_key file safe.
Store it to several locations.
If you loose your master_key file or if it ever gets corrupted - your files are lost for good - you can NEVER restore them.
