/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/qd-channel-model.h"

#include "ns3/test.h"
#include "ns3/node-container.h"
#include "ns3/constant-position-mobility-model.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

// Test case for importing information from the Input/ folder
class QdChannelTestCaseInput : public TestCase
{
public:
  QdChannelTestCaseInput ();
  virtual ~QdChannelTestCaseInput ();

private:
  virtual void DoRun (void);
};

QdChannelTestCaseInput::QdChannelTestCaseInput ()
  : TestCase ("QdChannelTestCaseInput")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
QdChannelTestCaseInput::~QdChannelTestCaseInput ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
QdChannelTestCaseInput::DoRun (void)
{
  // Creating nodes
  // Code breaks (and thus the test fails) if not all positions from
  // Output/Ns3/NodesPosition/NodesPosition.csv are associated to a node
  NodeContainer nodes;
  nodes.Create (2);

  Ptr<MobilityModel> mob0 = CreateObject<ConstantPositionMobilityModel> ();
  mob0->SetPosition (Vector (5, 0.1, 1.5));
  Ptr<MobilityModel> mob1 = CreateObject<ConstantPositionMobilityModel> ();
  mob1->SetPosition (Vector (5, 0.1, 2.9));

  nodes.Get (0)->AggregateObject (mob0);
  nodes.Get (1)->AggregateObject (mob1);

  // Create the channel model
  std::string qdFilesPath = "contrib/qd-channel/model/QD/"; // The path of the folder with the QD scenarios
  std::string scenario = "Indoor1"; // The name of the scenario
  Ptr<QdChannelModel> qdChannel = CreateObject<QdChannelModel> (qdFilesPath, scenario);

  // tests
  NS_TEST_ASSERT_MSG_EQ_TOL (qdChannel->GetQdSimTime ().GetSeconds (), 15.665, 1e-9, "Checking simulation time");
  NS_TEST_ASSERT_MSG_EQ_TOL (qdChannel->GetFrequency (), 60e9, 1, "Checking simulation frequency");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class QdChannelTestSuite : public TestSuite
{
public:
  QdChannelTestSuite ();
};

QdChannelTestSuite::QdChannelTestSuite ()
  : TestSuite ("qd-channel-test-suite", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new QdChannelTestCaseInput, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static QdChannelTestSuite sqdChannelTestSuite;
