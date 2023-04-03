/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef QD_CHANNEL_MODEL_H
#define QD_CHANNEL_MODEL_H

#include <complex.h>
#include <map>
#include "ns3/angles.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/boolean.h"
#include <ns3/matrix-based-channel-model.h>
#include <ns3/three-gpp-channel-model.h>

namespace ns3 {

class PhasedArrayModel;
class MatrixBasedChannelModel;
class MobilityModel;

/**
 * \ingroup spectrum
 *
 */
class QdChannelModel : public MatrixBasedChannelModel
{
public:
  /**
   * Constructor
   *
   * \param path folder path containing the scenario of interest
   * \param scenario scenario folder name, containg the Input/ and the Output/Ns3/ folders
   */
  QdChannelModel (std::string path = "", std::string scenario = "");

  /**
   * Destructor
   */
  virtual ~QdChannelModel () override;

  /**
   * Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Returns a matrix with a realization of the channel between
   * the nodes with mobility objects passed as input parameters.
   *
   * \param aMob mobility model of the a device
   * \param bMob mobility model of the b device
   * \param aAntenna antenna of the a device
   * \param bAntenna antenna of the b device
   * \return the channel matrix
   */
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> GetChannel (Ptr<const MobilityModel> aMob,
                                                                Ptr<const MobilityModel> bMob,
                                                                Ptr<const PhasedArrayModel> aAntenna,
                                                                Ptr<const PhasedArrayModel> bAntenna) override;

      /**
     * Looks for the channel params associated to the aMob and bMob pair in
     * m_channelParamsMap. If not found it will return a nullptr.
     *
     * \param aMob mobility model of the a device
     * \param bMob mobility model of the b device
     * \return the channel params
     */
    Ptr<const MatrixBasedChannelModel::ChannelParams> GetParams(Ptr<const MobilityModel> aMob,
                                       Ptr<const MobilityModel> bMob) const override;

  /*
   * Set the folder path containing the scenario of interest
   *
   * \param path folder path containing the scenario of interest
   */
  void SetPath (std::string path);

  /*
   * Get the folder path of the scenario of interest
   *
   * \return folder path containing the scenario of interest
   */
  std::string GetPath () const;

  /*
   * Set the scenario folder name, containg the Input/ and the Output/Ns3/ folders.
   * The scenario has to be set only after the Path has already been set.
   * This triggers the import of the scenario QD files.
   *
   * \param scenario folder name, containg the Input/ and the Output/Ns3/ folders
   */
  void SetScenario (std::string scenario);

  /*
   * Get the scenario folder name, containg the Input/ and the Output/Ns3/ folders.
   *
   * \return scenario folder name, containg the Input/ and the Output/Ns3/ folders
   */
  std::string GetScenario () const;


  /**
   * Just a dummy setter for compatibility reasons.
   * NOTE: the carrier frequency is imported from the QD input
   * files. This setter should not be manually used, and it is here
   * only because attributes are required to have a setter.
   * 
   * \param fc the center frequency in Hz
   */
  void SetFrequency (double fc);

  /**
   * Returns the center frequency
   * 
   * \return the center frequency in Hz
   */
  double GetFrequency (void) const;

  /**
   * Get the total simulation time
   * \return the simulation time considered in the qd files
   */
  Time GetQdSimTime () const;

private:
  using RtIdToNs3IdMap_t = std::map<uint32_t, uint32_t>;
  using Ns3IdToRtIdMap_t = std::map<uint32_t, uint32_t>;

  /**
   * Read paraCfgCurrent.txt file and imports necessary member variables
   */
  void ReadParaCfgFile (void);

  /**
   * Get the channel matrix between a and b using the ray tracer data
   *
   * \param aMob mobility model of the a device
   * \param bMob mobility model of the b device
   * \param aAntenna antenna of the a device
   * \param bAntenna antenna of the b device
   * \return the channel matrix
   */
  Ptr<MatrixBasedChannelModel::ChannelMatrix> GetNewChannel (Ptr<const MobilityModel> aMob,
                                                                   Ptr<const MobilityModel> bMob,
                                                                   Ptr<const PhasedArrayModel> aAntenna,
                                                                   Ptr<const PhasedArrayModel> bAntenna);

  /**
   * Check if the channel matrix has to be updated
   * \param channelMatrix channel matrix
   * \return true if the channel matrix has to be updated, false otherwise
   */
  bool ChannelMatrixNeedsUpdate (Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix) const;

  /**
   * Get qd-channel time-step of current time
   * \return qd-channel time-step of current time
   */
  uint64_t GetTimestep (void) const;

  /**
   * Get qd-channel time-step
   * \param t time to convert in timestep
   * \return qd-channel time-step
   */
  uint64_t GetTimestep (Time t) const;

  /**
   * Read all the configuration files
   */
  void ReadAllInputFiles ();

  /**
   * Read all NodesPosition for the given scenario
   */
  RtIdToNs3IdMap_t ReadNodesPosition (void);

  /**
   * Read all QdFiles for the given scenario
   * \param rtIdToNs3IdMap a map between user file name to ns-3 user ID
   */
  void ReadQdFiles (RtIdToNs3IdMap_t rtIdToNs3IdMap);

  /**
   * Get the list of QD file names in the given path
   *
   * \param pattern glob pattern for QD files
   * \return list of QD file names
   */
  std::vector<std::string> GetQdFilesList (const std::string &pattern);

  /**
   * Parse numerice CSV string
   *
   * \param str CSV-formatted string
   * \param toRad if true, convert from degrees to radians, otherwise, do nothing
   * \return vector of parsed numeric values
   */
  std::vector<double> ParseCsv (const std::string &str, bool toRad = false);

  /**
   * Trim folder name in order to avoid '/' at the beginning of the file name
   * and have exactly one '/' at the end
   *
   * \param folder string containing the folder name
   */
  static void TrimFolderName (std::string &folder);

  /*
   * Structure containing information parsed from QdFiles
   */
  struct QdInfo
  {
    uint64_t numMpcs;
    std::vector<double> delay_s;
    std::vector<double> pathGain_dbpow;
    std::vector<double> phase_rad;
    std::vector<double> elAod_rad;
    std::vector<double> azAod_rad;
    std::vector<double> elAoa_rad;
    std::vector<double> azAoa_rad;
  };

  std::map<uint32_t, Ptr<MatrixBasedChannelModel::ChannelMatrix>>
      m_channelMap; //!< map containing the channel realizations indexed by channel key
  std::map<uint64_t, Ptr<MatrixBasedChannelModel::ChannelParams>> m_channelParamsMap;
  Time m_updatePeriod; //!< the channel update period
  uint32_t m_totTimesteps; //!< total number of timesteps for the simulation
  Time m_totalTimeDuration; //!< duration of the simulation
  double m_frequency; /**< the operating frequency [Hz]. 
                           This value should NOT be manually set by the user, 
                           as the frequency is parsed from the channel traces instead. */
  std::vector<Vector3D> m_nodePositionList; //!< initial position of each node

  std::map<uint32_t, std::vector<QdInfo>>
      m_qdInfoMap; //!< map containing QD-related information for each node pair
  Ns3IdToRtIdMap_t
      m_ns3IdToRtIdMap; //!< map containing a conversion from ns-3 node id to qd-realization node id

  std::string m_path; //!< folder path containing the scenario of interest
  std::string m_scenario; //!< scenario folder name, containg the Input/ and the Output/Ns3/ folders
};

} // namespace ns3

#endif /* QD_CHANNEL_MODEL_H */
