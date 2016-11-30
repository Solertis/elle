#pragma once

#include <deque>

#include <elle/Buffer.hh>

#include <reactor/network/utp-server.hh>
#include <reactor/network/utp-socket-impl.hh>

namespace reactor
{
  namespace network
  {
    class UTPServer::Impl
      : public std::enable_shared_from_this<UTPServer::Impl>
    {
    public:
      Impl();
      ~Impl();
      std::unique_ptr<UTPSocket>
      accept();
      void
      _check_icmp();
      void
      _cleanup();
      void
      listen(EndPoint const& ep);
      void
      on_accept(utp_socket* s);
      void
      send_to(elle::ConstWeakBuffer buf, EndPoint where);
      void
      _send();
      void
      _send_cont(boost::system::error_code const&, size_t);
      /// Import from libutp/utp.h.
      using utp_context = ::struct_utp_context;
      ELLE_ATTRIBUTE(utp_context*, ctx);
      ELLE_ATTRIBUTE_R(std::unique_ptr<RDVSocket>, socket);
      ELLE_ATTRIBUTE_R(unsigned char, xorify);
      ELLE_ATTRIBUTE(std::vector<std::unique_ptr<UTPSocket>>, accept_queue);
      ELLE_ATTRIBUTE(Barrier, accept_barrier);
      ELLE_ATTRIBUTE(std::unique_ptr<Thread>, listener);
      ELLE_ATTRIBUTE(std::unique_ptr<Thread>, checker);
      ELLE_ATTRIBUTE(
        (std::deque<std::pair<elle::Buffer, EndPoint>>), send_buffer);
      ELLE_ATTRIBUTE(bool, sending);
      ELLE_ATTRIBUTE(int, icmp_fd);
      ELLE_ATTRIBUTE_RX(std::vector<Thread::unique_ptr>, socket_shutdown_threads);
      friend class UTPServer;
      friend class UTPSocket;
    };
  }
}

