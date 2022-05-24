/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 SIGNET Lab, Department of Information Engineering,
 * University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This example shows how to configure the QdChannelModel channel model to
 * compute the SNR between two nodes.
 * The default scenario (Indoor1) is shown.
 * Each node hosts a SimpleNetDevice and has a 2x2 antenna array.
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/qd-channel-model.h"
#include "ns3/uniform-planar-array.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/simple-net-device.h"
#include "ns3/node-container.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lte-spectrum-value-helper.h"
#include "ns3/qd-channel-utils.h"

NS_LOG_COMPONENT_DEFINE ("QdChannelModelExample");

using namespace ns3;

// Simulation parameters
double txPow = 20.0; // Tx power in dBm
double noiseFigure = 9.0; // Noise figure in dB
uint32_t timeRes = 5; // Time resolution in milliseconds

// main variables
Ptr<QdChannelModel> qdChannel;
Ptr<MobilityModel> txMob;
Ptr<MobilityModel> rxMob;
Ptr<PhasedArrayModel> txAntenna;
Ptr<PhasedArrayModel> rxAntenna;
Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel;

/**
 * Perform the beamforming using the SVD beamforming method
 * \param txDevice the device performing the beamforming
 * \param txAntenna the antenna object associated to txDevice
 * \param rxDevice the device towards which point the beam
 * \param rxAntenna the antenna object associated to rxDevice
 */
static void DoBeamforming (Ptr<NetDevice> txDevice, Ptr<PhasedArrayModel> txAntenna,
                           Ptr<NetDevice> rxDevice, Ptr<PhasedArrayModel> rxAntenna);
/*
 * Compute the average SNR, print it to both terminal and file
 */
static void ComputeSnr ();

int
main (int argc, char *argv[])
{
  std::string qdFilesPath = "contrib/qd-channel/model/QD/"; // The path of the folder with the QD scenarios
  std::string scenario = "Indoor1"; // The name of the scenario

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);

  // Create the tx and rx nodes
  NodeContainer nodes;
  nodes.Create (2);
  Ptr<Node> txNode = nodes.Get (0);
  Ptr<Node> rxNode = nodes.Get (1);

  // Create the tx and rx devices
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();

  // Associate the nodes and the devices
  txNode->AddDevice (txDev);
  txDev->SetNode (txNode);
  rxNode->AddDevice (rxDev);
  rxDev->SetNode (rxNode);

  // Create the tx and rx mobility models
  // Set the positions to be equal to the initial positions of the nodes in the ray tracer
  rxMob = CreateObject<ConstantPositionMobilityModel> ();
  rxMob->SetPosition (Vector (5, 0.1, 1.5));
  txMob = CreateObject<ConstantPositionMobilityModel> ();
  txMob->SetPosition (Vector (5, 0.1, 2.9));

  // Assign the mobility models to the nodes
  txNode->AggregateObject (txMob);
  rxNode->AggregateObject (rxMob);

  // Create the QdChannelModel
  qdChannel = CreateObject<QdChannelModel> (qdFilesPath, scenario);
  Time simTime = qdChannel->GetQdSimTime ();

  // Create the spectrum propagation loss model
  spectrumLossModel = CreateObjectWithAttributes<ThreeGppSpectrumPropagationLossModel> ("ChannelModel", PointerValue (qdChannel));

  // Create the antenna objects and set their dimensions
  txAntenna = CreateObjectWithAttributes<UniformPlanarArray> ("NumColumns", UintegerValue (2),
                                                              "NumRows", UintegerValue (2));
  txNode->AggregateObject (txAntenna);

  rxAntenna = CreateObjectWithAttributes<UniformPlanarArray> ("NumColumns", UintegerValue (2),
                                                              "NumRows", UintegerValue (2));
  rxNode->AggregateObject (rxAntenna);

  // Initialize the devices in the ThreeGppSpectrumPropagationLossModel
  spectrumLossModel->AddDevice (txDev, txAntenna);
  spectrumLossModel->AddDevice (rxDev, rxAntenna);

  // Compute and print SNR
  Simulator::ScheduleNow (&ComputeSnr);

  Simulator::Stop (simTime);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

/*  UTILITIES */
static void
DoBeamforming (Ptr<NetDevice> txDevice, Ptr<PhasedArrayModel> txAntenna, 
               Ptr<NetDevice> rxDevice, Ptr<PhasedArrayModel> rxAntenna)
{
  Ptr<MobilityModel> thisMob = txDevice->GetNode ()->GetObject<MobilityModel> ();
  Ptr<MobilityModel> otherMob = rxDevice->GetNode ()->GetObject<MobilityModel> ();
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix =
      qdChannel->GetChannel (thisMob, otherMob, txAntenna, rxAntenna);

  auto bfVectors = ComputeSvdBeamformingVectors (channelMatrix);

  // store the antenna weights
  txAntenna->SetBeamformingVector (std::get<0> (bfVectors));
  rxAntenna->SetBeamformingVector (std::get<1> (bfVectors));
}

static void
ComputeSnr ()
{
  // Create the tx PSD using the LteSpectrumValueHelper
  // 100 RBs corresponds to 18 MHz (1 RB = 180 kHz)
  // EARFCN 100 corresponds to 2125.00 MHz
  std::vector<int> activeRbs0 (100);
  for (int i = 0; i < 100; i++)
    {
      activeRbs0[i] = i;
    }
  Ptr<SpectrumValue> txPsd =
      LteSpectrumValueHelper::CreateTxPowerSpectralDensity (2100, 100, txPow, activeRbs0);
  Ptr<SpectrumValue> rxPsd = txPsd->Copy ();
  NS_LOG_DEBUG ("Average tx power " << 10 * log10 (Sum (*txPsd) * 180e3) << " dB");

  // Create the noise PSD
  Ptr<SpectrumValue> noisePsd =
      LteSpectrumValueHelper::CreateNoisePowerSpectralDensity (2100, 100, noiseFigure);
  NS_LOG_DEBUG ("Average noise power " << 10 * log10 (Sum (*noisePsd) * 180e3) << " dB");

  // compute beamforming vectors
  Ptr<NetDevice> txDevice = txMob->GetObject<Node> ()->GetDevice (0);
  Ptr<NetDevice> rxDevice = rxMob->GetObject<Node> ()->GetDevice (0);

  DoBeamforming (txDevice, txAntenna, rxDevice, rxAntenna);

  // Apply the fast fading and the beamforming gain
  rxPsd = spectrumLossModel->CalcRxPowerSpectralDensity (rxPsd, txMob, rxMob);
  NS_LOG_DEBUG ("Average rx power " << 10 * log10 (Sum (*rxPsd) * 180e3) << " dB");

  // Compute the SNR
  NS_LOG_DEBUG ("Average SNR " << 10 * log10 (Sum (*rxPsd) / Sum (*noisePsd)) << " dB");

  // Print the SNR and pathloss values in the snr-trace.txt file
  std::ofstream f;
  f.open ("snr-trace.txt", std::ios::out | std::ios::app);
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << 10 * log10 (Sum (*rxPsd) / Sum (*noisePsd)));
  f << Simulator::Now ().GetSeconds () << "\t" << 10 * log10 (Sum (*rxPsd) / Sum (*noisePsd)) << std::endl;
  f.close ();

  Simulator::Schedule (MilliSeconds (timeRes), &ComputeSnr);
}