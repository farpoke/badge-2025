from __future__ import annotations
import re
import string
import struct
import datetime
import enum
import sys
from dataclasses import dataclass
from pathlib import Path
from textwrap import dedent, wrap

# References:
#
# Microsoft Hardware White Paper, FAT32 File System Specification
# https://www.cs.fsu.edu/~cop4610t/assignments/project3/spec/fatspec.pdf
#
# ELM, FAT Filesystem
# http://elm-chan.org/docs/fat_e.html


BLOCK_COUNT = 256
IMAGE_SIZE = BLOCK_COUNT * 512
FAT_SIZE = BLOCK_COUNT + BLOCK_COUNT // 2
FAT_SIZE_SECTORS = (FAT_SIZE + 511) // 512

assert FAT_SIZE_SECTORS == 1

VOLUME_ID = int(datetime.datetime.now(datetime.UTC).timestamp()) & 0xFFFFFFFF
VOLUME_NAME = 'BADGE'

VALID_SFN_REGEX = re.compile(r'[A-Z0-9_]+(~\d+)? *([A-Z0-9_]{3}|[A-Z0-9_]{2} |[A-Z0-9_] {2}| {3})')

ROOT = Path(__file__).parent
FILES_ROOT = ROOT / 'files'
IMG_OUTPUT_PATH = ROOT / 'disk.img'
CPP_OUTPUT_PATH = ROOT / 'disk.cpp'

END_OF_CHAIN = 0xFFF
""" This value as a FAT12 entry indicates that there is no next cluster 
    (i.e. the current entry is the end of the current chain). """


def assemble_bytes(parts):
    format_string = '<'
    format_data = []
    for head, *tail in parts:
        format_string += head
        format_data.extend(tail)
    data = struct.pack(format_string, *format_data)
    return bytearray(data)


@dataclass
class BootSector:

    media_type: int = 0xF8
    """ Derelict media type enumeration. 0xF8 is "standard value for non-removable disks". """

    drive_number: int = 0x80
    """ Drive number. 0x80 supposedly used for a fixed disk, but OS-dependent. """

    volume_id: int = VOLUME_ID
    """ Volume ID. Used to track media and should be unique "enough". """

    volume_name: str = VOLUME_NAME
    """ Volume name/label. Should match the label stored in the root directory. """

    def to_bytes(self):
        parts = [
            # Common fields:
            ('3B', 0xEB, 0x3C, 0x90),  # Initial short jump + nop instructions. Required but unused.
            ('8s', b'MSDOS5.0'),  # OEM name. Not important, except some drivers require known values.
            ('H', 512),  # Bytes per sector.
            ('B', 1),  # Sectors per cluster.
            ('H', 1),  # Sectors in reserved area.
            ('B', 1),  # Number of FATs. We only have a single copy.
            ('H', 16),  # Number of entries in the root directory (16 entries of 32 bytes = 512 byte sector).
            ('H', BLOCK_COUNT),  # Total number of sectors. Max 2**16-1 for FAT12/16 volume.
            ('B', self.media_type),
            ('H', FAT_SIZE_SECTORS),  # FAT size in sectors.
            ('H', 1),  # Sectors per track. Not relevant.
            ('H', 1),  # Number of heads. Not relevant.
            ('I', 0),  # Number of "hidden" physical sectors between this boot sector and the FAT volume.
            ('I', 0),  # Total number of sectors for FAT32 volumes.
            # FAT12/16 fields:
            ('B', self.drive_number),
            ('x',),  # Reserved.
            ('B', 0x29),  # Extended boot signature, indicating that the subsequent three fields are present.
            ('I', self.volume_id),
            ('11s', self.volume_name.encode('ascii').ljust(11)),
            ('8s', b'FAT12   '),
            # File system type. Does not actually determine the type, but is set for compatibility.
            ('448x',),  # Bootloader code. Filled with zeros when unused.
            ('2B', 0x55, 0xAA),  # Boot sector signature.
        ]
        data = assemble_bytes(parts)
        assert len(data) == 512, f'Packed boot sector data was {len(data)} bytes, expected 512'
        return data


class DirEntryFlags(enum.IntFlag):
    READ_ONLY   = 0x01
    HIDDEN      = 0x02
    SYSTEM      = 0x04
    VOLUME_ID   = 0x08
    DIRECTORY   = 0x10
    ARCHIVE     = 0x20
    LONG_NAME   = 0x0F


