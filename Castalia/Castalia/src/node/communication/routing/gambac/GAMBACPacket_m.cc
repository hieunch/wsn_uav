//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/communication/routing/gambac/GAMBACPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "GAMBACPacket_m.h"

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
    cEnum *e = cEnum::find("GAMBACPacket_Type");
    if (!e) enums.getInstance()->add(e = new cEnum("GAMBACPacket_Type"));
    e->insert(GAMBAC_ROUTING_CONTROL_PACKET, "GAMBAC_ROUTING_CONTROL_PACKET");
    e->insert(GAMBAC_ROUTING_DATA_PACKET, "GAMBAC_ROUTING_DATA_PACKET");
    e->insert(GAMBAC_ROUTING_JOIN_PACKET, "GAMBAC_ROUTING_JOIN_PACKET");
    e->insert(GAMBAC_ROUTING_ADV_PACKET, "GAMBAC_ROUTING_ADV_PACKET");
);

Register_Class(GAMBACPacket);

GAMBACPacket::GAMBACPacket(const char *name, int kind) : ::RoutingPacket(name,kind)
{
    this->GAMBACPacketKind_var = 0;
    schedule_arraysize = 0;
    this->schedule_var = 0;
    this->remainingEnergy_var = 0;
}

GAMBACPacket::GAMBACPacket(const GAMBACPacket& other) : ::RoutingPacket(other)
{
    schedule_arraysize = 0;
    this->schedule_var = 0;
    copy(other);
}

GAMBACPacket::~GAMBACPacket()
{
    delete [] schedule_var;
}

GAMBACPacket& GAMBACPacket::operator=(const GAMBACPacket& other)
{
    if (this==&other) return *this;
    ::RoutingPacket::operator=(other);
    copy(other);
    return *this;
}

void GAMBACPacket::copy(const GAMBACPacket& other)
{
    this->GAMBACPacketKind_var = other.GAMBACPacketKind_var;
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

void GAMBACPacket::parsimPack(cCommBuffer *b)
{
    ::RoutingPacket::parsimPack(b);
    doPacking(b,this->GAMBACPacketKind_var);
    b->pack(schedule_arraysize);
    doPacking(b,this->schedule_var,schedule_arraysize);
    doPacking(b,this->remainingEnergy_var);
    doPacking(b,this->isCH_var);
    doPacking(b,this->nextCH_var);
    doPacking(b,this->trace_var);
}

void GAMBACPacket::parsimUnpack(cCommBuffer *b)
{
    ::RoutingPacket::parsimUnpack(b);
    doUnpacking(b,this->GAMBACPacketKind_var);
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

int GAMBACPacket::getGAMBACPacketKind() const
{
    return GAMBACPacketKind_var;
}

void GAMBACPacket::setGAMBACPacketKind(int GAMBACPacketKind)
{
    this->GAMBACPacketKind_var = GAMBACPacketKind;
}

void GAMBACPacket::setScheduleArraySize(unsigned int size)
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

unsigned int GAMBACPacket::getScheduleArraySize() const
{
    return schedule_arraysize;
}

int GAMBACPacket::getSchedule(unsigned int k) const
{
    if (k>=schedule_arraysize) throw cRuntimeError("Array of size %d indexed by %d", schedule_arraysize, k);
    return schedule_var[k];
}

void GAMBACPacket::setSchedule(unsigned int k, int schedule)
{
    if (k>=schedule_arraysize) throw cRuntimeError("Array of size %d indexed by %d", schedule_arraysize, k);
    this->schedule_var[k] = schedule;
}

double GAMBACPacket::getRemainingEnergy() const
{
    return remainingEnergy_var;
}

void GAMBACPacket::setRemainingEnergy(double remainingEnergy)
{
    this->remainingEnergy_var = remainingEnergy;
}

IntVector& GAMBACPacket::getIsCH()
{
    return isCH_var;
}

void GAMBACPacket::setIsCH(const IntVector& isCH)
{
    this->isCH_var = isCH;
}

IntVector& GAMBACPacket::getNextCH()
{
    return nextCH_var;
}

void GAMBACPacket::setNextCH(const IntVector& nextCH)
{
    this->nextCH_var = nextCH;
}

IntVector& GAMBACPacket::getTrace()
{
    return trace_var;
}

void GAMBACPacket::setTrace(const IntVector& trace)
{
    this->trace_var = trace;
}

class GAMBACPacketDescriptor : public cClassDescriptor
{
  public:
    GAMBACPacketDescriptor();
    virtual ~GAMBACPacketDescriptor();

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

Register_ClassDescriptor(GAMBACPacketDescriptor);

GAMBACPacketDescriptor::GAMBACPacketDescriptor() : cClassDescriptor("GAMBACPacket", "RoutingPacket")
{
}

GAMBACPacketDescriptor::~GAMBACPacketDescriptor()
{
}

bool GAMBACPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<GAMBACPacket *>(obj)!=NULL;
}

const char *GAMBACPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int GAMBACPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount(object) : 6;
}

unsigned int GAMBACPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<6) ? fieldTypeFlags[field] : 0;
}

