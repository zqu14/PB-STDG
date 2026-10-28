/*
 * RenewalBeaconing.cc
 *
 *  Created on: Sep 6, 2020
 *      Author: awillig
 */



#include <inet/common/IProtocolRegistrationListener.h>
#include <inet/common/ModuleAccess.h>
#include "../base/BeaconReport_m.h"
#include "../neighbortable/NeighborTable.h"
#include "Nbt.h"
#include "RenewalBeaconing.h"
#include <cassert>
#include <sstream>

// =======================================================================

Define_Module(RenewalBeaconing);

const inet::Protocol renewalBeaconingProtocol("renewalBeaconing","Renewal Beaconing protocol based on LBP");

simsignal_t sigBeaconsSent = cComponent::registerSignal("renewalBeaconingBeaconsSent");

// =======================================================================
// =======================================================================

using namespace inet;



// =======================================================================
// =======================================================================

RenewalBeaconing::RenewalBeaconing ()
{
    debugMsgPrefix = "RenewalBeaconing";
}

const inet::Protocol& RenewalBeaconing::getProtocol(void) const
{
    return renewalBeaconingProtocol;
}

// -------------------------------------------

void RenewalBeaconing::initialize (int stage)
{
  enter("initialize");
  debugMsg(xsprintf("initialize: stage is %d",stage));

  LBPClientBase::initialize(stage);

  if (stage == INITSTAGE_LOCAL)
    {
      readParameters ();
      startSelfMessages ();

      mobility           = nullptr;
      sequenceNumber     = 0;

      reportingGate      = gate("beaconReport");
      toProcessor        = gate("beaconOut");

      preferredSpeed     = par("preferredSpeed");
      maxpSpeed          = par("maxSpeed");

      assert(reportingGate);
    }

  if (stage == INITSTAGE_LAST)
    {
      findModulePointers ();
      registerProtocol(renewalBeaconingProtocol, gate("toLBP"), nullptr);
      registerService(renewalBeaconingProtocol, nullptr, gate("fromLBP"));
    }

  leave("initialize");
}


// -------------------------------------------

void RenewalBeaconing::readParameters (void)
{
  enter("readParameters");

  initialWaitTime         =  par ("initialWaitTime");          assert(initialWaitTime>=0);
  positionSamplingPeriod  =  par ("positionSamplingPeriod");   assert(positionSamplingPeriod>0);
  beaconLength            =  par ("beaconLength");             assert(beaconLength>0);

  leave("readParameters");
}


// -------------------------------------------

void RenewalBeaconing::findModulePointers (void)
{
  enter("findModulePointers");

  cModule *host = getContainingNode (this);
  assert(host);
  mobility      = check_and_cast<DroneLinearMobility *>(host->getSubmodule("mobility"));
  assert(mobility);

  samplePosition();

  leave("findModulePointers");
}

// -------------------------------------------

void RenewalBeaconing::startSelfMessages (void)
{
  enter("startSelfMessages");

  pMsgGenerate = new cMessage ("RenewalBeaconing::GenerateBeacon");
  assert (pMsgGenerate);
  scheduleAt (simTime() + initialWaitTime + par("iatDistribution"), pMsgGenerate);

  pMsgSamplePosition = new cMessage ("RenewalBeaconing::SamplePosition");
  assert (pMsgSamplePosition);
  scheduleAt (simTime() + positionSamplingPeriod, pMsgSamplePosition);

  leave("startSelfMessages");
}


// -------------------------------------------

void RenewalBeaconing::samplePosition (void)
{
  enter("samplePositions");
  assert (mobility);

  currPosition             = mobility->getCurrentPosition();
  currVelocity             = mobility->getCurrentVelocity();
  currI                    = mobility->getCurrI();
  currJ                    = mobility->getCurrJ();
  currRoadId               = mobility->getcurrRoadId();
  currT                    = mobility->getCurrentT();
  isSwitching              = mobility->getIsSwitching();
  if (!isSwitching) {
      switchToI = 9999;
      switchToJ = 9999;
  }
  leave("samplePositions");
}

