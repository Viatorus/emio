#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys
from io import StringIO
from pathlib import Path


def parse_args(args):
    parser = argparse.ArgumentParser(
        description='Creates the size difference of a base file size to multiple other files sizes as lcov info file')
    parser.add_argument('--size-tool', type=Path, help='path to the GNU size tool', metavar='SIZE_TOOL', required=True)
    parser.add_argument('base', type=Path, help='the first file is used as base size', metavar='BASE_FILE')
    parser.add_argument('files', type=Path, nargs='+', help='file(s) to compare', metavar='FILE')
    parser.add_argument('--output', '-o', type=Path, help='specify the output file', metavar='OUTPUT_FILE',
                        required=True)

    return parser.parse_args(args)


class Generator:
    def __init__(self, size_tool, base_file, out):
        self._size_tool = size_tool
        self._base_file = base_file.absolute()
        self._base_size = self._get_size(base_file)
        self._out = out
        self._out.write("TN:\n")

        print("{}: {} (base)".format(base_file, self._base_size))

    def generate(self, file: Path):
        file_path = file.absolute()
        file_size = self._get_size(file)

        print('{} - {}'.format(file_path, file_size))

        if file_size < self._base_size:
            raise Exception(f'{file} cannot be smaller than {self._base_file}')

        self._out.write(f'SF:{file_path}\n')
        for covered in range(self._base_size):
            self._out.write(f'DA:{covered},1\n')

        for uncovered in range(self._base_size, file_size):
            self._out.write(f'DA:{uncovered},0\n')
        self._out.write('end_of_record\n')

    def _get_size(self, file: Path):
        output = subprocess.check_output([str(self._size_tool), '--format=GNU', str(file)], encoding='UTF-8')
        # text       data        bss      total filename
        #  348        120         28        496  base
        # Get total from regex:
        total = re.search('text\s+data\s+bss\s+total\s+filename\n\s+\d+\s+\d+\s+\d+\s+(\d+)\s+', output)
        if not total:
            raise Exception(f'Unexpected output from size: {output}')
        return int(total[1])


def run():
    args = parse_args(sys.argv[1:])

    if not args.base.is_file():
        raise Exception(f'Base file not found: {args.base}')

    output = StringIO()
    generator = Generator(args.size_tool, args.base, output)

    for file in args.files:
        if not file.is_file():
            raise Exception(f'File not found: {file}')
        generator.generate(file)

    args.output.write_text(output.getvalue())


if __name__ == '__main__':
    run()
