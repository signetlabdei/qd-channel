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
 * This file contains some utility functions needed by the example script(s)
 */

#include "ns3/core-module.h"
#include "ns3/qd-channel-model.h"
#include "ns3/phased-array-model.h"
#include "ns3/qd-channel-utils.h"

NS_LOG_COMPONENT_DEFINE ("QdChannelUtils");

namespace ns3 {

PhasedArrayModel::ComplexVector
GetFirstEigenvector (MatrixBasedChannelModel::Complex2DVector A, uint32_t nIter, double threshold)
{
  PhasedArrayModel::ComplexVector antennaWeights;
  uint16_t arraySize = A.size ();
  for (uint16_t eIndex = 0; eIndex < arraySize; eIndex++)
    {
      antennaWeights.push_back (A[0][eIndex]);
    }

  uint32_t iter = 0;
  double diff = 1;
  while (iter < nIter && diff > threshold)
    {
      PhasedArrayModel::ComplexVector antennaWeightsNew;

      for (uint16_t row = 0; row < arraySize; row++)
        {
          std::complex<double> sum (0, 0);
          for (uint16_t col = 0; col < arraySize; col++)
            {
              sum += A[row][col] * antennaWeights[col];
            }

          antennaWeightsNew.push_back (sum);
        }
      // Normalize antennaWeights;
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
  // NS_LOG_DEBUG ("antennaWeigths stopped after " << iter << " iterations with diff=" << diff << std::endl);

  return antennaWeights;
}

std::pair<PhasedArrayModel::ComplexVector, PhasedArrayModel::ComplexVector>
ComputeSvdBeamformingVectors (Ptr<const MatrixBasedChannelModel::ChannelMatrix> params)
{
  // params
  uint32_t svdIter = 30;
  double svdThresh = 1e-8;

  // Generate transmitter side spatial correlation matrix
  uint16_t aSize = params->m_channel.size ();
  uint16_t bSize = params->m_channel[0].size ();
  uint16_t clusterSize = params->m_channel[0][0].size ();

  // Compute narrowband channel by summing over the tap index
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

  // Compute the transmitter side spatial correlation matrix bQ = H*H, where H is the sum of H_n over n taps.
  // The * operator is the transponse conjugate
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
          std::complex<double> aSum (0, 0);
          for (uint16_t aIndex = 0; aIndex < aSize; aIndex++)
            {
              aSum += std::conj (narrowbandChannel[aIndex][b1Index]) *
                      narrowbandChannel[aIndex][b2Index];
            }
          bQ[b1Index][b2Index] += aSum;
        }
    }

  // Calculate beamforming vector from spatial correlation matrix
  PhasedArrayModel::ComplexVector bW = GetFirstEigenvector (bQ, svdIter, svdThresh);

  // Compute the receiver side spatial correlation matrix aQ = HH*, where H is the sum of H_n over n clusters.
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
          std::complex<double> bSum (0, 0);
          for (uint16_t bIndex = 0; bIndex < bSize; bIndex++)
            {
              bSum += narrowbandChannel[a1Index][bIndex] *
                      std::conj (narrowbandChannel[a2Index][bIndex]);
            }
          aQ[a1Index][a2Index] += bSum;
        }
    }

  // Calculate beamforming vector from spatial correlation matrix.
  PhasedArrayModel::ComplexVector aW = GetFirstEigenvector (aQ, svdIter, svdThresh);

  for (size_t i = 0; i < aW.size (); ++i)
    {
      aW[i] = std::conj (aW[i]);
    }

  return std::make_pair (bW, aW);
}

} // namespace ns3