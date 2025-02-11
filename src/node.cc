//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without outputFileen the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "node.h"
#include "Message_m.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <bitset>
#include <vector>
#include <random>  // For generating random numbers
using namespace std;
Define_Module(Node);

vector<string> regularMessages; // message in the input file
vector<string> errorMessages;   // error codes in the input file
int currentMessageIndex = 0;
bool nodesender = false;
bool node1sender = false;
string sender = "";
string recioutputFileer = "";
Define_Module(Node);

int senderWindowStart = 0;
int senderWindowEnd = 0;
int senderWindowSize = 0;
int smallWindowSize = 0;
vector<bool> delays;
vector<Message_Base*> senderWindow;
vector<bool> ackedMessages(false);

double senderTimeout;
double processingDelay;
double duplicationDelay;
double transmissionDelay;
double errorDelay;
double LossProbability;
int expectedSeqNum;
int currentDataIndex;
int ackExpected;
int nextFrameToSend;
int isNetworkLayerReady;
int frameExpected;
int maxSeqNum;
int senderId;
int receiverId;

ofstream outputFile;

char calculateParity(const string &message) {
    bitset<8> parity(0);
    for (char c : message) {
        bitset<8> charBits(c);
        parity ^= charBits;
    }
    return (char) parity.to_ulong();
}

void Node::ReadMessageFromTextFile(const string &fileName) {
    outputFile.open("../src/output.txt", std::ios::app);
    ifstream inputFile(fileName);  // Use the provided file name
    if (inputFile.is_open()) {
        string line;
        while (getline(inputFile, line)) {
            size_t spacePos = line.find(' ');
            string numberString = line.substr(0, spacePos); // Error code or flag
            string message = line.substr(spacePos + 1);    // Message
            errorMessages.push_back(numberString);
            regularMessages.push_back(message);
        }
        inputFile.close();
    } else {
        outputFile << "Error: Unable to open file: " << fileName << endl;
    }
}

string Encoding(string payload) {
    string EncodedMessage = "$";
    for (int i = 0; i < payload.size(); i++) {
        if (payload[i] == '$') {
            EncodedMessage += "/$";
        } else if (payload[i] == '/') {
            EncodedMessage += "//";
        } else {
            EncodedMessage += payload[i];
        }
    }
    EncodedMessage += "$";
    return EncodedMessage;
}

string Decoding(string payload) {
    string DecodedMessage = "";
    for (int i = 1; i < payload.size() - 1; i++) {
        if (payload[i] == '/' && payload[i + 1] == '/') {
            DecodedMessage += '/';
            i++;
        } else if (payload[i] == '/' && payload[i + 1] == '$') {
            DecodedMessage += '$';
            i++;
        } else if (payload[i] == '/') {
            continue;
        } else if (payload[i] == '$') {
            break;    //this will return wrong message
        } else {
            DecodedMessage += payload[i];
        }
    }

    return DecodedMessage;
}

void Node::modifyMessage(Message_Base *mess) {
    // Implement the logic to modify a bit in the message's payload or checksum
    string payload = mess->getM_payload();
    if (!payload.empty()) {
        payload[0] ^= 1; //  Flip the first bit of the payload
        mess->setM_payload(payload.c_str());
    }
}

