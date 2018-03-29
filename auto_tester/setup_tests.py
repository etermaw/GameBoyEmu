import os
import sys
import subprocess

CALCULATE_HASH_CMD = bytearray([1])


def get_files(directory):
    ret = []

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('gb', 'gbc')):
                ret.append(os.path.join(root, file))

    return ret


def main(directory, out_file_name):
    files = get_files(directory)

    with open(out_file_name, 'w') as f:
        for file in files:
            p = subprocess.Popen(['./emu', file], stdin=subprocess.PIPE, stdout=subprocess.PIPE)

            print("Is this correct result? [y, n, b]")
            ans = input()

            if ans in ('y', 'n'):
                p.stdin.write(CALCULATE_HASH_CMD)
                p.stdin.close()

                hash_res = p.stdout.read(64)
                f.write('{};{};{}\n'.format(os.path.basename(file), ans, hash_res.decode('ascii')))

            elif ans == 'b':
                f.write('{};{};{}\n'.format(os.path.basename(file), ans, ''))

            else:
                pass

            p.kill()


if __name__ == '__main__':
    if len(sys.argv) == 3:
        main(sys.argv[1], sys.argv[2])

    else:
        print("setup_tests.py <directory with tests> <output file name>")
