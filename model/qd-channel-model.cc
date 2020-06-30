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
 *
 */

#include "ns3/qd-channel-model.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/three-gpp-antenna-array-model.h"
#include "ns3/node.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/integer.h"
#include <algorithm>
#include <random>
#include "ns3/log.h"
#include <ns3/simulator.h>
#include "ns3/mobility-model.h"
#include <glob.h>
#include <fstream>
#include <sstream>
#include <ns3/node-list.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QdChannelModel");

NS_OBJECT_ENSURE_REGISTERED (QdChannelModel);


QdChannelModel::QdChannelModel (std::string path, std::string scenario)
{
  NS_LOG_FUNCTION (this);

  SetPath (path);
  SetScenario (scenario);
}

QdChannelModel::~QdChannelModel ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
QdChannelModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QdChannelModel")
    .SetParent<MatrixBasedChannelModel> ()
    .SetGroupName ("Spectrum")
    .AddConstructor<QdChannelModel> ()
    .AddAttribute ("Frequency",
                   "The operating Frequency in Hz. This attribute is here "
                   "only for compatibility reasons.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&QdChannelModel::SetFrequency,
                                       &QdChannelModel::GetFrequency),
                   MakeDoubleChecker<double> ());

  return tid;
}

std::vector<std::string>
QdChannelModel::GetQdFilesList (const std::string& pattern)
{
  NS_LOG_FUNCTION (this << pattern);

  glob_t glob_result;
  glob (pattern.c_str (), GLOB_TILDE, NULL, &glob_result);
  std::vector<std::string> files;
  for (uint32_t i = 0; i < glob_result.gl_pathc; ++i)
    {
      files.push_back (std::string (glob_result.gl_pathv[i]));
    }
  globfree (&glob_result);
  return files;
}

std::vector<double>
QdChannelModel::ParseCsv (const std::string& str)
{
  NS_LOG_FUNCTION (this << str);

  std::stringstream ss (str);
  std::vector<double> vect{};

  for (double i; ss >> i;)
    {
      vect.push_back (i);
      if (ss.peek () == ',')
        {
          ss.ignore ();
        }
    }

  return vect;
}

QdChannelModel::RtIdToNs3IdMap_t
QdChannelModel::ReadNodesPosition ()
{
  NS_LOG_FUNCTION (this);

  std::string posFileName {m_path + m_scenario + "Output/Ns3/NodesPosition/NodesPosition.csv"};
  std::ifstream posFile {posFileName.c_str ()};
  NS_ABORT_MSG_IF (!posFile.good (), posFileName + " not found");

  std::string line{};
  uint32_t id {0};
  QdChannelModel::RtIdToNs3IdMap_t rtIdToNs3IdMap;
  while (std::getline (posFile, line))
    {
      std::istringstream ss{line};
      std::string pos;
      std::getline (ss, pos, ',');
      double x = ::atof (pos.c_str ());
      std::getline (ss, pos, ',');
      double y = ::atof (pos.c_str ());
      std::getline (ss, pos, ',');
      double z = ::atof (pos.c_str ());
      Vector3D nodePosition {x,y,z};
      m_nodePositionList.push_back (nodePosition);
      bool found {false};
      uint32_t matchedNodeId;
      for (NodeList::Iterator nit = NodeList::Begin (); nit != NodeList::End (); ++nit)
        {
          Ptr<MobilityModel> mm = (*nit)->GetObject<MobilityModel> ();
          if (mm != 0)
            {
              // TODO automatically import nodes' initial positions to avoid manual setting every time the scenario changes
              Vector3D pos = mm->GetPosition ();
              if (x == pos.x && y == pos.y && z == pos.z)
                {
                  found = true;
                  matchedNodeId = (*nit)->GetId ();
                  NS_LOG_LOGIC ("got a match " << pos << " ID " << matchedNodeId);
                  break;
                }
            }
        }
      NS_ABORT_MSG_IF (!found, "Position not matched - did you install the mobility model before the channel is created");

      rtIdToNs3IdMap.insert (std::make_pair (id, matchedNodeId));
      m_ns3IdToRtIdMap.insert (std::make_pair (matchedNodeId, id));

      ++id;
    }

  for (auto elem : m_nodePositionList)
    {
      NS_LOG_LOGIC (elem);
    }

  return rtIdToNs3IdMap;
}

