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


#ifndef QD_CHANNEL_UTILS_H
#define QD_CHANNEL_UTILS_H


namespace ns3 {

class MatrixBasedChannelModel;
class ThreeGppAntennaArrayModel;

/**
 * Compute the eigenvector associated to the largest eigenvalue.
 * It is based on the power iteration algorithm, with stopping criterion
 * based on both maximum number of iterations and iterative difference threshold.
 *
 * \param A complex 2D matrix
 * \param nIter maximum number of iterations
 * \param threshold difference threshold for consecutive iterations
 * \return the eigenvector associated to the largest eigenvalue
 */
ThreeGppAntennaArrayModel::ComplexVector GetFirstEigenvector (MatrixBasedChannelModel::Complex2DVector A, uint32_t nIter, double threshold);

/**
 * Compute analog SVD beamforming for a given channel matrix.
 * SVD beamforming is intended to be analog when only the left and right eigenvector
 * associated to the largest singular value are used, thus needing a single RF chain
 * per device, although with variable phases and magnitudes for each antenna element.
 * 
 * If the channel matrix is 3D (i.e., wideband, where the third dimension is the tap index),
 * the equivalent 2D narrowband channel matrix is computed by summing over the third dimension.
 *
 * \param A params
 * \return the beamforming vectors for the second and first dimension, respectively
 */
std::pair<ThreeGppAntennaArrayModel::ComplexVector, ThreeGppAntennaArrayModel::ComplexVector> ComputeSvdBeamformingVectors (Ptr<const MatrixBasedChannelModel::ChannelMatrix> params);

} // namespace ns3


#endif /* QD_CHANNEL_UTILS_H */
