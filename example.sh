#!/usr/bin/sh
# Example usage script.

PARPORT='/dev/parport0'
TIME='1'

# Gapped lights.
GAP=0
while [ "${GAP}" -lt 8 ]; do
    GAP="$(expr "${GAP}" + 1)"
    ./relayctl -f "${PARPORT}" $(seq 1 "${GAP}" 8)
    sleep "${TIME}"
done

# Runner lights.
START=0
while [ "${START}" -lt 7 ]; do
    START="$(expr "${START}" + 1)"
    ./relayctl -f "${PARPORT}" $(seq "${START}" "$(expr "${START}" + 1)")
    sleep "${TIME}"
done

# Reset the port.
./relayctl -f "${PARPORT}"
