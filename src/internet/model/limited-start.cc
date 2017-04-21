/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
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

#include "tcp-congestion-ops.h"
#include "limited-start.h"
#include "ns3/log.h"
#include "ns3/tcp-socket-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LimitedStart");
NS_OBJECT_ENSURE_REGISTERED (LimitedStart);

TypeId
LimitedStart::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LimitedStart")
    .SetParent<TcpNewReno> ()
    .SetGroupName ("Internet")
    .AddConstructor<LimitedStart> ()
  ;
  return tid;
}

LimitedStart::LimitedStart ()
  : TcpNewReno (),
  m_maxSsThresh (100.0)
{
  NS_LOG_FUNCTION (this);
}

LimitedStart::LimitedStart (const LimitedStart &sock)
  : TcpNewReno (sock),
  m_maxSsThresh (sock.m_maxSsThresh)
{
  NS_LOG_FUNCTION (this);
}

LimitedStart::~LimitedStart ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
LimitedStart::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);
  
  NS_LOG_INFO ("maxSSThresh = " << m_maxSsThresh ); 
    
  if (segmentsAcked >= 1)
    {
      int k;
      k = (int)(tcb->m_cWnd/(double)(0.5*m_maxSsThresh*tcb->m_segmentSize));
      NS_LOG_INFO ("k = " << k);
      tcb->m_cWnd += (int)(tcb->m_segmentSize/k);
      NS_LOG_INFO ("In Limited SlowStart, updated to cwnd " << (int)(tcb->m_cWnd/tcb->m_segmentSize)
      << " ssthresh " << (int)(tcb->m_ssThresh/tcb->m_segmentSize));
      return segmentsAcked - 1;
    }

  return 0;
}

void
LimitedStart::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (((tcb->m_cWnd <= m_maxSsThresh*tcb->m_segmentSize) && 
     (tcb->m_cWnd < tcb->m_ssThresh)) ||
     ((tcb->m_ssThresh < m_maxSsThresh*tcb->m_segmentSize) && 
     (tcb->m_cWnd <= m_maxSsThresh*tcb->m_segmentSize ) && 
     (tcb->m_cWnd < tcb->m_ssThresh)))
    {
      NS_LOG_INFO ("Entering Slow Start");
      segmentsAcked = TcpNewReno::SlowStart (tcb, segmentsAcked);
    }
  else if (m_maxSsThresh*tcb->m_segmentSize < tcb->m_cWnd && tcb->m_cWnd < tcb->m_ssThresh)
    {
      NS_LOG_INFO ("Entering Limited Slow Start ");
      segmentsAcked = LimitedStart::SlowStart (tcb, segmentsAcked);
    }

  else if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      TcpNewReno::CongestionAvoidance (tcb, segmentsAcked);
    }

}

Ptr<TcpCongestionOps>
LimitedStart::Fork ()
{
  return CopyObject<LimitedStart> (this);
}

std::string
LimitedStart::GetName () const
{
  return "LimitedStart";
}

} // namespace ns3

