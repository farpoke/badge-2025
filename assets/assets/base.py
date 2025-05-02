from pathlib import Path


class AssetBase:
    dependencies: list[Path]

    def __init__(self):
        self.dependencies = []

    def get_output(self) -> tuple[list[str], list[str]]:
        raise NotImplementedError()

    @staticmethod
    def format_data_array(
            *,
            name: str,
            data: bytes | bytearray | list[int],
            data_type: str = 'uint8_t',
            data_bits: int = 8,
            line_size: int = 64,
            indent: int = 0):

        n = len(data)
        indent_str = ' ' * indent

        lines = [indent_str + f'const {data_type} {name}[{n}] = {{']

        mask = (1 << data_bits) - 1
        assert all((item & mask) == item for item in data), f'Data has value(s) that does not fit in {data_bits} bits'

        nibbles = (data_bits + 3) // 4
        item_format = f'0x{{:0{nibbles}x}}, '
        for i0 in range(0, len(data), line_size):
            line_data = data[i0:min(n, i0 + line_size)]
            line = ''.join(item_format.format(item) for item in line_data)
            lines.append(indent_str + '    ' + line)

        lines.append(indent_str + '};')

        return lines