const char *GAMBACPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "GAMBACPacketKind",
        "schedule",
        "remainingEnergy",
        "isCH",
        "nextCH",
        "trace",
    };
    return (field>=0 && field<6) ? fieldNames[field] : NULL;
}

int GAMBACPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='G' && strcmp(fieldName, "GAMBACPacketKind")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "schedule")==0) return base+1;
    if (fieldName[0]=='r' && strcmp(fieldName, "remainingEnergy")==0) return base+2;
    if (fieldName[0]=='i' && strcmp(fieldName, "isCH")==0) return base+3;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextCH")==0) return base+4;
    if (fieldName[0]=='t' && strcmp(fieldName, "trace")==0) return base+5;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *GAMBACPacketDescriptor::getFieldTypeString(void *object, int field) const
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
        "double",
        "IntVector",
        "IntVector",
        "IntVector",
    };
    return (field>=0 && field<6) ? fieldTypeStrings[field] : NULL;
}

const char *GAMBACPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "GAMBACPacket_Type";
            return NULL;
        default: return NULL;
    }
}

int GAMBACPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    GAMBACPacket *pp = (GAMBACPacket *)object; (void)pp;
    switch (field) {
        case 1: return pp->getScheduleArraySize();
        default: return 0;
    }
}

std::string GAMBACPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    GAMBACPacket *pp = (GAMBACPacket *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getGAMBACPacketKind());
        case 1: return long2string(pp->getSchedule(i));
        case 2: return double2string(pp->getRemainingEnergy());
        case 3: {std::stringstream out; out << pp->getIsCH(); return out.str();}
        case 4: {std::stringstream out; out << pp->getNextCH(); return out.str();}
        case 5: {std::stringstream out; out << pp->getTrace(); return out.str();}
        default: return "";
    }
}

bool GAMBACPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    GAMBACPacket *pp = (GAMBACPacket *)object; (void)pp;
    switch (field) {
        case 0: pp->setGAMBACPacketKind(string2long(value)); return true;
        case 1: pp->setSchedule(i,string2long(value)); return true;
        case 2: pp->setRemainingEnergy(string2double(value)); return true;
        default: return false;
    }
}

const char *GAMBACPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 3: return opp_typename(typeid(IntVector));
        case 4: return opp_typename(typeid(IntVector));
        case 5: return opp_typename(typeid(IntVector));
        default: return NULL;
    };
}

void *GAMBACPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    GAMBACPacket *pp = (GAMBACPacket *)object; (void)pp;
    switch (field) {
        case 3: return (void *)(&pp->getIsCH()); break;
        case 4: return (void *)(&pp->getNextCH()); break;
        case 5: return (void *)(&pp->getTrace()); break;
        default: return NULL;
    }
}


