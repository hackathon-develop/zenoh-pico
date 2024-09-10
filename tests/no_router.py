import argparse
import os
from signal import SIGINT
import subprocess
import sys
import time

# Specify the directory for the binaries
DIR_EXAMPLES = "build/examples"


def test_all():
    print("*** Pub & sub test ***")
    test_status = 0

    # Expected output & status
    z_expected_status = 255
    z_expected_output = '''Opening session...
Unable to open session!'''

    print("Start subscriber")
    # Start z_sub
    z_sub_command = f"stdbuf -oL -eL ./{DIR_EXAMPLES}/z_sub"
    z_sub_process = subprocess.Popen(
        z_sub_command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    # Wait for z_sub to finish
    z_sub_process.wait()

    print("Start publisher")
    # Start z_pub
    z_pub_command = f"stdbuf -oL -eL ./{DIR_EXAMPLES}/z_pub"
    z_pub_process = subprocess.Popen(
        z_pub_command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    # Wait for z_pub to finish
    z_pub_process.wait()

    print("Start queryable")
    # Start z_queryable
    z_queryable_command = f"stdbuf -oL -eL ./{DIR_EXAMPLES}/z_queryable"
    z_queryable_process = subprocess.Popen(
        z_queryable_command,
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    # Wait for z_queryable to finish
    z_queryable_process.wait()

    print("Start query")
    # Start z_query
    z_query_command = f"stdbuf -oL -eL ./{DIR_EXAMPLES}/z_get"
    z_query_process = subprocess.Popen(
        z_query_command,
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    # Wait for z_query to finish
    z_query_process.wait()

    print("Check publisher status & output")
    # Check the exit status of z_pub
    z_pub_status = z_pub_process.returncode
    if z_pub_status == z_expected_status:
        print("z_pub status valid")
    else:
        print(f"z_pub status invalid, expected: {z_expected_status}, received: {z_pub_status}")
        test_status = 1

    # Check output of z_pub
    z_pub_output = z_pub_process.stdout.read()
    if z_expected_output in z_pub_output:
        print("z_pub output valid")
    else:
        print("z_pub output invalid:")
        print(f"Expected: \"{z_expected_output}\"")
        print(f"Received: \"{z_pub_output}\"")
        test_status = 1

    print("Check subscriber status & output")
    # Check the exit status of z_sub
    z_sub_status = z_sub_process.returncode
    if z_sub_status == z_expected_status:
        print("z_sub status valid")
    else:
        print(f"z_sub status invalid, expected: {z_expected_status}, received: {z_sub_status}")
        test_status = 1

    # Check output of z_sub
    z_sub_output = z_sub_process.stdout.read()
    if z_expected_output in z_sub_output:
        print("z_sub output valid")
    else:
        print("z_sub output invalid:")
        print(f"Expected: \"{z_expected_output}\"")
        print(f"Received: \"{z_sub_output}\"")
        test_status = 1

    print("Check query status & output")
    # Check the exit status of z_query
    z_query_status = z_query_process.returncode
    if z_query_status == z_expected_status:
        print("z_query status valid")
    else:
        print(f"z_query status invalid, expected: {z_expected_status}," f" received: {z_query_status}")
        test_status = 1

    # Check output of z_query
    z_query_output = z_query_process.stdout.read()
    if z_expected_output in z_query_output:
        print("z_query output valid")
    else:
        print("z_query output invalid:")
        print(f'Expected: "{z_expected_output}"')
        print(f'Received: "{z_query_output}"')
        test_status = 1

    print("Check queryable status & output")
    # Check the exit status of z_queryable
    z_queryable_status = z_queryable_process.returncode
    if z_queryable_status == z_expected_status:
        print("z_queryable status valid")
    else:
        print(f"z_queryable status invalid, expected: {z_expected_status}," f" received: {z_queryable_status}")
        test_status = 1

    # Check output of z_queryable
    z_queryable_output = z_queryable_process.stdout.read()
    if z_expected_output in z_queryable_output:
        print("z_queryable output valid")
    else:
        print("z_queryable output invalid:")
        print(f'Expected: "{z_expected_output}"')
        print(f'Received: "{z_queryable_output}"')
        test_status = 1

    # Return value
    return test_status


if __name__ == "__main__":
    EXIT_STATUS = 0

    # Test all examples
    if test_all() == 1:
        EXIT_STATUS = 1

    # Exit
    sys.exit(EXIT_STATUS)