import string
import ctypes
import time
from pathlib import Path
import subprocess

kernel32 = ctypes.windll.kernel32


ROOT = Path(__file__).parent
UF2_FILE = (ROOT.parent / 'cmake-build-factorytest' / 'badge-2025.uf2').resolve()

assert UF2_FILE.is_file()


def get_drives():
    # C.f. https://stackoverflow.com/a/827398

    drives = []
    bitmask = kernel32.GetLogicalDrives()
    for letter in string.ascii_uppercase:
        if bitmask & 1:
            drives.append(letter)
        bitmask >>= 1

    return drives


def get_volume_name(drive):
    # C.f. https://stackoverflow.com/a/12056414

    volume_name_buffer = ctypes.create_unicode_buffer(1024)
    file_system_name_buffer = ctypes.create_unicode_buffer(1024)
    serial_number = None
    max_component_length = None
    file_system_flags = None

    rc = kernel32.GetVolumeInformationW(
        ctypes.c_wchar_p(drive + ":\\"),
        volume_name_buffer,
        ctypes.sizeof(volume_name_buffer),
        serial_number,
        max_component_length,
        file_system_flags,
        file_system_name_buffer,
        ctypes.sizeof(file_system_name_buffer)
    )

    return str(volume_name_buffer.value)


def get_rpi_drive():
    drives = get_drives()
    for drive in drives:
        if drive > 'C':
            name = get_volume_name(drive)
            if name == 'RPI-RP2':
                return drive
    return None


def wait_for_drive():
    while True:
        drive = get_rpi_drive()
        if drive is None:
            time.sleep(0.5)
        else:
            return drive


def run():
    while True:

        print('Waiting for RPI-RP2 drive...')
        drive = wait_for_drive()
        print('Found drive:', drive)

        print('Copy firmware...')
        subprocess.run(
            ['copy', str(UF2_FILE), drive + ':\\'],
            shell=True,
            check=True
        )

        print('Waiting for drive to go away...')
        while drive in get_drives():
            time.sleep(0.1)

        print('Drive removed')
        print()


if __name__ == '__main__':
    run()