// -------------------------------------------

void RenewalBeaconing::handleSelfMessage (cMessage *msg)
{
  enter("handleSelfMessage");

  if (msg == pMsgSamplePosition)
    {
      debugMsg(xsprintf("handleSelfMessage: sampling positions"));
      scheduleAt (simTime() + positionSamplingPeriod, pMsgSamplePosition);
      samplePosition();
      return;
    }

  if (msg == pMsgGenerate)
    {
      debugMsg(xsprintf("handleSelfMessage: sending beacon"));
      scheduleAt (simTime()+par("iatDistribution"), pMsgGenerate);
      sendBeacon();
      return;
    }

  error("RenewalBeaconing::handleSelfMessage: cannot handle message");
}

// -------------------------------------------


void RenewalBeaconing::sendBeaconNow() {
    Enter_Method("sendBeaconNow()");
    //cancelEvent(pMsgGenerate);
    //scheduleAt (simTime()+par("iatDistribution"), pMsgGenerate);
    sendBeacon();
}


// -------------------------------------------

void RenewalBeaconing::handleOtherMessage (cMessage *msg)
{
    if (strcmp(msg->getName(),"SBN") == 0) {
        cancelEvent(pMsgGenerate);
        scheduleAt (simTime()+par("iatDistribution"), pMsgGenerate);
        sendBeacon();
        delete msg;
    } else {
        error("RenewalBeaconing::handleOtherMessage: cannot handle message");
    }
}

// -------------------------------------------

bool RenewalBeaconing::beaconWellFormed (const Beacon& beacon)
{
    //assert(beacon);

    if (beacon.getMagicNo() != SWARMSTACK_BEACON_MAGICNO)
    {
        EV << "beaconWellFormed: magicno is wrong, beacon = " << beacon << std::endl;
        return false;
    }

    if (beacon.getVersion() != SWARMSTACK_VERSION)
    {
        EV << "beaconWellFormed: wrong version number, beacon = " << beacon << std::endl;
        return false;
    }

    return true;
}

// -------------------------------------------


void RenewalBeaconing::handleReceivedBroadcast(Packet* packet)
{
  enter("handleReceivedBroadcast");
  packet->clearTags();
  const auto& beacon    = *(packet->popAtFront<Beacon>());
  if (beaconWellFormed(beacon))
  {
      processReceivedBeacon (beacon);
      //sendBeaconToProcessor(beacon);
  }
  delete packet;

  leave("handleReceivedBroadcast");
}

// -------------------------------------------

void RenewalBeaconing::sendBeaconReport(const Beacon& beacon)
{
    enter("sendBeaconReport");
    if ((beacon.getSenderId() != ownIdentifier) && reportingGate && reportingGate->isConnected())
    {
        debugMsg("sendBeaconReport: sending a report");
        BeaconReport  *report = new BeaconReport("BeaconReport");
        report->setSeqno(beacon.getSeqno());
        report->setSenderId(beacon.getSenderId());
        report->setSenderPosition(beacon.getCurrPosition());
        report->setSenderVelocity(beacon.getCurrVelocity());
        report->setCurrI(beacon.getCurrI());
        report->setCurrJ(beacon.getCurrJ());
        report->setCurrentRoadId(beacon.getCurrentRoadId());
        report->setCurrT(beacon.getCurrT());
        report->setTargetI(beacon.getTargetI());
        report->setTargetJ(beacon.getTargetJ());
        report->setTargetT(beacon.getTargetT());
        report->setTargetRoadId(beacon.getTargetRoadId());
        report->setNextAlgorithmTime(beacon.getNextAlgorithmTime());
        report->setIsSwitching(beacon.getIsSwitching());
        report->setSwitchToI(beacon.getSwitchToI());
        report->setSwitchToJ(beacon.getSwitchToJ());
        report->setPSpeed(beacon.getPSpeed());
        report->setGapTime(simTime().dbl() - beacon.getCTime());
        BeaconReport* copy = new BeaconReport(*report);
        send(report, reportingGate);
        send(copy, toProcessor);

    }
    leave("sendBeaconReport");
}

