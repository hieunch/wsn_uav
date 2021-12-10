//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/communication/routing/globecom/globecomPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "globecomPacket_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("globecomPacket_Type");
    if (!e) enums.getInstance()->add(e = new cEnum("globecomPacket_Type"));
    e->insert(GP_ROUTING_CONTROL_PACKET, "GP_ROUTING_CONTROL_PACKET");
    e->insert(GP_ROUTING_DATA_PACKET, "GP_ROUTING_DATA_PACKET");
    e->insert(GP_ROUTING_JOIN_PACKET, "GP_ROUTING_JOIN_PACKET");
    e->insert(GP_ROUTING_ADV_PACKET, "GP_ROUTING_ADV_PACKET");
);

Register_Class(globecomPacket);

globecomPacket::globecomPacket(const char *name, int kind) : ::RoutingPacket(name,kind)
{
    this->globecomPacketKind_var = 0;
    this->sourceId_var = 0;
    schedule_arraysize = 0;
    this->schedule_var = 0;
    this->remainingEnergy_var = 0;
}

globecomPacket::globecomPacket(const globecomPacket& other) : ::RoutingPacket(other)
{
    schedule_arraysize = 0;
    this->schedule_var = 0;
    copy(other);
}

globecomPacket::~globecomPacket()
{
    delete [] schedule_var;
}

globecomPacket& globecomPacket::operator=(const globecomPacket& other)
{
    if (this==&other) return *this;
    ::RoutingPacket::operator=(other);
    copy(other);
    return *this;
}

void globecomPacket::copy(const globecomPacket& other)
{
    this->globecomPacketKind_var = other.globecomPacketKind_var;
    this->sourceId_var = other.sourceId_var;
    delete [] this->schedule_var;
    this->schedule_var = (other.schedule_arraysize==0) ? NULL : new int[other.schedule_arraysize];
    schedule_arraysize = other.schedule_arraysize;
    for (unsigned int i=0; i<schedule_arraysize; i++)
        this->schedule_var[i] = other.schedule_var[i];
    this->remainingEnergy_var = other.remainingEnergy_var;
    this->isCH_var = other.isCH_var;
    this->nextCH_var = other.nextCH_var;
    this->trace_var = other.trace_var;
}

void globecomPacket::parsimPack(cCommBuffer *b)
{
    ::RoutingPacket::parsimPack(b);
    doPacking(b,this->globecomPacketKind_var);
    doPacking(b,this->sourceId_var);
    b->pack(schedule_arraysize);
    doPacking(b,this->schedule_var,schedule_arraysize);
    doPacking(b,this->remainingEnergy_var);
    doPacking(b,this->isCH_var);
    doPacking(b,this->nextCH_var);
    doPacking(b,this->trace_var);
}

void globecomPacket::parsimUnpack(cCommBuffer *b)
{
    ::RoutingPacket::parsimUnpack(b);
    doUnpacking(b,this->globecomPacketKind_var);
    doUnpacking(b,this->sourceId_var);
    delete [] this->schedule_var;
    b->unpack(schedule_arraysize);
    if (schedule_arraysize==0) {
        this->schedule_var = 0;
    } else {
        this->schedule_var = new int[schedule_arraysize];
        doUnpacking(b,this->schedule_var,schedule_arraysize);
    }
    doUnpacking(b,this->remainingEnergy_var);
    doUnpacking(b,this->isCH_var);
    doUnpacking(b,this->nextCH_var);
    doUnpacking(b,this->trace_var);
}

int globecomPacket::getGlobecomPacketKind() const
{
    return globecomPacketKind_var;
}

void globecomPacket::setGlobecomPacketKind(int globecomPacketKind)
{
    this->globecomPacketKind_var = globecomPacketKind;
}

int globecomPacket::getSourceId() const
{
    return sourceId_var;
}

void globecomPacket::setSourceId(int sourceId)
{
    this->sourceId_var = sourceId;
}

void globecomPacket::setScheduleArraySize(unsigned int size)
{
    int *schedule_var2 = (size==0) ? NULL : new int[size];
    unsigned int sz = schedule_arraysize < size ? schedule_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        schedule_var2[i] = this->schedule_var[i];
    for (unsigned int i=sz; i<size; i++)
        schedule_var2[i] = 0;
    schedule_arraysize = size;
    delete [] this->schedule_var;
    this->schedule_var = schedule_var2;
}

unsigned int globecomPacket::getScheduleArraySize() const
{
    return schedule_arraysize;
}

