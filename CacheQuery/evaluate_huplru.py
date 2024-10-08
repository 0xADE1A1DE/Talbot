import subprocess
from collections import defaultdict
import statistics as stats
import re

# Maximum attempts
max_attempts = 3

# Number of executions
iterations = 10


# Execute the command multiple times
def run_command(command, arguments):
    # Dictionary to store the unique outputs, their counts, and line numbers
    outputs = [{} for i in range(len(arguments))]

    runs = iterations

    for i in range(iterations):
        attempts = 0
        current_vector = 0;
        while attempts < max_attempts:
            try:
                # Run the command and capture the output
                result = subprocess.run([command] + arguments, capture_output=True, text=True, check=True)

                # Split the output into lines and find the start after the banner
                lines = result.stdout.strip().splitlines()

                for line in lines:
                    if "Cache state: " in line:
                        cache_state = line.split("Cache state: ")[1].strip()

                        if cache_state not in outputs[current_vector]:
                            outputs[current_vector][cache_state] = 0
                        outputs[current_vector][cache_state] += 1

                        current_vector += 1

                break

            except subprocess.CalledProcessError as e:
                attempts += 1
                if attempts == max_attempts:
                    print(f"Iteration {i+1}: Maximum retries reached. Giving up.")
                    runs -= 1

    return outputs

queries = ["AABBCCDDE", "AABBCCDDAE",   "AABBCCDDAEF",   "AABBCCDDEC",   "AABBCCDDECCF",    "AABCCDDE", "AABCCDDEF",   "AABBCCDDAEDF", "AABBCCDDAEDCDF"]
results = ["BCDE",      "ACDE",         "ACDF",          "BCDE",         "CDEF",            "ACDE",     "ACDF",        "ADEF",         "CDEF"]


command_outputs = run_command("./trigger", queries)

huplru = True
correct_result_measured = 0

for i in range(len(command_outputs)):
    max_key = max(command_outputs[i], key=command_outputs[i].get)
    if(max_key != results[i]):
        print(f"Most likely output for query \"{queries[i]}\" is not \"{results[i]}\"")
        print(f"Measured outputs: {command_outputs[i]}")
        huplru = False
        if results[i] in command_outputs[i]:
            correct_result_measured += command_outputs[i][results[i]]
    else:
        correct_result_measured += command_outputs[i][max_key]
    #print(f"{max_key} - {(command_outputs[i][max_key]/iterations):.0%}");

if(huplru):
    print("Translation cache seems to implement HUPLRU!")
else:
    print("Translation cache does NOT appear to implement HUPLRU!")

huplru_likelyhood = correct_result_measured / (iterations * len(results));
print(f"Likelyhood that translation cache implements HUPLRU: {huplru_likelyhood:.0%}")