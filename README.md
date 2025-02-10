# Data Link Layer Protocol Simulation

## Overview
This project is a **Data Link Layer Protocol Simulation** implemented in **OMNeT++ (C++)**. It simulates communication between two nodes over a noisy channel using the **Go-Back-N ARQ protocol**, **Byte Stuffing for framing**, and **Parity Byte for error detection**. The simulation accounts for various transmission errors, including **packet loss, duplication, modification, and delay**.

## Features
- **Go-Back-N ARQ protocol** for reliable transmission.
- **Byte Stuffing** for framing to ensure proper data encapsulation.
- **Parity Byte** for error detection and correction.
- **Noisy Channel Simulation** to introduce random transmission errors.
- **Event Logging** to track message transmissions and protocol behavior.

## Configuration
Modify the `.ini` configuration file to adjust:
- **Window Size (WS)** for Go-Back-N
- **Timeout Interval (TO)**
- **Packet Error Probabilities** (Loss, Duplication, Modification, Delay)

## Output Files
- **output.txt**: Logs all message transmissions, error events, and protocol actions.
- **debug.log**: Optional debugging information.

## Example Log Output
```
At time 1.5, Node 0 sent frame 2 with payload "Hello" and parity 10101101, Modified -1, Lost No, Duplicate 0, Delay 0.
At time 2.0, Node 1 received frame 2, sending ACK 2.
At time 5.5, Node 0 retransmitted frame 3 due to timeout.
```

## Contributors
Developed as part of **Cairo University’s Computer Networks course**.


