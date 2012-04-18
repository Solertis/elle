//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// author        julien quintard   [wed may 25 14:20:06 2011]
//

#ifndef ELLE_NETWORK_TCPSOCKET_HXX
#define ELLE_NETWORK_TCPSOCKET_HXX

//
// ---------- includes --------------------------------------------------------
//

#include <elle/standalone/Maid.hh>
#include <elle/standalone/Report.hh>

#include <elle/network/Packet.hh>
#include <elle/network/Header.hh>
#include <elle/network/Data.hh>
#include <elle/network/Parcel.hh>

#include <elle/Manifest.hh>

namespace elle
{
  namespace network
  {

//
// ---------- methods ---------------------------------------------------------
//

    ///
    /// this method sends a packet.
    ///
    template <typename I>
    Status              TCPSocket::Send(const I                 inputs,
                                        const Event&            event)
    {
      Packet            packet;
      Data              data;
      Header            header;

      try
        {
          data.Writer() << inputs;
        }
      catch (std::exception const& err)
        {
          escape(err.what());
        }

      // create the header now that we know that final archive's size.
      if (header.Create(event,
                        inputs.tag,
                        data.Size()) == Status::Error)
        escape("unable to create the header");

      try
        {
          packet.Writer() << header << data;
        }
      catch (std::exception const& err)
        {
          escape(err.what());
        }

      // write the socket.
      if (this->Write(packet) == Status::Error)
        escape("unable to write the socket");

      return Status::Ok;
    }

    ///
    /// this method receives a packet by blocking.
    ///
    template <typename O>
    Status              TCPSocket::Receive(const Event&         event,
                                           O                    outputs)
    {
      std::shared_ptr<Parcel> parcel;

      // block the current fiber until the given event is received.
      if (Fiber::Wait(event, parcel) == Status::Error)
        escape("an error occured while waiting for a specific event");

      assert(parcel.get() != nullptr && "The event should have filled the parcel");

      // check the tag.
      if (parcel->header->tag != outputs.tag)
        {
          //
          // in this case, the blocked fiber received a message whose
          // tag does not match the expected one.
          //
          Tag           tag = parcel->header->tag;

          // first, test if the message received is an error, if so, append
          // the report to the local one.
          if (tag == TagError)
            {
              Report    report;

              parcel->data->Reader() >> report;

              // report the remote error.
              transpose(report);
            }
          else
            {
              // otherwise, try to ship the parcel since a handler may be
              // able to treat it.
              if (Socket::Ship(parcel) == Status::Error)
                log("an error occured while shipping the parcel");
            }

          // in any case, return an error from the Receive() method.
          escape("received a packet with an unexpected tag '%u'", tag);
        }

      try
        {
          parcel->data->Writer() << outputs;
        }
      catch (std::exception const& err)
        {
          escape(err.what());
        }

      return Status::Ok;
    }

    ///
    /// this method sends and waits for an appropriate response.
    ///
    template <typename I,
              typename O>
    Status              TCPSocket::Call(const I                 inputs,
                                        O                       outputs)
    {
      Event             event;

      // generate an event to link the request with the response.
      if (event.Generate() == Status::Error)
        escape("unable to generate the event");

      // send the inputs.
      if (this->Send(inputs, event) == Status::Error)
        escape("unable to send the inputs");

      // wait for the reply.
      if (this->Receive(event, outputs) == Status::Error)
        escape("unable to receive the outputs");

      return Status::Ok;
    }

    ///
    /// this method replies to the message which has just been received i.e
    /// whose tag is specified in the current session.
    ///
    template <typename I>
    Status              TCPSocket::Reply(const I                inputs,
                                         Session*               session)
    {
      // retrieve the current session, if necessary.
      if (session == NULL)
        {
          if (Session::Instance(session) == Status::Error)
            escape("unable to retrieve the session instance");
        }

      // send a message as a response by using the event of
      // the received message i.e the current session.
      if (this->Send(inputs, session->event) == Status::Error)
        escape("unable to send the reply");

      return Status::Ok;
    }

  }
}

#endif
