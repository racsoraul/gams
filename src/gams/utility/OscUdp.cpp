/**
 * Copyright (c) 2019 James Edmondson. All Rights Reserved.
 *
 **/

/**
 * @file OscUdp.h
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains a helper class for sending/receiving Open
 * Stage Control messages
 **/

#include "OscUdp.h"
#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/UdpSocket.h"

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

class OSCPacketListener : public osc::OscPacketListener
{
  protected:
    virtual void ProcessMessage(const osc::ReceivedMessage &m,
                                const IpEndpointName &remoteEndpoint)
    {
        (void)remoteEndpoint; // suppress unused parameter warning

        try
        {
            if (std::strcmp(m.AddressPattern(), "/agent/0/pos") == 0)
            {
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                float x, y, z;
                args >> x >> y >> z >> osc::EndMessage;

                std::cout << "pos from /agent/0: "
                             "x = " << x << " y = " << y << " z = " << z << "\n";
            }
            else if (std::strcmp(m.AddressPattern(), "/agent/0/rot") == 0)
            {
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                float x, y, z;
                args >> x >> y >> z >> osc::EndMessage;

                std::cout << "rot from /agent/0: "
                             "x = " << x << " y = " << y << " z = " << z << "\n";
            }
        }
        catch (osc::Exception &e)
        {
            // any parsing errors such as unexpected argument types, or
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                      << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};

size_t
gams::utility::OscUdp::pack (osc::OutboundPacketStream bundle, size_t output_buffer_size, const OscMap& map)
{
  bundle << osc::BeginBundle();

  for(auto i : map)
  {
    if (
        madara::utility::ends_with (i.first, "/velocity/xy") ||
        madara::utility::ends_with (i.first, "/velocity/z") ||
        madara::utility::ends_with (i.first, "/yaw")
      ) {
      bundle << osc::BeginMessage(i.first.c_str())
        << i.second[0] << i.second[1]
        << osc::EndMessage;
    }
  }

  bundle << osc::EndBundle;
  
  return bundle.Size();
}

int
gams::utility::OscUdp::receive (OscMap & dest, double max_wait_seconds)
{
  gams::utility::OscUdp::OscMap source_map;
  
  //TODO: insert data into the `dest` map.
  OSCPacketListener listener;
    UdpListeningReceiveSocket socket(
        IpEndpointName(IpEndpointName::ANY_ADDRESS, 8000),
        &listener);
  
  std::cout << "press ctrl-c to end\n";

  socket.RunUntilSigInt();

  return 0;
}

#define ADDRESS "127.0.0.1"
#define PORT 5555
#define OUTPUT_BUFFER_SIZE 1024

int
gams::utility::OscUdp::send (const OscMap & values)
{
  UdpTransmitSocket transmitSocket(IpEndpointName(ADDRESS, PORT));

  char buffer[OUTPUT_BUFFER_SIZE];
  osc::OutboundPacketStream bundle(buffer, OUTPUT_BUFFER_SIZE);

  pack(bundle, OUTPUT_BUFFER_SIZE, values);

  transmitSocket.Send(bundle.Data(), bundle.Size());
}
