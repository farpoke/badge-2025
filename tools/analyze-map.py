#!/usr/bin/env python3
# -*- encoding: utf-8 -*-

import sys
from pathlib import Path
import re
from dataclasses import dataclass, InitVar, field
import subprocess
from collections import defaultdict


COLOR_WHITE = '\033[97m'
COLOR_RESET = '\033[0m'


def pretty_color(x) -> str:
    x = max(0, min(1, x))
    x = x ** 0.5

    points = [
        (0.5, 0.5, 0.5),
        (0.5, 0.7, 0.7),
        (0.0, 0.9, 0.9),
        (0.1, 0.9, 0.1),
        (1.0, 1.0, 0.1),
        (1.0, 0.5, 0.0),
        (1.0, 0.1, 0.1),
    ]

    top = len(points) - 1
    i = min(int(x * top), top - 1)
    k = x * top - i

    r1, g1, b1 = points[i]
    r2, g2, b2 = points[i + 1]

    r = r1 * (1 - k) + r2 * k
    g = g1 * (1 - k) + g2 * k
    b = b1 * (1 - k) + b2 * k

    r = max(0, min(255, int(r * 255)))
    g = max(0, min(255, int(g * 255)))
    b = max(0, min(255, int(b * 255)))

    return f'\033[38;2;{r};{g};{b}m'


def color_ljust(text: str, width: int, fill: str = ' '):
    assert len(fill) == 1
    uncolored_text = re.sub(r'\033[^m]+m', '', text)
    uncolored_width = len(uncolored_text)
    if uncolored_width < width:
        text += fill * (width - uncolored_width)
    return text


CPPFILT_PATH = r"C:\Users\artemis\.pico-sdk\toolchain\14_2_Rel1\bin\arm-none-eabi-c++filt.exe"

_demangle_cache = {}

def demangle(name: str):
   global _demangle_cache
   if name in _demangle_cache:
       return _demangle_cache[name]

   process = subprocess.run(
       [CPPFILT_PATH, name],
       stdout=subprocess.PIPE,
       stderr=subprocess.PIPE,
       stdin=None,
       encoding='utf-8'
   )
   if process.returncode != 0:
       raise Exception(f'c++filt failed: {process.stderr}')

   result = process.stdout.strip()
   _demangle_cache[name] = result

   return result


def pretty_size(n: int):
    if n < 500:
        return f"{n:4} B "

    n /= 1024

    if float.is_integer(n) and n < 1000:
        return f"{n:4.0f} KB"
    elif n < 10:
        return f"{n:4.1f} KB"
    elif n < 500:
        return f"{n:4.0f} KB"

    n /= 1024

    if float.is_integer(n):
        return f"{n:4.0f} MB"
    elif n < 10:
        return f"{n:4.2f} MB"
    else:
        return f"{n:4.0f} MB"


@dataclass
class MemoryRegion:
    name: str
    origin: int
    length: int

    attributes: InitVar[str]

    exec: bool = field(init=False)
    read: bool = field(init=False)
    write: bool = field(init=False)

    def __post_init__(self, attributes: str):
        self.exec = 'x' in attributes
        self.read = 'r' in attributes
        self.write = 'w' in attributes

    def __str__(self):
        attrs = ('x' if self.exec else '.') + ('r' if self.read else '.') + ('w' if self.write else '.')
        return f'{self.name:12} 0x{self.origin:08x} 0x{self.length:08x} {attrs}  // {pretty_size(self.length)}'


@dataclass
class LinkRegion:
    name: str
    origin: int
    length: int

    load_address: int | None = None

    sections: list['LinkSection'] = field(default_factory=list)


@dataclass
class LinkSection:
    name: str
    origin: int
    length: int
    object: str

    symbols: list['LinkSymbol'] = field(default_factory=list)

    @property
    def demangled_name(self):
        demangled_parts = [demangle(part) for part in self.name.split('.')]
        return '.'.join(demangled_parts)


@dataclass
class LinkSymbol:
    name: str
    origin: int
    size: int | None = None