int globecomPacket::getSchedule(unsigned int k) const
{
    if (k>=schedule_arraysize) throw cRuntimeError("Array of size %d indexed by %d", schedule_arraysize, k);
    return schedule_var[k];
}

void globecomPacket::setSchedule(unsigned int k, int schedule)
{
    if (k>=schedule_arraysize) throw cRuntimeError("Array of size %d indexed by %d", schedule_arraysize, k);
    this->schedule_var[k] = schedule;
}

double globecomPacket::getRemainingEnergy() const
{
    return remainingEnergy_var;
}

void globecomPacket::setRemainingEnergy(double remainingEnergy)
{
    this->remainingEnergy_var = remainingEnergy;
}

IntVector& globecomPacket::getIsCH()
{
    return isCH_var;
}

void globecomPacket::setIsCH(const IntVector& isCH)
{
    this->isCH_var = isCH;
}

IntVector& globecomPacket::getNextCH()
{
    return nextCH_var;
}

void globecomPacket::setNextCH(const IntVector& nextCH)
{
    this->nextCH_var = nextCH;
}

IntVector& globecomPacket::getTrace()
{
    return trace_var;
}

void globecomPacket::setTrace(const IntVector& trace)
{
    this->trace_var = trace;
}

class globecomPacketDescriptor : public cClassDescriptor
{
  public:
    globecomPacketDescriptor();
    virtual ~globecomPacketDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(globecomPacketDescriptor);

globecomPacketDescriptor::globecomPacketDescriptor() : cClassDescriptor("globecomPacket", "RoutingPacket")
{
}

globecomPacketDescriptor::~globecomPacketDescriptor()
{
}

bool globecomPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<globecomPacket *>(obj)!=NULL;
}

const char *globecomPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int globecomPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int globecomPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<7) ? fieldTypeFlags[field] : 0;
}

const char *globecomPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "globecomPacketKind",
        "sourceId",
        "schedule",
        "remainingEnergy",
        "isCH",
        "nextCH",
        "trace",
    };
    return (field>=0 && field<7) ? fieldNames[field] : NULL;
}

int globecomPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='g' && strcmp(fieldName, "globecomPacketKind")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sourceId")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "schedule")==0) return base+2;
    if (fieldName[0]=='r' && strcmp(fieldName, "remainingEnergy")==0) return base+3;
    if (fieldName[0]=='i' && strcmp(fieldName, "isCH")==0) return base+4;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextCH")==0) return base+5;
    if (fieldName[0]=='t' && strcmp(fieldName, "trace")==0) return base+6;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *globecomPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "double",
        "IntVector",
        "IntVector",
        "IntVector",
    };
    return (field>=0 && field<7) ? fieldTypeStrings[field] : NULL;
}

const char *globecomPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "globecomPacket_Type";
            return NULL;
        default: return NULL;
    }
}

int globecomPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    globecomPacket *pp = (globecomPacket *)object; (void)pp;
    switch (field) {
        case 2: return pp->getScheduleArraySize();
        default: return 0;
    }
}

std::string globecomPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    globecomPacket *pp = (globecomPacket *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getGlobecomPacketKind());
        case 1: return long2string(pp->getSourceId());
        case 2: return long2string(pp->getSchedule(i));
        case 3: return double2string(pp->getRemainingEnergy());
        case 4: {std::stringstream out; out << pp->getIsCH(); return out.str();}
        case 5: {std::stringstream out; out << pp->getNextCH(); return out.str();}
        case 6: {std::stringstream out; out << pp->getTrace(); return out.str();}
        default: return "";
    }
}

bool globecomPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    globecomPacket *pp = (globecomPacket *)object; (void)pp;
    switch (field) {
        case 0: pp->setGlobecomPacketKind(string2long(value)); return true;
        case 1: pp->setSourceId(string2long(value)); return true;
        case 2: pp->setSchedule(i,string2long(value)); return true;
        case 3: pp->setRemainingEnergy(string2double(value)); return true;
        default: return false;
    }
}

const char *globecomPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 4: return opp_typename(typeid(IntVector));
        case 5: return opp_typename(typeid(IntVector));
        case 6: return opp_typename(typeid(IntVector));
        default: return NULL;
    };
}

void *globecomPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    globecomPacket *pp = (globecomPacket *)object; (void)pp;
    switch (field) {
        case 4: return (void *)(&pp->getIsCH()); break;
        case 5: return (void *)(&pp->getNextCH()); break;
        case 6: return (void *)(&pp->getTrace()); break;
        default: return NULL;
    }
}


