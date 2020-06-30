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
 * Each node hosts a SimpleNetDevice and has an antenna array with 4 elements.
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/qd-channel-model.h"
#include "ns3/three-gpp-antenna-array-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/simple-net-device.h"
#include "ns3/node-container.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lte-spectrum-value-helper.h"

NS_LOG_COMPONENT_DEFINE ("ThreeGppChannelExample");

using namespace ns3;

static Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel; //!< the SpectrumPropagationLossModel object

// Simulation parameters
double txPow = 20.0; // Tx power in dBm
double noiseFigure = 9.0; // Noise figure in dB
uint32_t timeRes = 5; // Time resolution in milliseconds

// main variables
Ptr<QdChannelModel> qdChannel;
Ptr<MobilityModel> txMob;
Ptr<MobilityModel> rxMob;
Ptr<ThreeGppAntennaArrayModel> txAntenna;
Ptr<ThreeGppAntennaArrayModel> rxAntenna;

ThreeGppAntennaArrayModel::ComplexVector
GetFirstEigenvector (MatrixBasedChannelModel::Complex2DVector A, uint32_t nIter, double threshold)
{
  ThreeGppAntennaArrayModel::ComplexVector antennaWeights;
  uint16_t arraySize = A.size ();
  for (uint16_t eIndex = 0; eIndex < arraySize; eIndex++)
    {
      antennaWeights.push_back (A[0][eIndex]);
    }


  uint32_t iter = 0;
  double diff = 1;
  while (iter < nIter && diff > threshold)
    {
      ThreeGppAntennaArrayModel::ComplexVector antennaWeightsNew;

      for (uint16_t row = 0; row < arraySize; row++)
        {
          std::complex<double> sum (0,0);
          for (uint16_t col = 0; col < arraySize; col++)
            {
              sum += A[row][col] * antennaWeights[col];
            }

          antennaWeightsNew.push_back (sum);
        }
      //normalize antennaWeights;
      double weighbSum = 0;
      for (uint16_t i = 0; i < arraySize; i++)
        {
          weighbSum += norm (antennaWeightsNew[i]);
        }
      for (uint16_t i = 0; i < arraySize; i++)
        {
          antennaWeightsNew[i] = antennaWeightsNew[i] / sqrt (weighbSum);
        }
      diff = 0;
      for (uint16_t i = 0; i < arraySize; i++)
        {
          diff += std::norm (antennaWeightsNew[i] - antennaWeights[i]);
        }
      iter++;
      antennaWeights = antennaWeightsNew;
    }
  NS_LOG_DEBUG ("antennaWeigths stopped after " << iter << " iterations with diff=" << diff << std::endl);

  return antennaWeights;
}

std::pair<ThreeGppAntennaArrayModel::ComplexVector, ThreeGppAntennaArrayModel::ComplexVector>
ComputeSvdBeamformingVectors (Ptr<const MatrixBasedChannelModel::ChannelMatrix> params)
{
  // params
  uint32_t svdIter = 30;
  double svdThresh = 1e-8;

  //generate transmitter side spatial correlation matrix
  uint16_t aSize = params->m_channel.size ();
  uint16_t bSize = params->m_channel[0].size ();
  uint16_t clusterSize = params->m_channel[0][0].size ();

  // compute narrowband channel by summing over the cluster index
  MatrixBasedChannelModel::Complex2DVector narrowbandChannel;
  narrowbandChannel.resize (aSize);

  for (uint16_t aIndex = 0; aIndex < aSize; aIndex++)
    {
      narrowbandChannel[aIndex].resize (bSize);
    }

  for (uint16_t aIndex = 0; aIndex < aSize; aIndex++)
    {
      for (uint16_t bIndex = 0; bIndex < bSize; bIndex++)
        {
          std::complex<double> cSum (0, 0);
          for (uint16_t cIndex = 0; cIndex < clusterSize; cIndex++)
            {
              cSum += params->m_channel[aIndex][bIndex][cIndex];
            }
          narrowbandChannel[aIndex][bIndex] = cSum;
        }
    }

  //compute the transmitter side spatial correlation matrix bQ = H*H, where H is the sum of H_n over n clusters.
  MatrixBasedChannelModel::Complex2DVector bQ;
  bQ.resize (bSize);

  for (uint16_t bIndex = 0; bIndex < bSize; bIndex++)
    {
      bQ[bIndex].resize (bSize);
    }

  for (uint16_t b1Index = 0; b1Index < bSize; b1Index++)
    {
      for (uint16_t b2Index = 0; b2Index < bSize; b2Index++)
        {
          std::complex<double> aSum (0,0);
          for (uint16_t aIndex = 0; aIndex < aSize; aIndex++)
            {
              aSum += std::conj (narrowbandChannel[aIndex][b1Index]) * narrowbandChannel[aIndex][b2Index];
            }
          bQ[b1Index][b2Index] += aSum;
        }
    }

  //calculate beamforming vector from spatial correlation matrix
  ThreeGppAntennaArrayModel::ComplexVector bW = GetFirstEigenvector (bQ, svdIter, svdThresh);

  //compute the receiver side spatial correlation matrix aQ = HH*, where H is the sum of H_n over n clusters.
  MatrixBasedChannelModel::Complex2DVector aQ;
  aQ.resize (aSize);

  for (uint16_t aIndex = 0; aIndex < aSize; aIndex++)
    {
      aQ[aIndex].resize (aSize);
    }

  for (uint16_t a1Index = 0; a1Index < aSize; a1Index++)
    {
      for (uint16_t a2Index = 0; a2Index < aSize; a2Index++)
        {
          std::complex<double> bSum (0,0);
          for (uint16_t bIndex = 0; bIndex < bSize; bIndex++)
            {
              bSum += narrowbandChannel[a1Index][bIndex] * std::conj (narrowbandChannel[a2Index][bIndex]);
            }
          aQ[a1Index][a2Index] += bSum;
        }
    }

  //calculate beamforming vector from spatial correlation matrix.
  ThreeGppAntennaArrayModel::ComplexVector aW = GetFirstEigenvector (aQ, svdIter, svdThresh);

  for (size_t i = 0; i < aW.size (); ++i)
    {
      aW[i] = std::conj (aW[i]);
    }

  return std::make_pair (bW, aW);
}