class ParserBase:
    _lines: list[str]
    _idx: int

    @property
    def line(self):
        if 0 <= self._idx < len(self._lines):
            return self._lines[self._idx]
        else:
            return None

    @property
    def line_number(self):
        return self._idx + 1 + self.line_number_offset

    def __init__(self, lines: list[str], line_number_offset: int = 0):
        self._lines = list(lines)
        self._idx = -1
        self.line_number_offset = line_number_offset

    def __len__(self):
        return len(self._lines)

    def __getitem__(self, item):
        return self._lines[item]

    def next(self):
        self._idx += 1
        return self.line

    def find(self, line):
        for i in range(len(self)):
            if self[i] == line:
                return i
        return None

    def tell(self):
        return self._idx

    def seek(self, idx):
        self._idx = idx

    def run(self):
        raise NotImplementedError()


class MemoryConfigParser(ParserBase):
    ROW_REGEX = re.compile(r'(\S+)\s+(\w+)\s+(\w+)\s*(\w*)$')

    regions: list[MemoryRegion]

    def run(self):
        # Advance to table header.
        while not self.next().startswith('Name'):
            pass

        # Parse table rows.
        self.regions = []
        while len(self.next()) > 0:
            match = self.ROW_REGEX.match(self.line)
            assert match is not None
            name = match.group(1)
            origin = int(match.group(2), 16)
            length = int(match.group(3), 16)
            attr_text = match.group(4)

            if name == '*default*':
                continue

            region = MemoryRegion(name, origin, length, attr_text)
            self.regions.append(region)


class MapParser(ParserBase):
    LINE_REGEX = re.compile(r'''
        # ===== REGION =====
        (?= .{15} \s 0x[0-9a-f]{8,16} \s [0-9a-fx ]{9} [0-9a-f] )  # Lookahead to verify line structure.
        (?P<region_name> \S+ )? \s+
        (?P<region_origin> 0x[0-9a-f]{8,16} ) \s+
        (?P<region_size> 0x[0-9a-f]+ )
        (?: \ load\ address\  (?P<load_address> 0x[0-9a-f]+ ) )?
        $
        |
        # ===== SECTION =====
        (?= \s .{14} \s 0x[0-9a-f]{8,16} \s [0-9a-fx ]{9} [0-9a-f] \s \S .* $ )  # Lookahead to verify line structure.
        \s (?P<section_name> \S+ )? \s+
        (?P<section_origin> 0x[0-9a-f]{8,16} ) \s+
        (?P<section_size> 0x[0-9a-f]+ ) \s
        (?P<section_object> \S+ )
        |
        # ===== LONG REGION NAME =====
        (?P<long_region_name> \S{14,} $ )
        |
        # ===== LONG SECTION NAME =====
        \s (?P<long_section_name> \S{14,} $ )
        |
        # ===== SYMBOL =====
        \s{16}
        (?P<symbol_origin> 0x[0-9a-f]{8,16} )
        \s{16,}
        (?P<symbol_name> \S .+ $ )
    ''', re.VERBOSE)

    regions: list[LinkRegion]

    def run(self):
        # Skip header line
        self.next()

        self.regions = []
        current_region: LinkRegion | None = None
        current_section: LinkSection | None = None

        n = 0
        while (line := self.next()) is not None:

            # Skip empty lines.
            if len(line) == 0:
                continue

            # Skip load/group lines.
            if line.startswith('LOAD') or line in ('START GROUP', 'END GROUP'):
                continue

            # Skip wildcards.
            if line.startswith(' *'):
                continue

            # Skip special lines.
            if line.endswith('(size before relaxing)') or line.strip().startswith('[!provide]'):
                continue

            # Ignore trailing discarded information.
            if line == '/DISCARD/':
                break

            # Match map line.
            match = self.LINE_REGEX.match(line)
            if match is None:
                print(f'Line {self.line_number} did not match: "{line}"')
                continue

            region_name = match.group('region_name')
            section_name = match.group('section_name')

            long_region_name = match.group('long_region_name')
            long_section_name = match.group('long_section_name')

            if long_region_name is not None:
                region_name = long_region_name
                match = self.LINE_REGEX.match(self.next())
                assert match.group('region_origin') is not None

            if long_section_name is not None:
                section_name = long_section_name
                match = self.LINE_REGEX.match(self.next())
                assert match.group('section_origin') is not None

            if region_name is not None:
                region_origin = int(match.group('region_origin'), 16)
                region_size = int(match.group('region_size'), 16)
                load_address = match.group('load_address')

                current_region = LinkRegion(region_name, region_origin, region_size)
                if load_address is not None:
                    current_region.load_address = int(load_address, 16)

                self.regions.append(current_region)

                current_section = None

            elif section_name is not None:
                section_origin = int(match.group('section_origin'), 16)
                section_size = int(match.group('section_size'), 16)
                section_object = match.group('section_object')

                current_section = LinkSection(section_name, section_origin, section_size, section_object)
                current_region.sections.append(current_section)

            else:

                # If we don't have a section, this is a variable, alignment, or other something we'll ignore.
                if current_section is None:
                    continue

                symbol_name = match.group('symbol_name')
                symbol_origin = int(match.group('symbol_origin'), 16)

                # Skip symbols that just mess up the mapping logic.
                if symbol_name.startswith('ASSERT') or re.match(r'^[A-Za-z_]+\s*=', symbol_name) is not None:
                    continue

                symbol = LinkSymbol(symbol_name, symbol_origin)
                current_section.symbols.append(symbol)


