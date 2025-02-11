//
// Generated file, do not edit! Created by opp_msgtool 6.1 from Message.msg.
//

#ifndef __MESSAGE_M_H
#define __MESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>



class Message;
/**
 * Class generated from <tt>Message.msg:19</tt> by opp_msgtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet Message
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 *     int m_header;
 *     string m_payload;
 *     char m_trailer;
 *     int m_type;              // 2=data, 1=ack, 0=notack
 *     int m_ack_number;        // nack_ack 
 * }
 * </pre>
 *
 * Message_Base is only useful if it gets subclassed, and Message is derived from it.
 * The minimum code to be written for Message is the following:
 *
 * <pre>
 * class Message : public Message_Base
 * {
 *   private:
 *     void copy(const Message& other) { ... }

 *   public:
 *     Message(const char *name=nullptr, short kind=0) : Message_Base(name,kind) {}
 *     Message(const Message& other) : Message_Base(other) {copy(other);}
 *     Message& operator=(const Message& other) {if (this==&other) return *this; Message_Base::operator=(other); copy(other); return *this;}
 *     virtual Message *dup() const override {return new Message(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from Message_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(Message)
 * </pre>
 */
class Message_Base : public ::omnetpp::cPacket
{
public:
    int m_header = 0;
    omnetpp::opp_string m_payload;
    char m_trailer = 0;
    int m_type = 0;
    int m_ack_number = 0;
    int m_id=0;


    void copy(const Message_Base& other);


    bool operator==(const Message_Base&) = delete;
    // make constructors protected to avoid instantiation
    Message_Base(const char *name=nullptr, short kind=0);
    Message_Base(const Message_Base& other);
    // make assignment operator protected to force the user override it
    Message_Base& operator=(const Message_Base& other);


    virtual ~Message_Base();
    virtual Message_Base *dup() const override {return new Message_Base(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual int getM_header() const;
    virtual void setM_header(int m_header);

    virtual const char * getM_payload() const;
    virtual void setM_payload(const char * m_payload);

    virtual char getM_trailer() const;
    virtual void setM_trailer(char m_trailer);

    virtual int getM_type() const;
    virtual void setM_type(int m_type);

    virtual int getM_ack_number() const;
    virtual void setM_ack_number(int m_ack_number);

    virtual int getM_id() const;
    virtual void setM_id(int id);

};


namespace omnetpp {

template<> inline Message_Base *fromAnyPtr(any_ptr ptr) { return check_and_cast<Message_Base*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __MESSAGE_M_H

