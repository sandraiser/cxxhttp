/* asio.hpp Generic Client
 *
 * A generic asynchronous client template using asio.hpp.
 *
 * See also:
 * * Project Documentation: https://ef.gy/documentation/cxxhttp
 * * Project Source Code: https://github.com/ef-gy/cxxhttp
 * * Licence Terms: https://github.com/ef-gy/cxxhttp/blob/master/COPYING
 *
 * @copyright
 * This file is part of the cxxhttp project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 */
#if !defined(CXXHTTP_CLIENT_H)
#define CXXHTTP_CLIENT_H

#include <cxxhttp/network.h>

namespace cxxhttp {
namespace net {
/* Basic asynchronous client wrapper
 * @session The session type.
 * @requestProcessor The functor class to handle requests.
 *
 * Contains code that connects to a given endpoint and establishes a session for
 * the duration of that connection.
 */
template <typename session, typename requestProcessor>
class client : public connection<session, requestProcessor> {
 public:
  using connection = net::connection<session, requestProcessor>;
  using typename connection::transport;

  /* Initialise with IO service
   * @endpoint Endpoint for the socket to bind.
   * @pClients The set of clients to register with.
   * @pio IO service to use.
   * @logfile A stream to write log messages to.
   *
   * Default constructor which binds an IO service to a socket endpoint that was
   * passed in. The socket is bound asynchronously.
   */
  client(
      endpointType<transport> &endpoint,
      efgy::beacons<client> &pClients = efgy::global<efgy::beacons<client>>(),
      service &pio = efgy::global<service>(), std::ostream &logfile = std::cout)
      : connection(pio, logfile), target(endpoint), beacon(*this, pClients) {
    startConnect();
  }

 protected:
  /* Target endpoint.
   *
   * This is where we want to connect to.
   */
  endpointType<transport> target;

  /* Client beacon.
   *
   * Registration in this set is handled automatically in the constructor.
   */
  efgy::beacon<client> beacon;

  /* Connect to the socket.
   * @newSession An optional session to reuse.
   *
   * This function creates a new, blank session and attempts to connect to the
   * given socket.
   */
  void startConnect(session *newSession = 0) {
    if (newSession == 0) {
      newSession = connection::getSession();
    }
    if (!newSession) {
      newSession = new session(*this);
    }

    newSession->socket.lowest_layer().async_connect(
        target, [newSession, this](const std::error_code &error) {
          handleConnect(newSession, error);
        });
  }

  /* Handle new connection
   * @newSession The blank session object that was created by startConnect().
   * @error Describes any error condition that may have occurred.
   *
   * Called by asio.hpp when a new outbound connection has been accepted; this
   * allows the session object to begin interacting with the new session.
   */
  void handleConnect(session *newSession, const std::error_code &error) {
    connection::pending = false;

    if (error) {
      delete newSession;
    } else {
      newSession->start();
    }
  }
};
}
}

#endif
