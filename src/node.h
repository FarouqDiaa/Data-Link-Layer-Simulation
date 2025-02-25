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

#ifndef __NETWORKPROJECT_NODE_H_
#define __NETWORKPROJECT_NODE_H_

#include <omnetpp.h>
#include "Message_m.h"

using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  public :
    virtual ~Node();
    void  ReadMessageFromTextFile(const string &fileName);
    void modifyMessage(Message_Base *mess);
    void errorHandle(Message_Base* mess, string errorcode,int i);
    void sendMessages();
    void handleAcknowledgment(int ackNum);
    void receiveMessage(Message_Base *data);
    void sendACKorNACK(int seqNum, bool isACK, string decodedMessage);
};

#endif