void Node::errorHandle(Message_Base *mess, string errorcode, int i = 1) {
    Message_Base *modifiedMsg;

    // No error case
    if (errorcode == "0000") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(mess, i * (processingDelay) + transmissionDelay, "out");

        outputFile << "At time = [" << simTime() + i * (processingDelay)
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer()).to_string()
                << " and Modified[-1], Lost[no], Duplicate[0], Delay[0]"
                << endl;
    }
    // Delay only
    else if (errorcode == "0001") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(mess,
                i * (processingDelay) + transmissionDelay + errorDelay, "out");
        outputFile << "At time = [" << simTime() + i * (processingDelay)
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[no], Duplicate[0], Delay["
                << errorDelay << "]" << endl;
    }
    // Duplication only
    else if (errorcode == "0010") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(mess, i * processingDelay + transmissionDelay, "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[no], Duplicate[1], Delay[0]"
                << endl;

        sendDelayed(mess->dup(),
                i * processingDelay + duplicationDelay + transmissionDelay,
                "out");
        outputFile << "At time = ["
                << simTime() + i * processingDelay + duplicationDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[no], Duplicate[2], Delay[0]"
                << endl;
    }
    // Duplication and Delay
    else if (errorcode == "0011") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(mess, i * processingDelay + transmissionDelay + errorDelay,
                "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[no], Duplicate[1], Delay["
                << errorDelay << "]" << endl;

        sendDelayed(mess->dup(),
                i * processingDelay + duplicationDelay + transmissionDelay
                        + errorDelay, "out");
        outputFile << "At time = ["
                << simTime() + i * processingDelay + duplicationDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[no], Duplicate[2], Delay["
                << errorDelay << "]" << endl;
    }
    // Loss only
    else if (errorcode == "0100") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[yes], Duplicate[0], Delay[0]"
                << endl;
    }
    // Loss and Delay
    else if (errorcode == "0101") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[yes], Duplicate[0], Delay["
                << errorDelay << "]" << endl;
    }
    // Loss with Duplication
    else if (errorcode == "0110") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[yes], Duplicate[1], Delay[0]"
                << endl;
    }
    // Loss with Duplication and Delay
    else if (errorcode == "0111") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[-1], Lost[yes], Duplicate[1], Delay["
                << errorDelay << "]" << endl;
    }
    // Modification only
    else if (errorcode == "1000") {
        modifiedMsg = mess->dup();
        modifyMessage(modifiedMsg);
        modifiedMsg->setM_payload(
                Encoding(modifiedMsg->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(modifiedMsg, i * processingDelay + transmissionDelay,
                "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[0], Delay[0]" << endl;
    }
    // Modification and Delay
    else if (errorcode == "1001") {
        modifiedMsg = mess->dup();
        modifyMessage(modifiedMsg);
        modifiedMsg->setM_payload(
                Encoding(modifiedMsg->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(modifiedMsg,
                i * processingDelay + transmissionDelay + errorDelay, "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[0], Delay["
                << errorDelay << "]" << endl;
    }
    // Modification and Duplication
    else if (errorcode == "1010") {
        modifiedMsg = mess->dup();
        modifyMessage(modifiedMsg);
        modifiedMsg->setM_payload(
                Encoding(modifiedMsg->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(modifiedMsg, i * processingDelay + transmissionDelay,
                "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[1], Delay[0]" << endl;

        sendDelayed(modifiedMsg->dup(),
                i * processingDelay + duplicationDelay + transmissionDelay,
                "out");
        outputFile << "At time = ["
                << simTime() + i * processingDelay + duplicationDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[2], Delay[0]" << endl;
    }
    // Modification, Duplication and Delay
    else if (errorcode == "1011") {
        modifiedMsg = mess->dup();
        modifyMessage(modifiedMsg);
        modifiedMsg->setM_payload(
                Encoding(modifiedMsg->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        sendDelayed(modifiedMsg,
                i * processingDelay + transmissionDelay + errorDelay, "out");
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[1], Delay["
                << errorDelay << "]" << endl;

        sendDelayed(modifiedMsg->dup(),
                i * processingDelay + duplicationDelay + transmissionDelay
                        + errorDelay, "out");
        outputFile << "At time = ["
                << simTime() + i * processingDelay + duplicationDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << modifiedMsg->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[no], Duplicate[2], Delay["
                << errorDelay << "]" << endl;
    }
    // Modification and Loss
    else if (errorcode == "1100") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[yes], Duplicate[0], Delay[0]"
                << endl;
    }
    // Modification, Loss and Delay
    else if (errorcode == "1101") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[yes], Duplicate[0], Delay["
                << errorDelay << "]" << endl;
    }
    // Modification, Loss and Duplication
    else if (errorcode == "1110") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);
        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[yes], Duplicate[1], Delay[0]"
                << endl;
    }
    // Modification, Loss, Duplication and Delay
    else if (errorcode == "1111") {
        mess->setM_payload(Encoding(mess->getM_payload()).c_str());
        char parity = calculateParity(mess->getM_payload());
        mess->setM_trailer(parity);

        outputFile << "At time = [" << simTime() + i * processingDelay
                << "] Node["<< senderId<<"][Sent] Frame with seq_num = "
                << mess->getM_header() << " and payload = ["
                << mess->getM_payload() << "] and trailer = "
                << bitset<8>(mess->getM_trailer())
                << " and Modified[1], Lost[yes], Duplicate[1], Delay["
                << errorDelay << "]" << endl;
    }
}

void increment(int &currentSeqNum) {
    if (currentSeqNum < (maxSeqNum - 1))
        currentSeqNum++;
    else
        currentSeqNum = 0;
}

void Node::sendMessages() {

    int sendMessageConstant = 1;
    // Calculate available window space
    int availableSpace = senderWindowSize - smallWindowSize;

    // Send new messages while window space is available

    while (availableSpace > 0 && currentMessageIndex < regularMessages.size()) {
        Message_Base *mess = new Message_Base();
        mess->setM_payload(regularMessages[currentMessageIndex].c_str());
        mess->setM_type(2);
        mess->setM_header(currentMessageIndex % senderWindowSize);
        mess->setM_id(currentMessageIndex);
        delays.push_back(false);

        // Calculate and set parity
        char parity = calculateParity(regularMessages[currentMessageIndex]);
        mess->setM_trailer(parity);

        // Store message in window
        int windowIndex = currentMessageIndex % senderWindowSize;
        if (senderWindow[windowIndex] != nullptr) {
            delete senderWindow[windowIndex];
        }
        senderWindow[windowIndex] = mess;

        // Log and handle errors
        outputFile << "At time ["
                << simTime() + (sendMessageConstant - 1) * (processingDelay)
                << "], Node["<< senderId<<"], Introducing channel error with code =["
                << errorMessages[currentMessageIndex] << "]." << endl;

        // Handle errors and send message
        errorHandle(mess->dup(), errorMessages[currentMessageIndex],
                sendMessageConstant);

        // Set timer for this message
        scheduleAt(simTime() + senderTimeout + processingDelay,
                new cMessage(to_string(currentMessageIndex).c_str()));

        currentMessageIndex++;
        //increment(senderWindowEnd);
        senderWindowEnd = (senderWindowEnd + 1) % senderWindowSize;
        smallWindowSize++;
        availableSpace--;
        sendMessageConstant++;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Node::initialize() {
    // TODO - Generated method body
    senderWindowSize = par("WS");
    senderTimeout = par("TO");
    processingDelay = par("PT");
    duplicationDelay = par("DD");
    transmissionDelay = par("TD");
    LossProbability = par("LP");
    errorDelay = par("ED");
    senderWindow.resize(senderWindowSize);
    ackedMessages.resize(senderWindowSize, false);
    maxSeqNum = senderWindowSize;
    currentDataIndex = 0;
    expectedSeqNum = 0;
    ackExpected = 0;
    nextFrameToSend = 0;
    isNetworkLayerReady = true;
    frameExpected = 0;

}
void Node::handleAcknowledgment(int ackNum) {
    // Check if the ackNum is within the valid sender window range
    if ((senderWindowStart < senderWindowEnd
            && (ackNum < senderWindowStart || ackNum >= senderWindowEnd))
            || (senderWindowStart > senderWindowEnd
                    && (ackNum < senderWindowStart && ackNum >= senderWindowEnd))) {
        return; // Invalid ACK number
    }

    // Slide the sender window forward to discard all acknowledged frames
    do {
        int index = senderWindowStart % (int) senderWindowSize;
        int id = senderWindow[index]->getM_id();

        delays[id] = true;
        senderWindow[index] = nullptr; // Remove the acknowledged message from the sender window
        ackedMessages[index] = false;  // Reset the ACK status for that message
        increment(senderWindowStart); // Increment and wrap around senderWindowStart
        smallWindowSize--;
    } while (senderWindowStart != (ackNum + 1) % senderWindowSize);

    // Now that the sender window has been updated, send new messages if needed
    sendMessages();
}

bool ReadCoordinatorInput(const string &filePath, int &startNode,
        double &startTime) {
    ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        outputFile << "Unable to open coordinator file: " << filePath << endl;
        return false;
    }

    string inputLine;
    if (getline(inputFile, inputLine)) {
        size_t spacePos = inputLine.find(' '); // Find the first space
        if (spacePos == string::npos) {
            outputFile
                    << "Invalid format. Expected two numbers separated by a space."
                    << endl;
            return false;
        }

        try {
            string firstNumber = inputLine.substr(0, spacePos); // Extract the first number
            string secondNumber = inputLine.substr(spacePos + 1); // Extract the second number

            startNode = stoi(firstNumber); // Parse the first number as an integer
            startTime = stod(secondNumber); // Parse the second number as a double
        } catch (const exception &e) {
            outputFile << "Error parsing coordinator file: " << e.what()
                    << endl;
            return false;
        }
    } else {
        outputFile << "Coordinator file format is invalid or empty." << endl;
        return false;
    }

    inputFile.close();
    return true;
}

void Node::handleMessage(cMessage *msg) {
    //if i am a self message this means timeout happened!
    if (msg->isSelfMessage()) {
        int timedOutIndex = stoi(msg->getName());
        int index = stoi(msg->getName()) % senderWindowSize;
        if ((!delays[timedOutIndex]) && (index == senderWindowStart)) { // Resend the message
            //will only resend the message if this timeout happened within the senderwindow frame (outputFileery message has a timeout)
            //only sending that specific message not entire frame
            outputFile << "Time out Event at time [" << simTime()
                    << "], at Node "<< senderId<<"  for frame with seq_num=[" << index
                    << "]." << endl;
            errorMessages[timedOutIndex] = "0000";
            int j = timedOutIndex;
            int errorHandleConstant = 1;
            int tempWindowSize = smallWindowSize;
            while (tempWindowSize != 0) {
                Message_Base *resend =
                        senderWindow[j % (int) senderWindowSize]->dup();
                errorHandle(resend, errorMessages[j], errorHandleConstant);
                scheduleAt(
                        simTime() + (errorHandleConstant) * processingDelay
                                + senderTimeout,
                        new cMessage(to_string(timedOutIndex).c_str()));
                errorHandleConstant++;
                j++;
                tempWindowSize--;
                timedOutIndex++;

            }

        }
    } else {
        if (strcmp(msg->getSenderModule()->getName(), "coordinator") == 0) {
            // First message to determine if the node is a sender or receiver
            string role = msg->getName();

            if (role == "You are the sender") {
                sender = getName();  // Set the current node as the sender
                if(sender=="node")
                {
                    senderId=0;
                    receiverId=1;
                }
                else
                {
                    senderId=1;
                    receiverId=0;
                }
            } else if (role == "You are the receiver") {
                recioutputFileer = getName(); // Set the current node as the receiver
                EV<<recioutputFileer;
            } else {
                outputFile << "Invalid role received from coordinator: " << role
                        << endl;
            }

            delete msg;  // Clean up the message
        }

        // Here I am the sender
        if (strcmp(getName(), sender.c_str()) == 0) {

            if (strcmp(msg->getSenderModule()->getName(), "coordinator") == 0) {
                string filePath;

                if (strcmp(getName(), "node1") == 0) {
                    // If the current node is "node1" (the sender)
                    filePath = "../src/input1.txt";
                } else {
                    filePath = "../src/input0.txt";
                }
                ReadMessageFromTextFile(filePath);
                sendMessages();

            } else {
                // Only ACKs or NACKs will be received in the sender
                Message_Base *ack = check_and_cast<Message_Base*>(msg);
                if (ack->getM_type() == 1) {  // ACK
                    int ackNum = ack->getM_ack_number();
                    //outputFile << "At time [" << simTime()
                    //<< "], Node ["<< senderId<<"] received ["
                    //<< (ack->getM_type() == 1 ? "ACK" : "NACK")
                    //<< "] with number [" << ackNum << "]" << endl;
                    handleAcknowledgment(ackNum);  // Function handles only ACKs
                } else if (ack->getM_type() == 0) {  // NACK
                    //outputFile << "At time [" << simTime()
                    //<< "], Node ["<< senderId<<"] received ["
                    //<< (ack->getM_type() == 1 ? "ACK" : "NACK")
                    //<< "] with number [" << ack->getM_ack_number()
                    //<< "]" << endl;
                }
            }
        }

        // Here I am the receiver
        if (strcmp(getName(), recioutputFileer.c_str()) == 0) {
            // Handle data messages as the receiver
            if (strcmp(msg->getSenderModule()->getName(), "coordinator") == 0) {
            } else {
                Message_Base *data = check_and_cast<Message_Base*>(msg);
                if (data->getM_type() == 2) {  // Data message
//                    outputFile << "At time [" << simTime()
//                            << "], Node [recioutputFileer] recioutputFileed data with content: "
//                            << data->getName() << endl;
                    receiveMessage(data);
                }
            }
        }

    }

}

void Node::receiveMessage(Message_Base *data) {
    int seqNum = data->getM_header();  // Get the sequence number from the frame

    // First decode the message
    string encodedPayload = data->getM_payload();
    string decodedPayload = Decoding(encodedPayload);

    // Calculate the parity of the decoded message
    char calculatedParity = calculateParity(encodedPayload);
    char expectedParity = data->getM_trailer();

    // Check if the calculated parity matches the expected parity
    bool hasError = (calculatedParity != expectedParity);

    // If the frame is in order and the parity is correct
    if (seqNum == expectedSeqNum) {
        if (hasError) {

            sendACKorNACK(expectedSeqNum, false, decodedPayload);
        } else {

            // Update expectedSeqNum
            if (currentMessageIndex != (regularMessages.size() - 1)) {
                expectedSeqNum = (expectedSeqNum + 1) % senderWindowSize;
            }
            // Send cumulative ACK
            sendACKorNACK(seqNum, true, decodedPayload);  // Send ACK
        }
    } else {
        // Frame is out of order, send ACK for last correctly received frame
//        outputFile << "At time [" << simTime()
//                << "], Node["<<receiverId<<"] discarded out-of-order frame with seq_num=["
//                << seqNum << "] and content=[" << decodedPayload
//                << "]. Expected seq_num=[" << expectedSeqNum << "]." << endl;

        // Send ACK for the last correctly received frame
    }

    // Cleanup the received message object
    delete data;
}

void Node::sendACKorNACK(int seqNum, bool isACK, string decodedMessage) {
    // Randomly decide whether the ACK/NACK should be "lost" based on LossProbability
    int lossRand = uniform(0, 100); // Random integer between 0 and 99
    if (lossRand < LossProbability) {
        outputFile << "At time [" << simTime() + processingDelay
                << "], Node["<<receiverId<<"] lost the ACK/NACK for seq_num=[" << seqNum
                << "]. " << "Original Message: " << decodedMessage << endl;
        return;  // Simulate packet loss
    }

    // Send ACK or NACK based on isACK flag
    Message_Base *ackOrNack = new Message_Base();
    ackOrNack->setM_ack_number(seqNum);
    ackOrNack->setM_type(isACK ? 1 : 0);  // 1 for ACK, 0 for NACK

    outputFile << "At time [" << simTime() + processingDelay
            << "], Node["<<receiverId<<"] sent [" << (isACK ? "ACK" : "NACK")
            << "] for seq_num=[" << seqNum << "]. " << "Original Message: " << decodedMessage << endl;
    sendDelayed(ackOrNack, processingDelay + transmissionDelay, "out");
}

Node::~Node() {
    string filePath = "output.txt";  // Your file path
    ifstream inputFile(filePath);

}