@dataclass(kw_only=True)
class DirectoryEntry:
    long_name: str
    """ Full file name. Not stored in this short name entry, but kept around to generate LFN entries. """

    flags: int = 0
    """ Entry type/flags. """

    cluster: int = 0
    """ Lower word of cluster number. Zero means an empty and unallocated file. Must be valid for directories. """

    size: int = 0
    """ File size in bytes. Always zero for directories. """

    disambiguation: int = 1
    """ Number to use in the short file name if needed to disambiguate from other filenames. """

    @property
    def short_name(self) -> str:
        """
        Short file name. Must adhere to FAT 8.3 short name format, and we're extra strict to make the code
        safer and simpler. In proper systems, more complex logic is used (see http://elm-chan.org/docs/fat_e.html).
        """
        name = self.long_name.upper()
        if match := re.fullmatch(r'([A-Z0-9_]{1,8})\.([A-Z0-9_]{1,3})', name):
            prefix, suffix = match.groups()
            return prefix.ljust(8) + suffix.ljust(3)
        elif re.fullmatch(r'[A-Z0-9_]{1,8}', name):
            return name.ljust(11)
        elif match := re.fullmatch(r'([A-Z0-9_]{9,})\.([A-Z0-9_]{1,3})', name):
            prefix, suffix = match.groups()
            number = str(self.disambiguation)
            return prefix[:7 - len(number)] + '~' + number + suffix.ljust(3)
        else:
            prefix = ''.join(ch for ch in name if ch in string.ascii_uppercase + string.digits + '_')
            number = str(self.disambiguation)
            return (prefix[:min(len(prefix), 7 - len(number))] + '~' + number).ljust(11)

    @property
    def short_name_checksum(self) -> int:
        name_bytes = self.short_name.encode('ascii')
        checksum = 0
        for byte in name_bytes:
            checksum = (checksum >> 1) + (checksum << 7) + byte
            checksum &= 0xFF
        return checksum

    def to_bytes(self):
        short_name = self.short_name
        assert len(short_name) == 11 and VALID_SFN_REGEX.fullmatch(short_name), \
            f'Name "{short_name}" not a valid short name'
        name_bytes = short_name.encode('ascii')
        parts = [
            ('11s', name_bytes),
            ('B', self.flags),
            ('B', 0),  # Optional short name case information.
            ('B', 0),  # Optional sub-second value for file creation time.
            ('H', 0),  # File creation time.
            ('H', 0),  # File creation date.
            ('H', 0),  # Last access date.
            ('H', 0),  # Upper word of cluster number. Always zero for FAT12/16.
            ('H', 0),  # File modified time.
            ('H', 0),  # File modified date.
            ('H', self.cluster),
            ('I', self.size),
        ]
        data = assemble_bytes(parts)
        assert len(data) == 32, f'Packed directory entry is {len(data)} bytes, expected 32'
        return data

    def create_lfn_entries(self) -> list[LongFileNameEntry]:
        """
        Generate LongFileNameEntry instances to store the long file name of this directory entry.
        """
        utf16_bytes = self.long_name.encode('utf-16-le')
        checksum = self.short_name_checksum
        n_entries = (len(utf16_bytes) + 25) // 26
        entries = []
        for i in range(n_entries):
            n = i + 1
            if n == n_entries:
                data = utf16_bytes[i * 26:]
                n |= 0x40
            else:
                data = utf16_bytes[i * 26:(i + 1) * 26]
            entries.append(LongFileNameEntry(n, data, checksum))
        return entries


@dataclass
class LongFileNameEntry:
    number: int
    """ Sequence number of this part of the file name (1-20). 
        Additionally, bit 6 (0x40) being set indicates that this is the last entry. """

    data: bytes
    """ The text data contained in this entry. Maximum 13 UTF-16 code points, i.e. 26 bytes. """

    checksum: int
    """ Checksum of the associated short file name. See DirectoryEntry.short_name_checksum. """

    def to_bytes(self):
        data = self.data
        assert len(data) <= 26
        if len(data) < 26:
            data += b'\x00\x00'
            data = data.ljust(26, b'\xFF')
        assert len(data) == 26
        parts = [
            ('B', self.number),
            ('10s', data[:10]),
            ('B', DirEntryFlags.LONG_NAME),
            ('B', 0),
            ('B', self.checksum),
            ('12s', data[10:22]),
            ('H', 0),
            ('4s', data[22:26]),
        ]
        data = assemble_bytes(parts)
        assert len(data) == 32, f'Packed LFN entry is {len(data)} bytes, expected 32'
        return data


