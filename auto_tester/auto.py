#!/bin/python3

import os
import sys
import asyncio
import async_timeout

CALCULATE_HASH_CMD = bytearray([1])


async def worker_func(prog_path, file_names, wait_time, result_dict):
    for file_path in file_names:
        file_name = os.path.basename(file_path)
        result_dict[file_name] = None  # if any crash happen, None will mark it

        process = await asyncio.create_subprocess_exec(prog_path, file_path, stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE)

        await asyncio.sleep(wait_time)

        process.stdin.write(CALCULATE_HASH_CMD)
        await process.stdin.drain()

        async with async_timeout.timeout(wait_time):
            result_dict[file_name] = await process.stdout.read(64)  # get screen hash (sha256)

        process.kill()


def get_files(directory):
    ret = []

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('gb', 'gbc')):
                ret.append(os.path.join(root, file))

    return ret


def load_answers(file_name):
    res = dict()

    with open(file_name, 'r') as file:
        for line in file:
            test_name, category, result_hash = line.split(';')

            if category in ('y', 'n'):
                res[test_name] = (category, result_hash.rstrip())

            else:
                res[test_name] = ('b', '')

    return res


def print_by_name(header, list_of_names):
    if len(list_of_names) == 0:
        return

    print("\n{}".format(header))

    for n in list_of_names:
        print("{}".format(n))


def print_result(test_name, exp, act):
    add_info = ''

    if exp != act:
        add_info = ' [{} unexpected results]'.format(abs(exp-act))

    act = abs(act)
    perc = act/exp*100 if exp != 0 else 100

    print("expected to {0}: {1} actually {0}ed: {2} ({3}%){4}".format(test_name, exp, act, perc, add_info))

if __name__ == '__main__':
    if len(sys.argv) != 6:
        print('Args: <emulator path> <result file> <top test directory> <number of parallel tests> <wait time for tests>')

    else:
        program_path = sys.argv[1]
        result_file_name = sys.argv[2]
        dir_name = sys.argv[3]
        workers_num = int(sys.argv[4])
        sleep_time = int(sys.argv[5])

        file_list = get_files(dir_name)
        work_chunks = [file_list[n::workers_num] for n in range(0, workers_num)]

        results = dict()
        tests_to_run = asyncio.gather(*[worker_func(program_path, w, sleep_time, results) for w in work_chunks], return_exceptions=True)

        loop = asyncio.get_event_loop()
        crashed_tests = loop.run_until_complete(tests_to_run)
        loop.close()

        answers = load_answers(result_file_name)

        passed_count = 0
        failed_count = 0
        broke_change = 0

        not_passed = []
        not_failed = []
        crashed = []
        unknown = []

        for name, screen_hash in results.items():
            if screen_hash is None:
                crashed.append(name)

            else:
                try:
                    result, saved_hash = answers[name]

                    if result == 'y':
                        passed_count += 1

                        if saved_hash != screen_hash.decode('ascii'):
                            not_passed.append(name)

                    elif result == 'n':
                        failed_count += 1

                        if saved_hash != screen_hash.decode('ascii'):
                            not_failed.append(name)

                    else:
                        broke_change += 1

                except KeyError:
                    unknown.append(name)

        test_count = len(file_list)
        crashed_count = len(crashed)
        unknown_count = len(unknown)
        passed_change = len(not_passed)
        failed_change = len(not_failed)

        print("tests total: {}".format(test_count))
        print_result('pass', passed_count, passed_count - passed_change)
        print_result('fail', failed_count, failed_count - failed_change)
        print_result('crash', broke_change, broke_change - crashed_count)
        print('unknown tests: {} ({}%)'.format(unknown_count, unknown_count/test_count*100))

        print_by_name("unexpected results in passed tests:", not_passed)
        print_by_name("unexpected results in failed tests:", not_failed)
        print_by_name("crashed tests by name:", crashed)
        print_by_name("unknown tests by name:", unknown)
