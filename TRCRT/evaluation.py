import subprocess
from collections import defaultdict
import statistics as stats
import re

# Maximum attempts
max_attempts = 3

# Number of executions
iterations = 100
runs = iterations

# Command to execute
command = './trigger'

# Dictionary to store the unique outputs, their counts, and line numbers
output_info = defaultdict(lambda: {'count': 0, 'line_numbers': []})

# Execute the command multiple times
for i in range(iterations):
    attempts = 0
    while attempts < max_attempts:
        try:
            # Run the command and capture the output
            result = subprocess.run([command], capture_output=True, text=True, check=True)

            print(f"Iteration {i+1}: Success!")

            # Split the output into lines and find the start after the banner
            lines = result.stdout.strip().splitlines()

            # Find the index of the start of relevant output
            start_index = 0
            for index, line in enumerate(lines):
                if line.strip() == "[********************** RESULTS **********************]":
                    start_index = index + 1  # Start processing from the next line
                    break

            # Get the relevant output lines
            relevant_lines = lines[start_index:]

            # Process each line to clean and count occurrences
            for line_number, line in enumerate(relevant_lines, start=start_index + 1):
                # Remove probability information in parentheses
                cleaned_line = re.sub(r'[\s]+?\((\d+?)\/(\d+?)\)', '', line).strip()

                certainty = re.search(r'\((\d+?)\/(\d+?)\)', line)

                if certainty:
                    if 'certainty' not in output_info[cleaned_line]:
                        output_info[cleaned_line]['certainty'] = []
                    output_info[cleaned_line]['certainty'].append(int(certainty.group(1), 10) / int(certainty.group(2), 10))
                
                # Count the occurrence of the cleaned line
                if cleaned_line:  # Only count non-empty lines
                    output_info[cleaned_line]['count'] += 1
                    if line_number not in output_info[cleaned_line]['line_numbers']:
                        output_info[cleaned_line]['line_numbers'].append(line_number)

            break

        except subprocess.CalledProcessError as e:
            attempts += 1
            if attempts == max_attempts:
                print(f"Iteration {i+1}: Maximum retries reached. Giving up.")
                runs -= 1

# Calculate the probability of each unique output
print("\nSummary:\n")

positions = []

for output, info in output_info.items():
    position = max(info['line_numbers'])
    if position not in positions:
        positions.append(position)

positions.sort()

for position in positions:
    for output, info in reversed(output_info.items()):
        if position != max(info['line_numbers']):
            continue;
        probability = info['count'] / runs
        if 'certainty' in info:
            avg_certainty = stats.mean(info['certainty'])
            standard_deviation_certainty = stats.stdev(info['certainty']) if len(info['certainty']) > 1 else 0 
            print(f"{output} - {probability:.0%} -> Avg. certainty {avg_certainty:.0%}, Standard deviation {standard_deviation_certainty:.0%}")
        else:
            print(f"{output} - {probability:.0%}")
