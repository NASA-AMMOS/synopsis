'''

Description: calls C++ SYNOPSIS library by passing input files

'''

import subprocess
import os
import json

def synopsis(path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file, output_file=None):
    '''
    Entry point to C++ implementation of SYNOPSIS (synopsis_cli)

    downlink states (given in dictionaries describing each DP):
    UNTRANSMITTED = 0
    TRANSMITTED = 1
    DOWNLINKED = 2

    Parameters
    ----------
    path_to_synopsis_cli: str
        Path to the CLI executable `synopsis_cli`
    asdpdb_file: str
        Path to DB file
    rule_config_file: str
        Path to JSON file with rules
    similarity_config_file: str
        Path to JSON file with similarity and diversity parameters
    output_file: str
        Path to JSON file to output, or None if writing to file is not desired

    Returns
    -------
    logs: list
        List of log statements from SYNOPSIS
    prioritized_dps: list
        None if there is an error running SYNOPSIS, else a list of dictionaries holding DP information in order of priority 

    '''

    if not os.path.exists(path_to_synopsis_cli):
        raise ValueError("Invalid synopsis_cli: " + str(path_to_synopsis_cli))
    if not os.path.exists(asdpdb_file):
        raise ValueError("Invalid asdpdb_file: " + str(asdpdb_file))
    if not os.path.exists(rule_config_file):
        raise ValueError("Invalid rule_config_file: " + str(rule_config_file))
    if not os.path.exists(similarity_config_file):
        raise ValueError("Invalid similarity_config_file: " + str(similarity_config_file))

    logs = []
    prioritized_dps = None

    try:
        commands_list = [path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file]
        if output_file is not None:
            print("Saving priorities to file: ", output_file)
            commands_list.append(output_file)
        # cp = subprocess.run(commands_list,capture_output=True, check=True)
        process = subprocess.Popen(
            commands_list,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            encoding='utf-8',
            errors='replace'
        )

        while True:
            realtime_output = process.stdout.readline()
            if realtime_output == '' and process.poll() is not None:
                break
            else:
                logs.append(realtime_output.strip())
            if realtime_output:
                print(realtime_output.strip(), flush=True)
        if len(logs) == 0:
            logs = None

    except FileNotFoundError as exc:
        print("Error running SYNOPSIS")
        print(f"{type(exc).__name__}: {exc}")
        raise

    except subprocess.CalledProcessError as exc:
        print("Error running SYNOPSIS")
        print(f"{type(exc).__name__}: {exc}")
        stderr = exc.stderr.decode().split('\n')
        stdout = exc.stdout.decode().split('\n')

        print("stderr: ")
        for line in stderr:
            print(line)
        print('stdout: ')
        for line in stdout:
            print(line)
        raise

    else:
        print("SYNOPSIS ran successfully")
        idx_start = logs.index('BEGIN PRIORITIZATION RESULTS')
        idx_end = logs.index('END PRIORITIZATION RESULTS')
        json_msg_lines = logs[idx_start+1:idx_end]
        json_string = ""
        for l in json_msg_lines:
            json_string +=  l
        json_obj = json.loads(json_string)
        # prioritized_dps = []
        # for j in json_obj:
        #     prioritized_dps.append(j["dp_id"])
        prioritized_dps = json_obj
        # logs  = cp.stderr.decode().split('\n')  # NOTE: not needed if using subprocess.Popen instead of subprocess.run
        # try:
        #     prioritized_dps = json.loads(cp.stdout.decode())
        # except json.decoder.JSONDecodeError as exc:
        #     print(f"{type(exc).__name__}: {exc}")
        #     print("Invalid JSON: \n", cp.stdout.decode())
        #     raise
    finally:
        return logs, prioritized_dps


if __name__ == "__main__":

    path_to_synopsis_cli = './build/synopsis_cli'

    # Example files
    asdpdb_file = 'test/data/dd_example.db'
    rule_config_file = 'test/data/dd_example_rules.json'
    similarity_config_file = 'test/data/dd_example_similarity_config.json'
    # CLI call: ./build/synopsis_cli test/data/dd_example.db test/data/dd_example_rules.json test/data/dd_example_similarity_config.json output.json
    
    # Pass name of JSON File to output to, or None if writing to file is not desired
    output_file = 'output.json'  # or None if outputs are not to be saved
    # output_file = None
    logs, prioritized_dps = synopsis(path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file, output_file)

    print(" >>>>>>>>>>>>> END RUN <<<<<<<<<<<<<<<")
    print(" >>>>>>>>>>>>> END RUN <<<<<<<<<<<<<<<")
    print(" >>>>>>>>>>>>> END RUN <<<<<<<<<<<<<<<")
    print("logs: ")
    for line in logs:
        print(line)
    
    print("prioritized_dps: ", prioritized_dps)
    # if prioritized_dps is not None:
    #     print("prioritized_dps: ", prioritized_dps)
    