void RenewalBeaconing::sendBeaconToProcessor(Beacon* beacon)
{
    //Beacon* copy = new Beacon(*beacon);
    //send(copy, toProcessor);
}



// -------------------------------------------

void RenewalBeaconing::processReceivedBeacon(const Beacon& beacon)
{
    enter("processReceivedBeacon");
    sendBeaconReport(beacon);

    EV << "processReceivedBeacon " << ownIdentifier << ": received beacon from " << beacon.getSenderId()
       << " on position " << beacon.getCurrPosition()
       << " with seqno " << beacon.getSeqno()
       << " on Lane(" << beacon.getCurrI() << "," << beacon.getCurrJ() << ")"
       << " on Road" << beacon.getCurrentRoadId()
       << endl;

    leave("processReceivedBeacon");
}

// -------------------------------------------

Ptr<Beacon> RenewalBeaconing::composeBeacon (void)
{
    samplePosition();
  enter("composeBeacon");

  //auto nbTable = pro->getnbTable();
  //auto seqTable = pro->getseqTable();

  auto beacon = makeShared<Beacon>();
  assert(beacon);
  beacon->setMagicNo(SWARMSTACK_BEACON_MAGICNO);
  beacon->setVersion(SWARMSTACK_VERSION);
  beacon->setSenderId(ownIdentifier);
  beacon->setSeqno(sequenceNumber++);
  beacon->setCurrPosition(currPosition);
  beacon->setCurrVelocity(currVelocity);
  beacon->setCurrI(currI);
  beacon->setCurrJ(currJ);
  beacon->setCurrentRoadId(currRoadId.c_str());
  beacon->setTargetRoadId(targetRoadId.c_str());
  beacon->setCurrT(currT);
  beacon->setTargetI(targetI);
  beacon->setTargetJ(targetJ);
  beacon->setTargetT(targetT);
  beacon->setSwitchToI(switchToI);
  beacon->setSwitchToJ(switchToJ);
  beacon->setIsSwitching(isSwitching);
  beacon->setNextAlgorithmTime(nextAlgorithmTime);
  beacon->setPSpeed(preferredSpeed);
  beacon->setCTime(simTime().dbl());
  //beacon->setChunkLength(B(beaconLength) + B(nbTable.size() * 13));
  beacon->setChunkLength(B(beaconLength));
  leave("composeBeacon");

  //std::cout << "send target I: " << targetI << "  j: " << targetJ << "  T: " << targetT << "  iS: " << isSwitching << "  nRT: " << nextAlgorithmTime << endl;
  //std::cout << "send curret I: " << currI << "  j: " << currJ << "  T: " << currT << "  cT: " << simTime().dbl() << endl;
  //std::cout << "send switchTo I: " << switchToI << "  j: " << switchToJ << endl;
  return beacon;
}

// -------------------------------------------

void RenewalBeaconing::sendBeacon (void)
{
  enter("sendBeacon");
  Ptr<Beacon> beacon  = composeBeacon();
  Packet     *packet  = new Packet ("Beacon");
  assert(beacon);
  assert(packet);
  packet->insertAtBack(beacon);
  packet->setKind(SWARMSTACK_BEACON_KIND);
  sendViaLBP (packet);
  emit(sigBeaconsSent, true);
  leave("sendBeacon");
}

// -------------------------------------------

void RenewalBeaconing::finish (void)
{
}

// -------------------------------------------

RenewalBeaconing::~RenewalBeaconing (void)
{
    enter("~RenewalBeaconing");
    cancelAndDelete(pMsgGenerate);
    cancelAndDelete(pMsgSamplePosition);
    leave("~RenewalBeaconing");
}