class Filesystem:
    boot_sector: BootSector
    table: list[int]
    root_entries: list[DirectoryEntry | LongFileNameEntry]
    data: list[bytes | None]

    def __init__(self):
        self.boot_sector = BootSector()

        self.table = [
            # First two entries are special.
            0xF00 | self.boot_sector.media_type,
            END_OF_CHAIN,
            # Remaining clusters are unused for now.
        ] + [0] * (BLOCK_COUNT - 2)

        self.root_entries = [
            DirectoryEntry(long_name=self.boot_sector.volume_name, flags=DirEntryFlags.VOLUME_ID),
        ]

        self.data = [None] * (BLOCK_COUNT - 3)

    def _claim_clusters(self, n_clusters) -> list[int]:
        assert n_clusters >= 0
        if n_clusters == 0:
            return [0]
        unused = [i for i in range(len(self.table)) if self.table[i] == 0]
        if n_clusters > len(unused):
            raise Exception(f'Out of space; need {n_clusters} cluster(s) but only {len(unused)} free')
        claim = unused[:n_clusters]
        for i in range(n_clusters - 1):
            self.table[claim[i]] = claim[i + 1]
        self.table[claim[-1]] = END_OF_CHAIN
        return claim

    def _encode_fat12(self):
        table_bytes = []
        for hi, lo in zip(self.table[:-1:2], self.table[1::2]):
            assert 0 <= hi <= 0xFFF
            assert 0 <= lo <= 0xFFF
            # Combine two 12-bit values into three little-endian bytes:
            #
            #   HHhhhh LLLLll  -->  hhhh llHH LLLL
            #
            table_bytes.extend([
                hi & 0xFF,
                ((hi >> 8) & 0xF) | ((lo & 0xF) << 4),
                (lo >> 4) & 0xFF,
            ])
        data = bytes(table_bytes)
        assert len(data) == FAT_SIZE, f'Expected FAT data to be {FAT_SIZE} bytes, but is {len(data)}'
        return data

    def add_file(self, name: str):
        path = Path(name).resolve()
        assert path.is_file(), f'Path "{path}" is not a file'
        content = path.read_bytes()

        flags = DirEntryFlags.ARCHIVE

        if path.name == 'hidden.txt':
            flags = DirEntryFlags.HIDDEN | DirEntryFlags.SYSTEM

        n_clusters = (len(content) + 511) // 512
        clusters = self._claim_clusters(n_clusters)

        entry = DirectoryEntry(
            long_name=path.name,
            flags=flags,
            cluster=clusters[0],
            size=len(content)
        )

        lfn_entries = entry.create_lfn_entries()
        self.root_entries.extend(reversed(lfn_entries))
        self.root_entries.append(entry)

        for i, cluster in enumerate(clusters):
            if i + 1 < n_clusters:
                self.data[cluster - 2] = content[i * 512:(i + 1) * 512]
            else:
                self.data[cluster - 2] = content[i * 512:]

        print(f'File {path.relative_to(ROOT, walk_up=True)} ({len(content)} bytes) stored in {len(clusters)} cluster(s)')

    def to_bytes(self):
        image_bytes = self.boot_sector.to_bytes()

        fat = self._encode_fat12()
        fat = fat.ljust(512, b'\0')
        image_bytes += fat

        assert len(self.root_entries) <= 16, f'Too many root entries, {len(self.root_entries)} > 16'
        root_bytes = b''.join(entry.to_bytes() for entry in self.root_entries)
        root_bytes = root_bytes.ljust(512, b'\0')
        image_bytes += root_bytes

        for data in self.data:
            if data is None:
                image_bytes += bytes([0] * 512)
            else:
                assert isinstance(data, bytes) and 1 <= len(data) <= 512
                image_bytes += data.ljust(512, b'\0')

        assert len(image_bytes) == IMAGE_SIZE, f'Final image is {len(image_bytes)} bytes, expected {IMAGE_SIZE} bytes'
        return image_bytes


def format_cpp_file(data: bytes) -> str:
    lines = [
        '#include "disk.hpp"',
        '#include "usb/tusb_config.h"',
        '',
        f'constexpr int DISK_IMAGE_SIZE = {len(data)};',
        'constexpr int DISK_IMAGE_BLOCK_COUNT = DISK_IMAGE_SIZE / USB_MSC_BLOCK_SIZE;',
        'static_assert(DISK_IMAGE_BLOCK_COUNT * USB_MSC_BLOCK_SIZE == DISK_IMAGE_SIZE);',
        '',
        'const uint8_t DISK_IMAGE[DISK_IMAGE_SIZE] = {',
    ] \
    + wrap(', '.join(f'0x{byte:02x}' for byte in data),
           width=100,
           initial_indent='    ',
           subsequent_indent='    ') \
    + [
        '};',
        '',
    ]
    return '\n'.join(lines)


def run():
    fs = Filesystem()

    for arg in sys.argv[1:]:
        fs.add_file(arg)

    image_bytes = fs.to_bytes()

    IMG_OUTPUT_PATH.write_bytes(image_bytes)
    print('Disk image written to', IMG_OUTPUT_PATH)

    cpp_text = format_cpp_file(image_bytes)
    CPP_OUTPUT_PATH.write_text(cpp_text)
    print('Disk .cpp data written to', CPP_OUTPUT_PATH)


if __name__ == '__main__':
    run()
