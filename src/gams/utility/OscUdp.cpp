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
      bundle << osc::BeginMessage(i.first.c_str)
        << i.second[0] << i.second[1]
        << osc::EndMessage;
    }
  }

  bundle << osc::EndBundle;
  
  return bundle.Size();
}


void
gams::utility::OscUdp::unpack (char* buffer, size_t size, OscMap & map)
{
  // @Alex Rozgo. I've removed all of this and changed the function signature
  // to be more generic
}

int
gams::utility::OscUdp::receive (OscMap & values, double max_wait_seconds)
{
  int result = -1;
  boost::asio::ip::udp::endpoint remote;
  size_t bytes_read;

  if (has_socket())
  {
    madara::utility::EpochEnforcer<madara::utility::Clock> enforcer(
      0.0, max_wait_seconds);

    do
    {
      result = transport_.get()->receive_buffer(
        buffer_.get(), bytes_read, remote);

      if (result == 0 && bytes_read != 0)
      {
        madara_logger_ptr_log(gams::loggers::global_logger.get(),
          gams::loggers::LOG_MAJOR,
          "gams::utility::OscUdp::receive: " \
          " received %zu bytes. Calling unpack\n",
          bytes_read);

        unpack(buffer_.get(), bytes_read, values);
      }
    } while (result == 0 && !enforcer.is_done());
  }

  madara_logger_ptr_log(gams::loggers::global_logger.get(),
    gams::loggers::LOG_MAJOR,
    "gams::utility::OscUdp::receive: " \
    " returning result %d with %zu values\n",
    result, values.size());

  return result;
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
