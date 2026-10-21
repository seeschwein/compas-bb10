#!/bin/sh
# run-probes.sh - Launcher for on-device compatibility probes
# This script runs inside the BAR sandbox on a BlackBerry 10 device.

set -e

# Set up environment
export LD_LIBRARY_PATH="/app/native/probes:${LD_LIBRARY_PATH}"
PROBE_DIR="/app/native/probes"
RESULTS_FILE="/accounts/1000/appdata/com.compas.probes/probe-results.txt"

# Ensure results directory exists
mkdir -p "$(dirname "$RESULTS_FILE")"

echo "=== Compas BB10 On-Device Probe Suite ===" >> "$RESULTS_FILE"
echo "Started at: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

run_probe() {
    local probe_name="$1"
    local probe_path="$PROBE_DIR/$probe_name"

    if [ ! -f "$probe_path" ] || [ ! -x "$probe_path" ]; then
        echo "FAIL $probe_name: not found or not executable" >> "$RESULTS_FILE"
        return 1
    fi

    echo "Running $probe_name..." >> "$RESULTS_FILE"

    # Capture stdout, stderr, and exit code
    if output=$("$probe_path" 2>&1); then
        exit_code=0
    else
        exit_code=$?
    fi

    echo "$output" >> "$RESULTS_FILE"
    echo "Exit code: $exit_code" >> "$RESULTS_FILE"
    echo "" >> "$RESULTS_FILE"

    if [ $exit_code -eq 0 ]; then
        echo "PASS $probe_name" >> "$RESULTS_FILE"
    else
        echo "FAIL $probe_name (exit $exit_code)" >> "$RESULTS_FILE"
    fi
    echo "" >> "$RESULTS_FILE"
}

# Run all probes
run_probe "filesystem_probe"
run_probe "posix_probe"
run_probe "thread_probe"
run_probe "loader_probe"

echo "=== Probe Suite Complete ===" >> "$RESULTS_FILE"
echo "Finished at: $(date)" >> "$RESULTS_FILE"

# Ensure all output is flushed
sync

# Print results for debugging
cat "$RESULTS_FILE"