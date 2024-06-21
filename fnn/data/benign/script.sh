#!/bin/bash

# Counter starts at 1
counter=1

# Loop through all files that match ex*.c
for file in ex*.c; do
  # Check if file exists to avoid errors when no files match the pattern
  if [[ -f $file ]]; then
    # Rename the file
    mv "$file" "banign_ex${counter}.c"
    # Increment the counter
    ((counter++))
  fi
done