/**
 * Perform the beamforming using the SVD beamforming method
 * \param txDevice the device performing the beamforming
 * \param txAntenna the antenna object associated to txDevice
 * \param rxDevice the device towards which point the beam
 */
static void
DoBeamforming (Ptr<NetDevice> txDevice, Ptr<ThreeGppAntennaArrayModel> txAntenna, Ptr<NetDevice> rxDevice, Ptr<ThreeGppAntennaArrayModel> rxAntenna)
{
  Ptr<MobilityModel> thisMob = txDevice->GetNode ()->GetObject<MobilityModel> ();
  Ptr<MobilityModel> otherMob = rxDevice->GetNode ()->GetObject<MobilityModel> ();
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix = qdChannel->GetChannel (thisMob, otherMob, txAntenna, rxAntenna);

  auto bfVectors = ComputeSvdBeamformingVectors (channelMatrix);

  // store the antenna weights
  txAntenna->SetBeamformingVector (std::get<0> (bfVectors));
  rxAntenna->SetBeamformingVector (std::get<1> (bfVectors));
}

/*
 * Compute the average SNR
 * \param txMob the tx mobility model
 * \param rxMob the rx mobility model
 * \param txPow the transmitting power in dBm
 * \param noiseFigure the noise figure in dB
 */
static void
ComputeSnr ()
{
  // TODO: update to mmWave

  // Create the tx PSD using the LteSpectrumValueHelper
  // 100 RBs corresponds to 18 MHz (1 RB = 180 kHz)
  // EARFCN 100 corresponds to 2125.00 MHz
  std::vector<int> activeRbs0 (100);
  for (int i = 0; i < 100 ; i++)
  {
    activeRbs0[i] = i;
  }
  Ptr<SpectrumValue> txPsd = LteSpectrumValueHelper::CreateTxPowerSpectralDensity (2100, 100, txPow, activeRbs0);
  Ptr<SpectrumValue> rxPsd = txPsd->Copy ();
  NS_LOG_DEBUG ("Average tx power " << 10*log10(Sum (*txPsd) * 180e3) << " dB");

  // Create the noise PSD
  Ptr<SpectrumValue> noisePsd = LteSpectrumValueHelper::CreateNoisePowerSpectralDensity (2100, 100, noiseFigure);
  NS_LOG_DEBUG ("Average noise power " << 10*log10 (Sum (*noisePsd) * 180e3) << " dB");

  // compute beamforming vectors
  Ptr<NetDevice> txDevice = txMob->GetObject<Node> ()->GetDevice (0);
  Ptr<NetDevice> rxDevice = rxMob->GetObject<Node> ()->GetDevice (0);

  DoBeamforming (txDevice, txAntenna, rxDevice, rxAntenna);

  // Apply the fast fading and the beamforming gain
  rxPsd = spectrumLossModel->CalcRxPowerSpectralDensity (rxPsd, txMob, rxMob);
  NS_LOG_DEBUG ("Average rx power " << 10*log10 (Sum (*rxPsd) * 180e3) << " dB");

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

int
main (int argc, char *argv[])
{
  std::string qdFilesPath = "contrib/qd-channel/model/QD/"; // The path of the folder with the QD scenarios
  std::string scenario = "Indoor1"; // The name of the scenario
  
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

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
  txAntenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
  txNode->AggregateObject(txAntenna);

  rxAntenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
  rxNode->AggregateObject(rxAntenna);

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
