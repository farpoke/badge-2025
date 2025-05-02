from pathlib import Path
import yaml
import sys
from assets import *

ASSETS_ROOT = Path(__file__).parent

YAML_FILE = ASSETS_ROOT / 'assets.yaml'


class AssetCollection:
    assets: dict[str, AssetBase]
    _fs_items: list[dict[str, object]]

    def __init__(self):
        self.assets = {'disk': FilesystemAsset()}
        self._fs_items = []

    def process_spec(self, assets_spec):

        for spec in assets_spec['files']:
            self._fs_items.append(spec)

        for name, spec in assets_spec['fonts'].items():
            self.assets[name] = FontAsset(name, **spec)

        for name, spec in assets_spec['images'].items():
            self.assets[name] = ImageAsset(name, **spec)

    def all_dependencies(self):
        for item in self._fs_items:
            match item:
                case { 'archive': archive }:
                    for path in archive:
                        yield Path(path).resolve()
                case { 'path': path }:
                    yield Path(path).resolve()
        for item in self.assets.values():
            yield from item.dependencies

    def write_output(self, output_dir: Path):
        assert output_dir.is_dir()

        # noinspection PyTypeChecker
        fs: FilesystemAsset = self.assets['disk']
        for item in self._fs_items:
            fs.add(**item)

        header_lines = [
            '#pragma once',
            '',
            '#include <cstdint>',
            '',
            '#include <lcd/font_data.hpp>',
            '#include <lcd/image.hpp>',
            '',
        ]
        source_lines = [
            '#include <assets.hpp>',
            '',
        ]

        for name, asset in self.assets.items():
            asset_header_lines, asset_source_lines = asset.get_output()

            if len(asset_header_lines) > 0:
                header_lines.append(f'// ===== {name} ===== //')
                header_lines.extend(asset_header_lines)
                header_lines.append('')

            if len(asset_source_lines) > 0:
                source_lines.append(f'// ===== {name} ===== //')
                source_lines.extend(asset_source_lines)
                source_lines.append('')

        header_text = '\n'.join(header_lines)
        source_text = '\n'.join(source_lines)

        header_file = output_dir / 'assets.hpp'
        header_file.write_text(header_text, 'utf-8')
        print('Wrote', len(header_lines), 'lines to', header_file)

        source_file = output_dir / 'assets.cpp'
        source_file.write_text(source_text, 'utf-8')
        print('Wrote', len(source_lines), 'lines to', source_file)


def run():
    only_dependencies = len(sys.argv) == 1

    if not only_dependencies:
        print('Collecting assets...')

    yaml_text = YAML_FILE.read_text('utf-8')
    spec = yaml.safe_load(yaml_text)

    asset_collection = AssetCollection()
    asset_collection.process_spec(spec)

    if only_dependencies:
        # If we didn't get a command line argument, print dependencies instead of writing output.
        for path in asset_collection.all_dependencies():
            print(path.as_posix())

    else:
        # We got one command line argument, which should be the path where we write our output.
        assert len(sys.argv) == 2, 'Unexpected number of command line arguments'
        _, out_dir = sys.argv
        out_dir = Path(out_dir).resolve()
        out_dir.mkdir(parents=True, exist_ok=True)

        asset_collection.write_output(out_dir)

if __name__ == '__main__':
    run()
