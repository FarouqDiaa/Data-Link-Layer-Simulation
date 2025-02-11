//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "coordinator.h"
#include <fstream>

Define_Module(Coordinator);
void Coordinator::initialize() {
    // Open the input file
    std::ifstream inputFile("../src/coordinator.txt");

    if (inputFile.is_open()) {
        int startNode;
        double startTime;

        // Read the sender node and the delay time
        inputFile >> startNode;
        inputFile >> startTime;

        inputFile.close();

        // Create and send a message to the specified sender node
        if (startNode == 0 || startNode == 1) {
            cMessage *msg = new cMessage("You are the sender");
            // Send the message to the specified gate (out[startNode]) with a delay
            sendDelayed(msg, startTime, "out", startNode);

            // Notify the other node to act as the receiver
            int receiverNode = (startNode == 0) ? 1 : 0;
            cMessage *receiverMsg = new cMessage("You are the receiver");
            sendDelayed(receiverMsg, startTime, "out", receiverNode);
        } else {
            EV << "Invalid startNode (" << startNode << ") read from the file"
                      << endl;
        }
    } else {
        EV << "Error opening file coordinator.txt" << endl;
    }
}

void Coordinator::handleMessage(cMessage *msg) {
    // TODO - Generated method body
}
