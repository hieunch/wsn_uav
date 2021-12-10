//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/communication/routing/mipRouting/MIPRoutingPacket.msg.
//

#ifndef _MIPROUTINGPACKET_M_H_
#define _MIPROUTINGPACKET_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "RoutingPacket_m.h"
#include "GeoMathHelper.h"
typedef std::vector<int> IntVector;
// }}

/**
 * Enum generated from <tt>src/node/communication/routing/mipRouting/MIPRoutingPacket.msg:30</tt> by nedtool.
 * <pre>
 * enum MIPRoutingPacket_Type
 * {
 * 
 *     GP_ROUTING_CONTROL_PACKET = 1;
 *     GP_ROUTING_DATA_PACKET = 2;
 *     GP_ROUTING_JOIN_PACKET = 3;
 *     GP_ROUTING_ADV_PACKET = 4;
 * }
 * </pre>
 */
enum MIPRoutingPacket_Type {
    GP_ROUTING_CONTROL_PACKET = 1,
    GP_ROUTING_DATA_PACKET = 2,
    GP_ROUTING_JOIN_PACKET = 3,
    GP_ROUTING_ADV_PACKET = 4
};

/**
 * Class generated from <tt>src/node/communication/routing/mipRouting/MIPRoutingPacket.msg:38</tt> by nedtool.
 * <pre>
 * packet MIPRoutingPacket extends RoutingPacket
 * {
 *     int MIPRoutingPacketKind @enum(MIPRoutingPacket_Type);
 *     int sourceId;
 *     int schedule[];
 *     double remainingEnergy;
 *     IntVector isCH;
 *     IntVector nextCH;
 *     IntVector trace;
 * }
 * </pre>
 */
class MIPRoutingPacket : public ::RoutingPacket
{
  protected:
    int MIPRoutingPacketKind_var;
    int sourceId_var;
    int *schedule_var; // array ptr
    unsigned int schedule_arraysize;
    double remainingEnergy_var;
    IntVector isCH_var;
    IntVector nextCH_var;
    IntVector trace_var;

  private:
    void copy(const MIPRoutingPacket& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MIPRoutingPacket&);

  public:
    MIPRoutingPacket(const char *name=NULL, int kind=0);
    MIPRoutingPacket(const MIPRoutingPacket& other);
    virtual ~MIPRoutingPacket();
    MIPRoutingPacket& operator=(const MIPRoutingPacket& other);
    virtual MIPRoutingPacket *dup() const {return new MIPRoutingPacket(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getMIPRoutingPacketKind() const;
    virtual void setMIPRoutingPacketKind(int MIPRoutingPacketKind);
    virtual int getSourceId() const;
    virtual void setSourceId(int sourceId);
    virtual void setScheduleArraySize(unsigned int size);
    virtual unsigned int getScheduleArraySize() const;
    virtual int getSchedule(unsigned int k) const;
    virtual void setSchedule(unsigned int k, int schedule);
    virtual double getRemainingEnergy() const;
    virtual void setRemainingEnergy(double remainingEnergy);
    virtual IntVector& getIsCH();
    virtual const IntVector& getIsCH() const {return const_cast<MIPRoutingPacket*>(this)->getIsCH();}
    virtual void setIsCH(const IntVector& isCH);
    virtual IntVector& getNextCH();
    virtual const IntVector& getNextCH() const {return const_cast<MIPRoutingPacket*>(this)->getNextCH();}
    virtual void setNextCH(const IntVector& nextCH);
    virtual IntVector& getTrace();
    virtual const IntVector& getTrace() const {return const_cast<MIPRoutingPacket*>(this)->getTrace();}
    virtual void setTrace(const IntVector& trace);
};

inline void doPacking(cCommBuffer *b, MIPRoutingPacket& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, MIPRoutingPacket& obj) {obj.parsimUnpack(b);}


#endif // ifndef _MIPROUTINGPACKET_M_H_