void
QdChannelModel::ReadParaCfgFile ()
{
  NS_LOG_FUNCTION (this);

  std::string paraCfgCurrentFileName {m_path + m_scenario + "Input/paraCfgCurrent.txt"};
  std::ifstream paraCfgCurrentFile {paraCfgCurrentFileName.c_str ()};
  NS_ABORT_MSG_IF (!paraCfgCurrentFile.good (), paraCfgCurrentFileName + " not found");

  char delimiter = '\t';
  std::string line{};
  std::string varName, varValue;

  // ignore first line
  std::getline (paraCfgCurrentFile, line);
  // input following lines
  while (std::getline (paraCfgCurrentFile, line))
    {
      std::istringstream tokenStream (line);
      std::getline (tokenStream, varName, delimiter);
      std::getline (tokenStream, varValue, delimiter);

      if (varName.compare ("numberOfTimeDivisions") == 0)
        {
          m_totTimesteps = atoi (varValue.c_str ());
          NS_LOG_DEBUG ("numberOfTimeDivisions (int) = " << m_totTimesteps);
        }
      else if (varName.compare ("totalTimeDuration") == 0)
        {
          m_totalTimeDuration = Seconds (atof (varValue.c_str ()));
          NS_LOG_DEBUG ("m_totalTimeDuration = " << m_totalTimeDuration.GetSeconds () << " s");
        }
      else if (varName.compare ("carrierFrequency") == 0)
        {
          m_frequency = atof (varValue.c_str ());
          NS_LOG_DEBUG ("carrierFrequency (float) = " << m_frequency);
        }

    }

}

