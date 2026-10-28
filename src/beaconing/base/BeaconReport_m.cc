//
// Generated file, do not edit! Created by nedtool 5.6 from beaconing/base/BeaconReport.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "BeaconReport_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
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

Register_Class(BeaconReport)

BeaconReport::BeaconReport(const char *name, short kind) : ::omnetpp::cMessage(name,kind)
{
    this->seqno = 0;
    this->currI = 0;
    this->currJ = 0;
    this->currT = 0;
    this->targetI = 0;
    this->targetJ = 0;
    this->switchToI = 0;
    this->switchToJ = 0;
    this->targetT = 0;
    this->nextAlgorithmTime = 0;
    this->isSwitching = false;
    this->pSpeed = 0;
    this->gapTime = 0;
}

BeaconReport::BeaconReport(const BeaconReport& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

BeaconReport::~BeaconReport()
{
}

BeaconReport& BeaconReport::operator=(const BeaconReport& other)
{
    if (this==&other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void BeaconReport::copy(const BeaconReport& other)
{
    this->seqno = other.seqno;
    this->senderId = other.senderId;
    this->senderPosition = other.senderPosition;
    this->senderVelocity = other.senderVelocity;
    this->nnbt = other.nnbt;
    this->currI = other.currI;
    this->currJ = other.currJ;
    this->currentRoadId = other.currentRoadId;
    this->currT = other.currT;
    this->targetI = other.targetI;
    this->targetJ = other.targetJ;
    this->switchToI = other.switchToI;
    this->switchToJ = other.switchToJ;
    this->targetT = other.targetT;
    this->nextAlgorithmTime = other.nextAlgorithmTime;
    this->isSwitching = other.isSwitching;
    this->pSpeed = other.pSpeed;
    this->gapTime = other.gapTime;
    this->targetRoadId = other.targetRoadId;
}

void BeaconReport::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->seqno);
    doParsimPacking(b,this->senderId);
    doParsimPacking(b,this->senderPosition);
    doParsimPacking(b,this->senderVelocity);
    doParsimPacking(b,this->nnbt);
    doParsimPacking(b,this->currI);
    doParsimPacking(b,this->currJ);
    doParsimPacking(b,this->currentRoadId);
    doParsimPacking(b,this->currT);
    doParsimPacking(b,this->targetI);
    doParsimPacking(b,this->targetJ);
    doParsimPacking(b,this->switchToI);
    doParsimPacking(b,this->switchToJ);
    doParsimPacking(b,this->targetT);
    doParsimPacking(b,this->nextAlgorithmTime);
    doParsimPacking(b,this->isSwitching);
    doParsimPacking(b,this->pSpeed);
    doParsimPacking(b,this->gapTime);
    doParsimPacking(b,this->targetRoadId);
}

void BeaconReport::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->seqno);
    doParsimUnpacking(b,this->senderId);
    doParsimUnpacking(b,this->senderPosition);
    doParsimUnpacking(b,this->senderVelocity);
    doParsimUnpacking(b,this->nnbt);
    doParsimUnpacking(b,this->currI);
    doParsimUnpacking(b,this->currJ);
    doParsimUnpacking(b,this->currentRoadId);
    doParsimUnpacking(b,this->currT);
    doParsimUnpacking(b,this->targetI);
    doParsimUnpacking(b,this->targetJ);
    doParsimUnpacking(b,this->switchToI);
    doParsimUnpacking(b,this->switchToJ);
    doParsimUnpacking(b,this->targetT);
    doParsimUnpacking(b,this->nextAlgorithmTime);
    doParsimUnpacking(b,this->isSwitching);
    doParsimUnpacking(b,this->pSpeed);
    doParsimUnpacking(b,this->gapTime);
    doParsimUnpacking(b,this->targetRoadId);
}

uint32_t BeaconReport::getSeqno() const
{
    return this->seqno;
}

void BeaconReport::setSeqno(uint32_t seqno)
{
    this->seqno = seqno;
}

MacAddress& BeaconReport::getSenderId()
{
    return this->senderId;
}

void BeaconReport::setSenderId(const MacAddress& senderId)
{
    this->senderId = senderId;
}

Coord& BeaconReport::getSenderPosition()
{
    return this->senderPosition;
}

void BeaconReport::setSenderPosition(const Coord& senderPosition)
{
    this->senderPosition = senderPosition;
}

Coord& BeaconReport::getSenderVelocity()
{
    return this->senderVelocity;
}

void BeaconReport::setSenderVelocity(const Coord& senderVelocity)
{
    this->senderVelocity = senderVelocity;
}

Nnbt& BeaconReport::getNnbt()
{
    return this->nnbt;
}