class CrossReferenceParser(ParserBase):

    def run(self):
        pass


class TreeNode:
    printed_space: bool = False

    def __init__(self, name):
        self.name: str = name
        self.size: int = 0
        self.nodes: dict[str, TreeNode] = {}

    @property
    def appropriate_tree_width(self):
        if len(self.nodes) == 1:
            return next(iter(self.nodes.values())).appropriate_tree_width + 1
        return max((child.appropriate_tree_width for child in self.nodes.values()), default=0) + 3

    def insert(self, path_: list[str], size: int):
        self.size += size

        if len(path_) > 0:
            head, tail = path_[0], path_[1:]
            if head not in self.nodes:
                self.nodes[head] = TreeNode(head)
            self.nodes[head].insert(tail, size)

    def apply_min_size(self, min_size: int):
        child_names = list(self.nodes.keys())
        for name in child_names:
            if self.nodes[name].size < min_size:
                del self.nodes[name]
            else:
                self.nodes[name].apply_min_size(min_size)

    def _print(self, *,
               indent: str,
               hang_indent: str,
               name_prefix: tuple[str, ...],
               total_size: int,
               compact: bool,
               tree_width: int,
              ):

        children: list[TreeNode] = list(self.nodes.values())

        path_tuple = name_prefix + (self.name, )

        pct = 100 * self.size / total_size
        clr = pretty_color(pct / 100)

        if len(children) == 1:
            children[0]._print(
                indent=indent + clr + '-',
                hang_indent=hang_indent + ' ',
                name_prefix=path_tuple,
                total_size=total_size,
                compact=compact,
                tree_width=tree_width,
            )
            return

        print_name = '/'.join(path_tuple)
        if len(print_name) > 40 and len(path_tuple) > 2:
            print_name = '.../' + '/'.join(path_tuple[-2:])

        if len(indent) == 0:
            this_indent = ' ' * tree_width + clr
        elif len(children) == 0:
            this_indent = indent + clr + '─'
        else:
            this_indent = indent + '─' + clr + '┬─'

        this_indent = color_ljust(this_indent, tree_width, '─')

        print(f'{this_indent} {pretty_size(self.size)}  {pct:4.1f}%  {print_name}{COLOR_RESET}')
        TreeNode.printed_space = False

        children.sort(key=lambda n: n.size, reverse=True)
        for child in children:

            if len(indent) == 0:
                new_indent = clr + '─'
                new_hang_indent = ' '
            elif child is not children[-1]:
                new_indent = hang_indent + clr + ' ├─'
                new_hang_indent = hang_indent + clr + ' │ '
            else:
                new_indent = hang_indent + clr + ' └─'
                new_hang_indent = hang_indent + '   '

            if not compact and child is children[0] and len(children) > 1:
                print(new_hang_indent)
                TreeNode.printed_space = True

            child._print(
                indent=new_indent,
                hang_indent=new_hang_indent,
                name_prefix=(),
                total_size=total_size,
                compact=compact,
                tree_width=tree_width,
            )

        if not compact and len(children) > 0 and not TreeNode.printed_space:
            print(hang_indent)
            TreeNode.printed_space = True

    def print(self, *,
              total_size: int,
              compact: bool,
              ):
        self._print(
            indent='',
            hang_indent='',
            name_prefix=(),
            total_size=total_size or self.size,
            compact=compact,
            tree_width=self.appropriate_tree_width - 3
        )


