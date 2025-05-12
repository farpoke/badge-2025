import math
from pathlib import Path
from PIL import Image, GifImagePlugin

from .base import AssetBase

GifImagePlugin.LOADING_STRATEGY = GifImagePlugin.LoadingStrategy.RGB_ALWAYS


TARGET_SIZE = 160, 128


class AnimationCompressor:

    MIN_SAME = 4
    RUN_BITS = 7
    MAX_RUN = 1 << RUN_BITS

    def __init__(self):
        self._temp_value = 0
        self._temp_bits = 0
        self.data = bytearray()
        self.bpp = None
        self.prev_frame = None
        self.same_lengths = []
        self.diff_lengths = []

    def _write_bits(self, value: int, n_bits: int):
        mask = (1 << n_bits) - 1
        assert (value & ~mask) == 0, f'Value {value} does not fit in {n_bits} bits'
        self._temp_value |= (value & mask) << self._temp_bits
        self._temp_bits += n_bits
        while self._temp_bits >= 8:
            self.data.append(self._temp_value & 0xFF)
            self._temp_value >>= 8
            self._temp_bits -= 8

    def flush(self):
        if self._temp_bits > 0:
            self.data.append(self._temp_value & 0xFF)
            self._temp_value = 0
            self._temp_bits = 0

    def _write_byte(self, value: int):
        assert 0 <= value < 256
        self.flush()
        self.data.append(value & 0xFF)

    def _write_bytes(self, data: bytes | bytearray):
        self.flush()
        self.data.extend(data)

    def write_header(self, n_frames, interval, bpp, palette):
        assert 0 < n_frames < 256
        assert 0 < interval < 256
        assert 0 < bpp < 8
        assert len(palette) == 3 * 2 ** bpp

        self.bpp = bpp

        self._write_byte(n_frames)
        self._write_byte(interval)
        self._write_byte(bpp)
        self._write_bytes(palette)

    def _compare_vs_prev(self, data, offset):
        same = 0
        while offset + same < len(data) and self.prev_frame[offset + same] == data[offset + same] and same < self.MAX_RUN:
            same += 1

        different = 0
        while offset + different < len(data) and self.prev_frame[offset + different] != data[offset + different] and different < self.MAX_RUN:
            different += 1

        return same, different

    def _pick_mode(self, data, offset) -> tuple[bool, int]:
        same, diff = self._compare_vs_prev(data, offset)
        if same >= self.MIN_SAME:
            return True, same
        elif (len(self.prev_frame) - offset) < self.MIN_SAME:
            return False, len(self.prev_frame) - offset

        diff = 1
        while offset + diff < len(data) and diff < self.MAX_RUN:
            same, _ = self._compare_vs_prev(data, offset + diff)
            if same >= self.MIN_SAME:
                break
            diff += 1

        return False, diff

    def write_frame(self, data):
        assert len(data) == TARGET_SIZE[0] * TARGET_SIZE[1]

        if self.prev_frame is None:
            for pixel in data:
                self._write_bits(pixel, self.bpp)
            self.flush()

        else:
            assert len(data) == len(self.prev_frame)

            idx = 0
            while idx < len(data):
                same, n = self._pick_mode(data, idx)
                if same:
                    assert self.MIN_SAME <= n <= self.MAX_RUN
                    self.same_lengths.append(n)
                    self._write_bits(0, 1)
                    self._write_bits(n % self.MAX_RUN, self.RUN_BITS)
                else:
                    self.diff_lengths.append(n)
                    self._write_bits(1, 1)
                    self._write_bits(n % self.MAX_RUN, self.RUN_BITS)
                    for i in range(n):
                        self._write_bits(data[idx + i], self.bpp)
                idx += n
            self.flush()

            assert idx == len(data)

        self.prev_frame = data


class AnimationDecompressor:

    def __init__(self, data):
        self.data = data

        self._data_index = 0
        self._temp_value = 0
        self._temp_bits = 0

        self.n_frames = None
        self.interval = None
        self.bpp = None
        self.palette = None
        self.frame = None

        self._read_header()

    def _read_bits(self, n_bits):
        if self._temp_bits < n_bits:
            self._temp_value |= self.data[self._data_index] << self._temp_bits
            self._temp_bits += 8
            self._data_index += 1
        result = self._temp_value & ((1 << n_bits) - 1)
        self._temp_value >>= n_bits
        self._temp_bits -= n_bits
        return result

    def _flush(self):
        self._temp_value = 0
        self._temp_bits = 0

    def _read_byte(self):
        value = self.data[self._data_index]
        self._data_index += 1
        return value

    def _read_bytes(self, n):
        value = self.data[self._data_index:self._data_index + n]
        self._data_index += n
        return value

    def _read_header(self):
        self.n_frames = self._read_byte()
        self.interval = self._read_byte()
        self.bpp = self._read_byte()
        self.palette = self._read_bytes(3 * 2 ** self.bpp)

    def read_frame(self):
        if self.frame is None:
            self.frame = []
            for i in range(TARGET_SIZE[0] * TARGET_SIZE[1]):
                pixel = self._read_bits(self.bpp)
                self.frame.append(pixel)
            self._flush()
        else:
            idx = 0
            while idx < TARGET_SIZE[0] * TARGET_SIZE[1]:
                diff = self._read_bits(1)
                run = self._read_bits(AnimationCompressor.RUN_BITS)
                if run == 0:
                    run = AnimationCompressor.MAX_RUN
                if diff:
                    for i in range(run):
                        pixel = self._read_bits(self.bpp)
                        self.frame[idx + i] = pixel
                idx += run
            self._flush()
            assert idx == TARGET_SIZE[0] * TARGET_SIZE[1]
        return self.frame

    def eof(self):
        return self._data_index == len(self.data)