void BeaconReport::setNnbt(const Nnbt& nnbt)
{
    this->nnbt = nnbt;
}

int BeaconReport::getCurrI() const
{
    return this->currI;
}

void BeaconReport::setCurrI(int currI)
{
    this->currI = currI;
}

int BeaconReport::getCurrJ() const
{
    return this->currJ;
}

void BeaconReport::setCurrJ(int currJ)
{
    this->currJ = currJ;
}

const char * BeaconReport::getCurrentRoadId() const
{
    return this->currentRoadId.c_str();
}

void BeaconReport::setCurrentRoadId(const char * currentRoadId)
{
    this->currentRoadId = currentRoadId;
}

double BeaconReport::getCurrT() const
{
    return this->currT;
}

void BeaconReport::setCurrT(double currT)
{
    this->currT = currT;
}

int BeaconReport::getTargetI() const
{
    return this->targetI;
}

void BeaconReport::setTargetI(int targetI)
{
    this->targetI = targetI;
}

int BeaconReport::getTargetJ() const
{
    return this->targetJ;
}

void BeaconReport::setTargetJ(int targetJ)
{
    this->targetJ = targetJ;
}

int BeaconReport::getSwitchToI() const
{
    return this->switchToI;
}

void BeaconReport::setSwitchToI(int switchToI)
{
    this->switchToI = switchToI;
}

int BeaconReport::getSwitchToJ() const
{
    return this->switchToJ;
}

void BeaconReport::setSwitchToJ(int switchToJ)
{
    this->switchToJ = switchToJ;
}

double BeaconReport::getTargetT() const
{
    return this->targetT;
}

void BeaconReport::setTargetT(double targetT)
{
    this->targetT = targetT;
}

double BeaconReport::getNextAlgorithmTime() const
{
    return this->nextAlgorithmTime;
}

void BeaconReport::setNextAlgorithmTime(double nextAlgorithmTime)
{
    this->nextAlgorithmTime = nextAlgorithmTime;
}

bool BeaconReport::getIsSwitching() const
{
    return this->isSwitching;
}

void BeaconReport::setIsSwitching(bool isSwitching)
{
    this->isSwitching = isSwitching;
}

double BeaconReport::getPSpeed() const
{
    return this->pSpeed;
}

void BeaconReport::setPSpeed(double pSpeed)
{
    this->pSpeed = pSpeed;
}

double BeaconReport::getGapTime() const
{
    return this->gapTime;
}

void BeaconReport::setGapTime(double gapTime)
{
    this->gapTime = gapTime;
}

const char * BeaconReport::getTargetRoadId() const
{
    return this->targetRoadId.c_str();
}

void BeaconReport::setTargetRoadId(const char * targetRoadId)
{
    this->targetRoadId = targetRoadId;
}

class BeaconReportDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    BeaconReportDescriptor();
    virtual ~BeaconReportDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(BeaconReportDescriptor)

BeaconReportDescriptor::BeaconReportDescriptor() : omnetpp::cClassDescriptor("BeaconReport", "omnetpp::cMessage")
{
    propertynames = nullptr;
}

BeaconReportDescriptor::~BeaconReportDescriptor()
{
    delete[] propertynames;
}

bool BeaconReportDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BeaconReport *>(obj)!=nullptr;
}

const char **BeaconReportDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *BeaconReportDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int BeaconReportDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 19+basedesc->getFieldCount() : 19;
}

unsigned int BeaconReportDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<19) ? fieldTypeFlags[field] : 0;
}

const char *BeaconReportDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "seqno",
        "senderId",
        "senderPosition",
        "senderVelocity",
        "nnbt",
        "currI",
        "currJ",
        "currentRoadId",
        "currT",
        "targetI",
        "targetJ",
        "switchToI",
        "switchToJ",
        "targetT",
        "nextAlgorithmTime",
        "isSwitching",
        "pSpeed",
        "gapTime",
        "targetRoadId",
    };
    return (field>=0 && field<19) ? fieldNames[field] : nullptr;
}

int BeaconReportDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "seqno")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderId")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderPosition")==0) return base+2;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderVelocity")==0) return base+3;
    if (fieldName[0]=='n' && strcmp(fieldName, "nnbt")==0) return base+4;
    if (fieldName[0]=='c' && strcmp(fieldName, "currI")==0) return base+5;
    if (fieldName[0]=='c' && strcmp(fieldName, "currJ")==0) return base+6;
    if (fieldName[0]=='c' && strcmp(fieldName, "currentRoadId")==0) return base+7;
    if (fieldName[0]=='c' && strcmp(fieldName, "currT")==0) return base+8;
    if (fieldName[0]=='t' && strcmp(fieldName, "targetI")==0) return base+9;
    if (fieldName[0]=='t' && strcmp(fieldName, "targetJ")==0) return base+10;
    if (fieldName[0]=='s' && strcmp(fieldName, "switchToI")==0) return base+11;
    if (fieldName[0]=='s' && strcmp(fieldName, "switchToJ")==0) return base+12;
    if (fieldName[0]=='t' && strcmp(fieldName, "targetT")==0) return base+13;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextAlgorithmTime")==0) return base+14;
    if (fieldName[0]=='i' && strcmp(fieldName, "isSwitching")==0) return base+15;
    if (fieldName[0]=='p' && strcmp(fieldName, "pSpeed")==0) return base+16;
    if (fieldName[0]=='g' && strcmp(fieldName, "gapTime")==0) return base+17;
    if (fieldName[0]=='t' && strcmp(fieldName, "targetRoadId")==0) return base+18;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *BeaconReportDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint32_t",
        "MacAddress",
        "Coord",
        "Coord",
        "Nnbt",
        "int",
        "int",
        "string",
        "double",
        "int",
        "int",
        "int",
        "int",
        "double",
        "double",
        "bool",
        "double",
        "double",
        "string",
    };
    return (field>=0 && field<19) ? fieldTypeStrings[field] : nullptr;
}

const char **BeaconReportDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BeaconReportDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BeaconReportDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    BeaconReport *pp = (BeaconReport *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *BeaconReportDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BeaconReport *pp = (BeaconReport *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BeaconReportDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BeaconReport *pp = (BeaconReport *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getSeqno());
        case 1: {std::stringstream out; out << pp->getSenderId(); return out.str();}
        case 2: {std::stringstream out; out << pp->getSenderPosition(); return out.str();}
        case 3: {std::stringstream out; out << pp->getSenderVelocity(); return out.str();}
        case 4: {std::stringstream out; out << pp->getNnbt(); return out.str();}
        case 5: return long2string(pp->getCurrI());
        case 6: return long2string(pp->getCurrJ());
        case 7: return oppstring2string(pp->getCurrentRoadId());
        case 8: return double2string(pp->getCurrT());
        case 9: return long2string(pp->getTargetI());
        case 10: return long2string(pp->getTargetJ());
        case 11: return long2string(pp->getSwitchToI());
        case 12: return long2string(pp->getSwitchToJ());
        case 13: return double2string(pp->getTargetT());
        case 14: return double2string(pp->getNextAlgorithmTime());
        case 15: return bool2string(pp->getIsSwitching());
        case 16: return double2string(pp->getPSpeed());
        case 17: return double2string(pp->getGapTime());
        case 18: return oppstring2string(pp->getTargetRoadId());
        default: return "";
    }
}

bool BeaconReportDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    BeaconReport *pp = (BeaconReport *)object; (void)pp;
    switch (field) {
        case 0: pp->setSeqno(string2ulong(value)); return true;
        case 5: pp->setCurrI(string2long(value)); return true;
        case 6: pp->setCurrJ(string2long(value)); return true;
        case 7: pp->setCurrentRoadId((value)); return true;
        case 8: pp->setCurrT(string2double(value)); return true;
        case 9: pp->setTargetI(string2long(value)); return true;
        case 10: pp->setTargetJ(string2long(value)); return true;
        case 11: pp->setSwitchToI(string2long(value)); return true;
        case 12: pp->setSwitchToJ(string2long(value)); return true;
        case 13: pp->setTargetT(string2double(value)); return true;
        case 14: pp->setNextAlgorithmTime(string2double(value)); return true;
        case 15: pp->setIsSwitching(string2bool(value)); return true;
        case 16: pp->setPSpeed(string2double(value)); return true;
        case 17: pp->setGapTime(string2double(value)); return true;
        case 18: pp->setTargetRoadId((value)); return true;
        default: return false;
    }
}

const char *BeaconReportDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 1: return omnetpp::opp_typename(typeid(MacAddress));
        case 2: return omnetpp::opp_typename(typeid(Coord));
        case 3: return omnetpp::opp_typename(typeid(Coord));
        case 4: return omnetpp::opp_typename(typeid(Nnbt));
        default: return nullptr;
    };
}

void *BeaconReportDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    BeaconReport *pp = (BeaconReport *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSenderId()); break;
        case 2: return (void *)(&pp->getSenderPosition()); break;
        case 3: return (void *)(&pp->getSenderVelocity()); break;
        case 4: return (void *)(&pp->getNnbt()); break;
        default: return nullptr;
    }
}


