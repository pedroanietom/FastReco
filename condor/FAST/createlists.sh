#!/bin/bash

# Check if run number is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <runnumber>"
    exit 1
fi

RUNNUMBER=$1

BASE_DIR_CALO="/sphenix/lustre01/sphnxpro/production2/run2pp/physics/calofitting/ana509_2024p022_v001"
BASE_DIR_TRACK="/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana506_2024p023_v001/DST_TRKR_TRACKS"
BASE_DIR_CLUSTER="/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana517_2024p024_v001/DST_TRKR_CLUSTER"

OUTPUT_FILE_CALO="lists/DST_CALOFITTING_$RUNNUMBER.list"
OUTPUT_FILE_TRACK="lists/DST_TRACK_$RUNNUMBER.list"
OUTPUT_FILE_CLUSTER="lists/DST_CLUSTER_$RUNNUMBER.list"
OUTPUT_FILE_MAPPED="lists/CALOFITTING_TRACK_$RUNNUMBER.list"

# Determine the appropriate folder range
RUN_BASE=$((RUNNUMBER / 100 * 100))
RUN_FOLDER="run_000${RUN_BASE}_000$((RUN_BASE + 100))"

DIRECTORY_CALO="$BASE_DIR_CALO/$RUN_FOLDER"
DIRECTORY_TRACK="$BASE_DIR_TRACK/$RUN_FOLDER"
DIRECTORY_CLUSTER="$BASE_DIR_CLUSTER/$RUN_FOLDER"

# Check if directory exists
if [ ! -d "$DIRECTORY_CALO" ]; then
    echo "Error: Directory $DIRECTORY_CALO does not exist."
    exit 1
fi

if [ ! -d "$DIRECTORY_TRACK" ]; then
    echo "Error: Directory $DIRECTORY_TRACK does not exist."
    exit 1
fi

if [ ! -d "$DIRECTORY_CLUSTER" ]; then
    echo "Error: Directory $DIRECTORY_CLUSTER does not exist."
    exit 1
fi

# List files and save to output file
ls ${DIRECTORY_CALO}/*${RUNNUMBER}* >& ${OUTPUT_FILE_CALO}
echo "File list saved to $OUTPUT_FILE_CALO"

ls ${DIRECTORY_TRACK}/dst/*TRACKS*${RUNNUMBER}* >& ${OUTPUT_FILE_TRACK}
echo "File list saved to $OUTPUT_FILE_TRACK"

ls ${DIRECTORY_CLUSTER}/dst/*CLUSTER*${RUNNUMBER}* >& ${OUTPUT_FILE_CLUSTER}
echo "File list saved to $OUTPUT_FILE_CLUSTER"

#mapping
echo "Creating mapping file: $OUTPUT_FILE_MAPPED"

mapfile -t calo_files < "$OUTPUT_FILE_CALO"

mapfile -t track_files < "$OUTPUT_FILE_TRACK"

mapfile -t cluster_files < "$OUTPUT_FILE_CLUSTER"

> "$OUTPUT_FILE_MAPPED"

num_calo=${#calo_files[@]}
num_track=${#track_files[@]}
num_cluster=${#cluster_files[@]}

for ((i=0; i<num_track; i++)); do

    echo "../${OUTPUT_FILE_CALO} ${track_files[i]} ${cluster_files[i]}" >> "$OUTPUT_FILE_MAPPED"
done

echo "Mapping file saved to $OUTPUT_FILE_MAPPED"