def measure_entropy(values):
    n = len(values)
    counts = {}
    for value in values:
        counts[value] = counts.get(value, 0) + 1
    return -sum(k / n * math.log2(k / n) for k in counts.values())


class AnimationAsset(AssetBase):

    def __init__(self, name: str, *, path: str, bpp: int = 4, trim=None, rotate: int = 0, frames: int = 0,
                 preview: str | None = None, decimate: int | None = None):
        super().__init__()

        self.name = name
        self.path = Path(path).resolve()
        self.bpp = bpp
        self.trim = trim
        self.rotate = rotate
        self.frames = frames
        self.decimate = decimate
        self.preview = None if preview is None else Path(preview).resolve()

        self.dependencies.append(self.path)

    def get_output(self):
        print(f'- Animation asset {self.name}, path={self.path.as_posix()}')

        frames = []
        interval = 100

        with Image.open(self.path) as gif:
            assert gif.mode in ('RGB', 'RGBA'), f'GIF loaded in {gif.mode} mode instead of RGB(A)'
            interval = gif.info.get('duration', interval)
            while True:
                frame = gif.copy()
                if frame.mode == 'RGBA':
                    frame = frame.convert('RGB')
                if self.trim is not None:
                    box = self.trim[0], self.trim[1], gif.width - self.trim[2], gif.height - self.trim[3]
                    frame = frame.crop(box)
                if self.rotate != 0:
                    frame = frame.rotate(self.rotate, expand=True)
                frame = frame.resize(TARGET_SIZE)
                frames.append(frame)
                if self.frames > 0 and len(frames) == self.frames:
                    break
                try:
                    gif.seek(gif.tell() + 1)
                except EOFError:
                    break

        if self.decimate is not None and self.decimate > 1:
            frames = [frame for i, frame in enumerate(frames) if (i % self.decimate) == 0]
            interval *= self.decimate

        all_frames = Image.new('RGB', (TARGET_SIZE[0], TARGET_SIZE[1] * len(frames)))
        for i, frame in enumerate(frames):
            all_frames.paste(frame, (0, i * TARGET_SIZE[1]))

        quantized = all_frames.quantize(colors=2 ** self.bpp)

        palette = quantized.getpalette()
        assert len(palette) == 3 * 2 ** self.bpp

        quantized_frames = []
        for i in range(len(frames)):
            frame = quantized.crop((0, i * TARGET_SIZE[1], TARGET_SIZE[0], (i + 1) * TARGET_SIZE[1]))
            assert frame.size == TARGET_SIZE
            quantized_frames.append(frame)

        if self.preview is not None:
            quantized_frames[0].save(
                self.preview,
                save_all=True,
                append_images=quantized_frames[1:],
                loop=0,
                duration=interval,
                palette=palette,
            )

        compressor = AnimationCompressor()
        compressor.write_header(len(frames), interval, self.bpp, palette)
        for frame in quantized_frames:
            pixels = list(frame.getdata())
            compressor.write_frame(pixels)
        result = compressor.data

        decompressor = AnimationDecompressor(result)
        assert decompressor.n_frames == len(frames)
        assert decompressor.interval == interval
        assert decompressor.bpp == self.bpp
        assert bytes(decompressor.palette) == bytes(palette)
        for i, frame in enumerate(quantized_frames):
            decompressed_frame = decompressor.read_frame()
            decompressed_frame = bytes(decompressed_frame)
            expected = bytes(frame.getdata())
            assert decompressed_frame == expected, f'Frame {i} roundtrip failed'
        assert decompressor.eof()

        # efficiency = measure_entropy(result) / 8
        # print(f'  size: {len(result) // 1024:3d} KiB   efficiency: {efficiency * 100:.0f}%')

        data_name = self.name + '_DATA'

        header_lines = [
            'namespace anim {',
            f'    extern Animation {self.name};',
            '}',
        ]

        source_lines = [
            'namespace anim {',
        ] + self.format_data_array(name=data_name, data=result, indent=4) + [
            f'    Animation {self.name}({data_name});',
            '}',
        ]

        return header_lines, source_lines
