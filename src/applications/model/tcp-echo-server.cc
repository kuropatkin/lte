/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2012
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "tcp-echo-server.h"

#include <fstream>
#include <iostream>
using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpEchoServerApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpEchoServer);

TypeId
TcpEchoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpEchoServer")
    .SetParent<Application> ()
    .AddConstructor<TcpEchoServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming connections.",
                   UintegerValue (7),
                   MakeUintegerAccessor (&TcpEchoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TcpEchoServer::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&TcpEchoServer::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TcpEchoServer::SetDataSize,
                                         &TcpEchoServer::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TcpEchoServer::TcpEchoServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_socket6 = 0;
  m_running = false;
  m_sent = 0;
  m_bytesSent = 0;
  //m_sendEvent = EventId ();
}

TcpEchoServer::~TcpEchoServer()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_socket6 = 0;
}

void
TcpEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
TcpEchoServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_running = true;

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress listenAddress = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (listenAddress);
      m_socket->Listen();
    }
  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress listenAddress = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      m_socket6->Bind (listenAddress);
      m_socket6->Listen();
    }

  m_socket->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&TcpEchoServer::HandleAccept, this));
  m_socket6->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&TcpEchoServer::HandleAccept, this));
}

uint32_t
TcpEchoServer::GetDataSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_size;
}

void
TcpEchoServer::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (dataSize);
  m_size = dataSize;
}



void
TcpEchoServer::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_running = false;

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetAcceptCallback (
            MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
            MakeNullCallback<void, Ptr<Socket>, const Address &> () );
    }
  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetAcceptCallback (
            MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
            MakeNullCallback<void, Ptr<Socket>, const Address &> () );
    }
}
void
TcpEchoServer::ScheduleTransmit (Time dt, Ptr<Socket> s)
{
  NS_LOG_FUNCTION_NOARGS ();
  //m_sendEvent = Simulator::Schedule (dt, &TcpEchoServer::Send, this);
}

void
TcpEchoServer::Send (Ptr<Socket> s)
{
  NS_LOG_FUNCTION_NOARGS ();

  //NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  p = Create<Packet> (m_size);
  m_bytesSent += m_size;
  s->Send (p);
  Time packetTime = Simulator::Now();
  ofstream myfile;
  if (m_sent == 0){
        myfile.open ("ServerOutput.txt");   
        myfile << packetTime << "\n";
        myfile.close();
      }
  else{
        myfile.open ("ServerOutput.txt", ios_base::app);
        myfile << packetTime << "\n";
        myfile.close();
      }
  ++m_sent;

  NS_LOG_INFO ("Sending packet number " << m_sent << " ,size = " << p->GetSize () << " time " << Simulator::Now());
  
  if (m_sent < m_count)
    {
      Simulator::Schedule (m_interval, &TcpEchoServer::Send, this, s);
      //ScheduleTransmit (m_interval, s);
    }
}

void
TcpEchoServer::ReceivePacket (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  Ptr<Packet> packet;
  Address from;
  packet = s->RecvFrom (from);
  if (packet->GetSize () > 0)
        {
    	  NS_LOG_INFO ("Server Received " << packet->GetSize () << " bytes from " <<
    	                         (AddressPrinter)from);

    	  packet->RemoveAllPacketTags ();
    	  packet->RemoveAllByteTags ();
        }
  Send(s);
  //ScheduleTransmit (m_interval, s);
  /*while (m_sent < m_count){
      Ptr<Packet> p;
      p = Create<Packet> (m_size);
      m_bytesSent += m_size;
      s->Send (p);

      m_sent++;

      NS_LOG_INFO ("Echoing packet number " << m_sent << " ,size = " << p->GetSize ());
  }*/

}

void TcpEchoServer::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&TcpEchoServer::ReceivePacket, this));
  s->SetCloseCallbacks(MakeCallback (&TcpEchoServer::HandleSuccessClose, this),
    MakeNullCallback<void, Ptr<Socket> > () );
}

void TcpEchoServer::HandleSuccessClose(Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
  NS_LOG_LOGIC ("Client close received");
  s->Close();
  s->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > () );
  s->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket> > (),
      MakeNullCallback<void, Ptr<Socket> > () );
}

} // Namespace ns3