class CombinedParser(ParserBase):
    memory_regions: list[MemoryRegion]
    link_regions: list[LinkRegion]

    flash_start: int
    flash_end: int

    ram_start: int
    ram_end: int

    def run(self):
        # Find indices of headers, to split into different parts.
        i1 = self.find('Memory Configuration')
        i2 = self.find('Linker script and memory map')
        i3 = self.find('Cross Reference Table')

        # Create sub-parsers, giving them the different section lines.
        memory_parser = MemoryConfigParser(self[i1:i2], line_number_offset=i1)
        map_parser = MapParser(self[i2:i3], line_number_offset=i2)
        cref_parser = CrossReferenceParser(self[i3:], line_number_offset=i3)

        # Run sub-parsers.

        try:
            memory_parser.run()
            self.memory_regions = memory_parser.regions
            print('Parsed', len(self.memory_regions), 'memory regions')
        except Exception as err:
            print()
            print('! Memory parser error !')
            print(err)
            print(f'Line {memory_parser.line_number}: "{memory_parser.line}"')

        try:
            map_parser.run()
            self.link_regions = map_parser.regions
            print('Parsed', len(self.link_regions), 'link regions')
        except Exception as err:
            print()
            print('! Map parser error !')
            print(err)
            print(f'Line {map_parser.line_number}: "{map_parser.line}"')

        try:
            cref_parser.run()
        except Exception as err:
            print()
            print('! Cref parser error !')
            print(err)
            print(f'Line {cref_parser.line_number}: "{cref_parser.line}"')

        # Pick out FLASH start and end for later use.
        for mem_region in self.memory_regions:
            if mem_region.name == 'FLASH':
                self.flash_start = mem_region.origin
                self.flash_end = mem_region.origin + mem_region.length
                break
        else:
            raise Exception('Did not find FLASH region')

        # Pick out RAM start and end for later use.
        for mem_region in self.memory_regions:
            if mem_region.name == 'RAM':
                self.ram_start = mem_region.origin
                self.ram_end = mem_region.origin + mem_region.length
                break
        else:
            raise Exception('Did not find RAM region')

        # Attempt to figure out symbol sizes.
        self.guess_symbol_sizes()

    def guess_symbol_sizes(self):
        for region in self.link_regions:
            for section in region.sections:
                # Work backwards from the end and assume each symbol takes up all intervening space.
                prev_origin = section.origin + section.length
                for symbol in reversed(section.symbols):
                    symbol.size = prev_origin - symbol.origin
                    prev_origin = symbol.origin


    def print_memory_regions(self):
        print()
        print('══════════════ MEMORY REGIONS ══════════════')
        for region in self.memory_regions:
            print(region)

    def print_link_regions(self):
        print()
        print('══════════════ LINK REGIONS ══════════════')
        for region in self.link_regions:
            if region.length == 0:
                continue
            print(f'{region.name:20} 0x{region.origin:08x}   {pretty_size(region.length)}')


    def print_largest_sections(self, min_size: int = 1024):
        print()
        print('══════════════ LARGEST SECTIONS ══════════════')

        # Collect all sections and sort by size.
        sections: list[LinkSection] = [
            section
            for region in self.link_regions
            for section in region.sections
        ]
        sections.sort(key=lambda x: x.length, reverse=True)

        # Split the largest ones into groups by region.
        grouped_sections = defaultdict(list)
        for section in sections:
            name_parts = section.demangled_name.split('.')
            prefix = name_parts[1]
            if section.length < min_size:
                if prefix in grouped_sections:
                    grouped_sections[prefix].append(None)
                break
            grouped_sections[prefix].append(section)

        # Print the groups.
        for group_name, group_sections in grouped_sections.items():
            print('-----', group_name, '-----')
            for section in group_sections:
                if section is None:
                    print('...')
                    break
                print(pretty_size(section.length), section.demangled_name)

    def print_largest_objects(self, min_size: int = 2048):
        print()
        print('══════════════ LARGEST OBJECTS ══════════════')

        object_sizes = defaultdict(int)

        for region in self.link_regions:
            for section in region.sections:
                object_sizes[section.object] += section.length

        object_sizes = list(object_sizes.items())
        object_sizes.sort(key=lambda x: x[1], reverse=True)

        for obj, size in object_sizes:
            if size < min_size:
                print('...')
                break

            print(pretty_size(size), obj)

    def _print_object_hierarchy(self, title: str, region_start: int, region_end: int, min_size: int):
        print()
        print(f'{COLOR_WHITE}══════════════ {title} OBJECT HIERARCHY ══════════════{COLOR_RESET}')
        print(f'{pretty_color(0)}(Min size: {pretty_size(min_size).strip()}){COLOR_RESET}')
        print(f'{pretty_color(0)}(0x{region_start:08x} .. 0x{region_end:08x}){COLOR_RESET}')

        root = TreeNode('*')

        last_end = region_start

        for region in self.link_regions:
            # if region.name == '.bss':
            #     continue

            load_in_region = region.load_address is not None and region_start <= region.load_address < region_end
            origin_in_region = region.origin is not None and region_start <= region.origin < region_end
            if not (load_in_region or origin_in_region):
                continue

            for section in region.sections:

                section_path = Path(section.object).resolve()

                if section_path.is_relative_to(Path.cwd()):
                    section_path = section_path.relative_to(Path.cwd())
                for idx, part in enumerate(section_path.parts):
                    if part == 'CMakeFiles':
                        cmake_path = Path(*section_path.parts[:idx + 2])
                        section_path = section_path.relative_to(cmake_path)
                        break
                for trim_part in ('.pico-sdk', 'arm-none-eabi'):
                    for idx, part in enumerate(section_path.parts):
                        if part == trim_part:
                            base_path = Path(*section_path.parts[:idx + 1])
                            section_path = Path(f'({part})') / section_path.relative_to(base_path)
                            break

                if len(section.symbols) == 0:
                    assert section.length >= 0, f'Section {section.name} length = {section.length}'
                    root.insert(section_path.parts, section.length)
                    end = section.origin + section.length
                    if region.load_address:
                        end = region.load_address + end - region.origin
                    last_end = max(last_end, end)
                else:
                    for symbol in section.symbols:
                        assert symbol.size >= 0, f'Symbol {symbol.name} length = {symbol.size}'
                        symbol_path = section_path / symbol.name
                        root.insert(symbol_path.parts, symbol.size)
                        end = symbol.origin + symbol.size
                        if region.load_address:
                            end = region.load_address + end - region.origin
                        last_end = max(last_end, end)

        used_space = last_end - region_end
        if used_space > root.size:
            root.insert(['(padding)'], used_space - root.size)

        region_size = region_end - region_start
        free_space = region_size - root.size
        if free_space > 0:
            print(f'{pretty_size(free_space)} of free space')
        elif free_space < 0:
            print(f'{pretty_size(-free_space)} of space missing')

        root.name = f'* ({root.size} bytes)'
        root.apply_min_size(min_size)
        root.print(
            total_size=region_end - region_start,
            compact=True,
        )

    def print_flash_object_hierarchy(self):
        self._print_object_hierarchy('FLASH', self.flash_start, self.flash_end, 2048)

    def print_ram_object_hierarchy(self):
        self._print_object_hierarchy('RAM', self.ram_start, self.ram_end, 1024)

    def print_report(self):
        self.print_memory_regions()
        self.print_link_regions()
        # self.print_largest_sections()
        # self.print_largest_objects()
        self.print_flash_object_hierarchy()
        self.print_ram_object_hierarchy()


def run(map_path: Path):
    if not map_path.exists():
        print(map_path, 'does not exist')
        exit(1)

    lines = map_path.read_text().splitlines()
    print('Read', len(lines), 'lines from', map_path)

    print('Parsing...')
    parser = CombinedParser(lines)
    parser.run()

    parser.print_report()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Expect exactly one argument, a path to a linked map file.')
        exit(1)
    run(Path(sys.argv[1]))
