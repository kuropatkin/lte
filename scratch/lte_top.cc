/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-helper.h"

#include <iostream>
#include <fstream>

using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("TcpEchoExample");

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */


int
main (int argc, char *argv[])
{
  double simTime = 10;
  //double interPacketInterval = 100;
  FlowMonitorHelper fl;
  Ptr<FlowMonitor> monitor;
  LogComponentEnable ("TcpEchoExample", LOG_LEVEL_INFO);
  LogComponentEnable ("TcpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("TcpEchoServerApplication", LOG_LEVEL_ALL);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(0, 0, 0));
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // Assign IP address to UEs, and install applications
  Ptr<Node> ueNode = ueNodes.Get (0);
 // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Attach one UE per eNodeB
  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
  // side effect: the default EPS bearer will be activated


  NS_LOG_INFO("Create Server Application.");
  uint16_t port = 7; // well-known echo port number.
  TcpEchoServerHelper echoServer (port);
  echoServer.SetAttribute ("MaxPackets", UintegerValue (5));
  echoServer.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  echoServer.SetAttribute ("PacketSize", UintegerValue (183));
  ApplicationContainer serverApps = echoServer.Install (remoteHostContainer.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));


  // Create a TcpEchoClient application to send TCP packet to server.
  NS_LOG_INFO("Create Client Application.");
  TcpEchoClientHelper echoClient (remoteHostAddr, port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (183));
  ApplicationContainer clientApps = echoClient.Install (ueNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (20.0));

  lteHelper->EnableTraces ();
  AsciiTraceHelper ascii;
  //p2ph.EnablePcapAll("output/lte-epc");
  monitor = fl.Install(ueNodes);
  monitor = fl.Install(remoteHostContainer);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue(0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue(0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue(20));
  std::cout<<"Setting up flow monitor... done !"<<std::endl;

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  monitor->SerializeToXmlFile ("results.xml" , true, true );
  Simulator::Destroy();
  return 0;

}