void
QdChannelModel::ReadQdFiles (QdChannelModel::RtIdToNs3IdMap_t rtIdToNs3IdMap)
{
  NS_LOG_FUNCTION (this);

  // QdFiles input
  NS_LOG_DEBUG ("m_path + m_scenario = " << m_path + m_scenario);
  auto qdFileList = GetQdFilesList (m_path + m_scenario + "Output/Ns3/QdFiles/*");
  NS_LOG_DEBUG ("qdFileList.size ()=" << qdFileList.size ());

  for (auto fileName : qdFileList)
    {
      // get the nodes IDs from the file name
      int txIndex = fileName.find ("Tx");
      int rxIndex = fileName.find ("Rx");
      int txtIndex = fileName.find (".txt");

      int len {rxIndex - txIndex - 2};
      int id_tx {::atoi (fileName.substr (txIndex + 2, len).c_str ())};
      len = txtIndex - rxIndex - 2;
      int id_rx {::atoi (fileName.substr (rxIndex + 2, len).c_str ())};

      NS_ABORT_MSG_IF (rtIdToNs3IdMap.find (id_tx) == rtIdToNs3IdMap.end (), "ID not found for TX!");
      uint32_t nodeIdTx = rtIdToNs3IdMap.find (id_tx)->second;
      NS_ABORT_MSG_IF (rtIdToNs3IdMap.find (id_rx) == rtIdToNs3IdMap.end (), "ID not found for RX!");
      uint32_t nodeIdRx = rtIdToNs3IdMap.find (id_rx)->second;

      NS_LOG_LOGIC (id_tx);
      NS_LOG_LOGIC (id_rx);

      uint32_t key = GetKey (nodeIdTx, nodeIdRx);
      // std::pair<Ptr<const MobilityModel>, Ptr<const MobilityModel>> idPair {std::make_pair(tx_mm, rx_mm)};

      std::ifstream qdFile{fileName.c_str ()};

      std::string line{};
      std::vector<QdInfo> qdInfoVector;

      while (std::getline (qdFile, line))
        {
          QdInfo qdInfo {};
          // the file has a line with the number of multipath components
          qdInfo.numMpcs = std::stoul (line, 0, 10);
          NS_LOG_LOGIC ("numMpcs " << qdInfo.numMpcs);

          if (qdInfo.numMpcs > 0)
            {
              // a line with the delays
              std::getline (qdFile, line);
              auto pathDelays = ParseCsv (line);
              NS_ABORT_MSG_IF (pathDelays.size () != qdInfo.numMpcs,
                               "mismatch between number of path delays (" << pathDelays.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.delay_s = pathDelays;
              // a line with the path gains
              std::getline (qdFile, line);
              auto pathGains = ParseCsv (line);
              NS_ABORT_MSG_IF (pathGains.size () != qdInfo.numMpcs,
                               "mismatch between number of path gains (" << pathGains.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.pathGain_dbpow = pathGains;
              // a line with the path phases
              std::getline (qdFile, line);
              auto pathPhases = ParseCsv (line);
              NS_ABORT_MSG_IF (pathPhases.size () != qdInfo.numMpcs,
                               "mismatch between number of path phases (" << pathPhases.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.phase_rad = pathPhases;
              // a line with the elev AoD
              std::getline (qdFile, line);
              auto pathElevAod = ParseCsv (line);
              NS_ABORT_MSG_IF (pathElevAod.size () != qdInfo.numMpcs,
                               "mismatch between number of path elev AoDs (" << pathElevAod.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.elAod_deg = pathElevAod;
              // a line with the azimuth AoD
              std::getline (qdFile, line);
              auto pathAzAod = ParseCsv (line);
              NS_ABORT_MSG_IF (pathAzAod.size () != qdInfo.numMpcs,
                               "mismatch between number of path az AoDs (" << pathAzAod.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.azAod_deg = pathAzAod;
              // a line with the elev AoA
              std::getline (qdFile, line);
              auto pathElevAoa = ParseCsv (line);
              NS_ABORT_MSG_IF (pathElevAoa.size () != qdInfo.numMpcs,
                               "mismatch between number of path elev AoAs (" << pathElevAoa.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.elAoa_deg = pathElevAoa;
              // a line with the azimuth AoA
              std::getline (qdFile, line);
              auto pathAzAoa = ParseCsv (line);
              NS_ABORT_MSG_IF (pathAzAoa.size () != qdInfo.numMpcs,
                               "mismatch between number of path az AoAs (" << pathAzAoa.size () <<
                               ") and number of MPCs (" << qdInfo.numMpcs <<
                               "), timestep=" << qdInfoVector.size () + 1 <<
                               ", fileName=" << fileName);
              qdInfo.azAoa_deg = pathAzAoa;
            }
          qdInfoVector.push_back (qdInfo);
        }
      NS_LOG_DEBUG ("qdInfoVector.size ()=" << qdInfoVector.size ());
      m_qdInfoMap.insert (std::make_pair (key, qdInfoVector));
    }

  NS_LOG_DEBUG ("Imported files for " << m_qdInfoMap.size () << " tx/rx pairs");
}

void
QdChannelModel::ReadAllInputFiles ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("ReadAllInputFiles for scenario " << m_scenario << " path " << m_path);

  m_ns3IdToRtIdMap.clear ();
  m_qdInfoMap.clear ();

  ReadParaCfgFile ();
  QdChannelModel::RtIdToNs3IdMap_t rtIdToNs3IdMap = ReadNodesPosition ();
  ReadQdFiles (rtIdToNs3IdMap);

  // Setup simulation timings assuming constant periodicity
  NS_ASSERT_MSG (m_totTimesteps == m_qdInfoMap.begin ()->second.size (),
                 "m_totTimesteps = " << m_totTimesteps << " != QdFiles size = " << m_qdInfoMap.begin ()->second.size ());

  m_updatePeriod = NanoSeconds ((double) m_totalTimeDuration.GetNanoSeconds () / (double) m_totTimesteps);
  NS_LOG_DEBUG ("m_totalTimeDuration=" << m_totalTimeDuration.GetSeconds () << " s"
                ", m_updatePeriod=" << m_updatePeriod.GetNanoSeconds () / 1e6 << " ms"
                ", m_totTimesteps=" << m_totTimesteps);
}


Time
QdChannelModel::GetQdSimTime () const
{
  NS_LOG_FUNCTION (this);
  return m_totalTimeDuration;
}

void
QdChannelModel::SetFrequency (double freqHz)
{
  // m_frequency = freqHz;
  NS_LOG_WARN ("This method has no effect, as the frequency is read from the QD configuration file");
}

double
QdChannelModel::GetFrequency () const
{
  NS_LOG_FUNCTION (this);
  return m_frequency;
}

void
QdChannelModel::TrimFolderName (std::string& folder)
{
  // avoid starting with multiple '/'
  while (folder.front () == '/')
    {
      folder = folder.substr (1, folder.size ());
    }

  // avoid ending with multiple '/'
  while (folder.back () == '/')
    {
      folder = folder.substr (0, folder.size () - 1);
    }

  folder += '/';
}

void
QdChannelModel::SetScenario (std::string scenario)
{
  NS_LOG_FUNCTION (this << scenario);
  NS_ABORT_MSG_IF (m_path == "", "m_path empty, use SetPath first");

  TrimFolderName (scenario);

  if (scenario != m_scenario   // avoid re-reading input files
      && scenario != "")
    {
      m_scenario = scenario;
      // read the information for this scenario
      ReadAllInputFiles ();
    }
}

std::string
QdChannelModel::GetScenario () const
{
  NS_LOG_FUNCTION (this);
  return m_scenario;
}

void
QdChannelModel::SetPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);
  TrimFolderName (path);
  m_path = path;
}

std::string
QdChannelModel::GetPath () const
{
  NS_LOG_FUNCTION (this);
  return m_path;
}


bool
QdChannelModel::ChannelMatrixNeedsUpdate (Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix) const
{
  NS_LOG_FUNCTION (this << channelMatrix);

  uint64_t nowTimestep = GetTimestep ();
  uint64_t lastChanUpdateTimestep = GetTimestep (channelMatrix->m_generatedTime);

  NS_ASSERT_MSG (nowTimestep >= lastChanUpdateTimestep, "Current timestep=" << nowTimestep << ", last channel update timestep=" << lastChanUpdateTimestep);

  bool update = false;
  // if the coherence time is over the channel has to be updated
  if (lastChanUpdateTimestep < nowTimestep)
    {
      NS_LOG_DEBUG ("Generation time " << channelMatrix->m_generatedTime.GetNanoSeconds () << " now " << Simulator::Now ().GetNanoSeconds () << " update needed");
      update = true;
    }
  else
    {
      NS_LOG_DEBUG ("Generation time " << channelMatrix->m_generatedTime.GetNanoSeconds () << " now " << Simulator::Now ().GetNanoSeconds () << " update not needed");
    }

  return update;
}

Ptr<const MatrixBasedChannelModel::ChannelMatrix>
QdChannelModel::GetChannel (Ptr<const MobilityModel> aMob,
                            Ptr<const MobilityModel> bMob,
                            Ptr<const ThreeGppAntennaArrayModel> aAntenna,
                            Ptr<const ThreeGppAntennaArrayModel> bAntenna)
{
  NS_LOG_FUNCTION (this << aMob << bMob << aAntenna << bAntenna);

  // Compute the channel keys
  uint32_t aId = aMob->GetObject<Node> ()->GetId ();
  uint32_t bId = bMob->GetObject<Node> ()->GetId ();

  uint32_t channelId = GetKey (aId, bId);


  NS_LOG_DEBUG ("channelId " << channelId <<
                ", ns-3 aId=" << aId << " bId=" << bId <<
                ", RT sim. aId=" << m_ns3IdToRtIdMap[aId] << " bId=" << m_ns3IdToRtIdMap[bId]);

  // Check if the channel is present in the map and return it, otherwise
  // generate a new channel
  bool update = false;
  bool notFound = false;
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix;
  if (m_channelMap.find (channelId) != m_channelMap.end ())
    {
      // channel matrix present in the map
      NS_LOG_DEBUG ("channel matrix present in the map");
      channelMatrix = m_channelMap[channelId];

      // check if it has to be updated
      update = ChannelMatrixNeedsUpdate (channelMatrix);
    }
  else
    {
      NS_LOG_DEBUG ("channel matrix not found");
      notFound = true;
    }

  // If the channel is not present in the map or if it has to be updated
  // generate a new channel
  if (notFound || update)
    {
      NS_LOG_LOGIC ("channelMatrix notFound=" << notFound << " || update=" << update);
      channelMatrix = GetNewChannel (aMob, bMob, aAntenna, bAntenna);

      // store the channel matrix in the channel map
      m_channelMap[channelId] = channelMatrix;
    }

  return channelMatrix;
}

Ptr<const MatrixBasedChannelModel::ChannelMatrix>
QdChannelModel::GetNewChannel (Ptr<const MobilityModel> aMob,
                               Ptr<const MobilityModel> bMob,
                               Ptr<const ThreeGppAntennaArrayModel> aAntenna,
                               Ptr<const ThreeGppAntennaArrayModel> bAntenna) const
{
  NS_LOG_FUNCTION (this << aMob << bMob << aAntenna << bAntenna);

  Ptr<MatrixBasedChannelModel::ChannelMatrix> channelParams = Create<MatrixBasedChannelModel::ChannelMatrix> ();

  uint32_t timestep = GetTimestep ();
  uint32_t aId = aMob->GetObject<Node> ()->GetId ();
  uint32_t bId = bMob->GetObject<Node> ()->GetId ();
  uint32_t channelId = GetKey (aId, bId);

  QdInfo qdInfo = m_qdInfoMap.at (channelId)[timestep];

  uint64_t uSize = bAntenna->GetNumberOfElements ();
  uint64_t sSize = aAntenna->GetNumberOfElements ();

  // channel coffecient H[u][s][n];
  // considering only 1 cluster for retrocompatibility -> n=1
  MatrixBasedChannelModel::Complex3DVector H;

  H.resize (uSize);
  for (uint64_t uIndex = 0; uIndex < uSize; uIndex++)
    {
      H[uIndex].resize (sSize);
      for (uint64_t sIndex = 0; sIndex < sSize; sIndex++)
        {
          if (qdInfo.numMpcs > 0)
            {
              H[uIndex][sIndex].resize (1, std::complex<double> (0,0));
            }
          else
            {
              H[uIndex][sIndex].resize (0, std::complex<double> (0,0));
            }
        }
    }

  for (uint64_t mpcIndex = 0; mpcIndex < qdInfo.numMpcs; ++mpcIndex)
    {
      double initialPhase = -2 * M_PI * qdInfo.delay_s[mpcIndex] * m_frequency + qdInfo.phase_rad[mpcIndex];
      double pathGain = pow (10, qdInfo.pathGain_dbpow[mpcIndex] / 20);
      // double rxElementGain = bAntenna->GetRadiationPattern (qdInfo.elAoa_deg[mpcIndex], qdInfo.azAoa_deg[mpcIndex]);
      // double txElementGain = aAntenna->GetRadiationPattern (qdInfo.elAod_deg[mpcIndex], qdInfo.azAod_deg[mpcIndex]);
      double pgTimesGains = pathGain; // pathGain * rxElementGain * txElementGain;
      for (uint64_t uIndex = 0; uIndex < uSize; ++uIndex)
        {
          Vector uLoc = bAntenna->GetElementLocation (uIndex);
          double rxPhaseDiff = 2 * M_PI * (sin (qdInfo.elAoa_deg[mpcIndex] / 180 * M_PI) * cos (qdInfo.azAoa_deg[mpcIndex] / 180 * M_PI) * uLoc.x
                                           + sin (qdInfo.elAoa_deg[mpcIndex] / 180 * M_PI) * sin (qdInfo.azAoa_deg[mpcIndex] / 180 * M_PI) * uLoc.y
                                           + cos (qdInfo.elAoa_deg[mpcIndex] / 180 * M_PI) * uLoc.z);

          for (uint64_t sIndex = 0; sIndex < sSize; ++sIndex)
            {
              Vector sLoc = aAntenna->GetElementLocation (sIndex);
              // minus sign: complex conjugate for TX steering vector
              double txPhaseDiff = 2 * M_PI * (sin (qdInfo.elAod_deg[mpcIndex] / 180 * M_PI) * cos (qdInfo.azAod_deg[mpcIndex] / 180 * M_PI) * sLoc.x
                                               + sin (qdInfo.elAod_deg[mpcIndex] / 180 * M_PI) * sin (qdInfo.azAod_deg[mpcIndex] / 180 * M_PI) * sLoc.y
                                               + cos (qdInfo.elAod_deg[mpcIndex] / 180 * M_PI) * sLoc.z);

              std::complex<double> ray =  pgTimesGains * exp (std::complex<double> (0, initialPhase + rxPhaseDiff + txPhaseDiff));

              H[uIndex][sIndex][0] += ray;
            }
        }
    }

  channelParams->m_channel = H;
  channelParams->m_delay = qdInfo.delay_s;

  channelParams->m_angle.clear ();
  channelParams->m_angle.push_back (qdInfo.azAoa_deg);
  channelParams->m_angle.push_back (qdInfo.elAoa_deg);
  channelParams->m_angle.push_back (qdInfo.azAod_deg);
  channelParams->m_angle.push_back (qdInfo.elAod_deg);

  channelParams->m_generatedTime = Simulator::Now ();
  channelParams->m_nodeIds = std::make_pair (aId, bId);

  return channelParams;
}

uint64_t
QdChannelModel::GetTimestep (void) const
{
  return GetTimestep (Simulator::Now ());
}


uint64_t
QdChannelModel::GetTimestep (Time t) const
{
  NS_LOG_FUNCTION (this << t);
  NS_ASSERT_MSG (m_updatePeriod.GetNanoSeconds () > 0.0, "QdChannelModel update period not set correctly");

  uint64_t timestep = t.GetNanoSeconds () / m_updatePeriod.GetNanoSeconds ();
  NS_LOG_LOGIC ("t = " << t.GetNanoSeconds () << " ns" <<
                ", updatePeriod = " << m_updatePeriod.GetNanoSeconds () << " ns" <<
                ", timestep = " << timestep);

  NS_ASSERT_MSG (timestep <= m_totTimesteps, "Simulator is running for longer that expected: timestep > m_totTimesteps");

  return timestep;
}

}  // namespace ns3
