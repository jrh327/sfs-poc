# sfs-poc #

Proof of concept for a fully-encrypted filesystem in which files and directories can be encrypted individually with different passwords, which can be installed as a computer's filesystem or stored in a standalone file on a host computer to be accessed through an application.

## Requirements ##

### Files ###

- A file shall have a filename
  - A filename shall be UTF-8 encoded
  - A filename shall consist only of printable characters
  - A filename shall be of arbitrary length of 1 <= length <= 255
- A file shall have a length
- A file shall have a `created` date and time
- A file shall have a `modified` date and time
- A file shall have a `last-accessed` date (maybe?)
- A file shall have a parent directory
- A file shall be encrypted using password-based encryption
  - A file's encryption password may be the same as its parent directory
  - A file's encryption password may be different from its parent directory
- A file can be deleted
  - A soft deletion shall simply mark the file as deleted
  - A hard deletion shall zero out all traces of the file on the filesystem

### Directories ###

- A directory is a file
- A directory shall contain directory entries
- A directory shall contain a directory entry pointing to the directory's parent directory
  - The root directory may not contain a parent-directory entry since it does not have a parent directory
  - The parent-directory entry shall be named `..`
- Deleting a directory shall result in the same action (hard or soft deletion) being applied recursively to its contents
- Hard deleting a file in a directory shall result in all directory entries after it being moved up to replace the files' directory entries

### Directory Entries ###

- A directory entry shall be 32 bytes in size
  - 0x00: Reserved
    - This byte shall contain 0x00 for main directory entries
    - For extra filename entries, it shall contain the number of the entry in the group of extra filename entries
    - For soft-deleted files' directory entries, it shall be masked with 0xC0
  - 0x01: Attributes
  - 0x02: Created date (two bytes)
    - Month: four bits
    - Day: five bits
    - Year: seven bits (number of years since 2000, up to 2127)
  - 0x04: Created time (three bytes)
    - Hour: five bits
    - Minute: six bits
    - Second: six bits
    - Millisecond: seven bits (multiple of 10ms)
  - 0x07: Modified date (two bytes)
  - 0x09: Modified time (three bytes)
  - 0x0C: Number of allocation table of first cluster (two bytes)
  - 0x0E: Allocation table entry of first cluster (two bytes)
  - 0x10: File length (four bytes)
  - 0x14: Number of following entries used for filename
  - 0x15: Filename (variable length, UTF-8 encoded)
    - For a file whose name is longer than 11 bytes, the rest of the name shall be stored in the following entries
      - The first byte shall be the number of the entry in the group of extra filename entries, i.e. the first extra entry will have 1 in the first byte, the second extra entry will have 2 in the first byte, etc.
    - Additional bytes when filename does not extend to the end of the directory entry will contain 0x00

### File Allocation ###

- File data shall be stored in data blocks
- Data blocks shall be divided into clusters
- Clusters shall be divided into sectors
- Files shall be allocated using file allocation tables
- A filesystem may have 65535 allocation tables
- Allocation tables shall be stored between data blocks
- Allocation tables shall be one of multiple sizes
  - Small: 2K entries
  - Medium: 4K entries
  - Large: 8K entries
- All allocation tables in a filesystem shall be of the same size
- Allocation table entries shall be 16-bits in size
- The entries shall point to the next cluster of a file in the allocation table's data block
- When the next cluster is located in a different data block, the first entry shall be the cluster number | 0xC000 and the next entry shall contain the number of the correct allocation table

### Encryption ###

- The entire filesystem shall be encrypted
- All allocation tables shall share the password of the root directory
- Individual files may have their own password set for them
  - Directories with their own password shall have their contents encrypted with the same password
- When a file is modified, only the modified sectors are re-encrypted

### Additional Notes ###

All multi-byte values shall be stored in big-endian order
