#ifndef ATHENA_PAXOS_CLIENT_HXX
# define ATHENA_PAXOS_CLIENT_HXX

# include <elle/With.hh>

# include <reactor/Scope.hh>
# include <reactor/scheduler.hh>

namespace athena
{
  namespace paxos
  {
    /*-----.
    | Peer |
    `-----*/

    template <typename T, typename Version, typename ClientId>
    Client<T, Version, ClientId>::Peer::Peer(ClientId id)
      : _id(id)
    {}

    template <typename T, typename Version, typename ClientId>
    void
    Client<T, Version, ClientId>::Peer::print(std::ostream& output) const
    {
      elle::fprintf(output, "%s(%s)", elle::type_info(*this), this->id());
    }

    /*-------------.
    | Construction |
    `-------------*/

    template <typename T, typename Version, typename ClientId>
    Client<T, Version, ClientId>::Client(ClientId id, Peers peers)
      : _id(id)
      , _peers(std::move(peers))
      , _round(0)
    {
      ELLE_ASSERT(!this->_peers.empty());
    }

    class TooFewPeers
      : public elle::Error
    {
    public:
      TooFewPeers(int effective, int total)
        : elle::Error(
          elle::sprintf(
            "too few peers are available to reach consensus: %s of %s",
            effective, total))
      {}

      TooFewPeers(elle::serialization::SerializerIn& input)
        : elle::Error(input)
      {}
    };

    /*----------.
    | Consensus |
    `----------*/

    template <typename T, typename Version, typename ClientId>
    boost::optional<typename Client<T, Version, ClientId>::Accepted>
    Client<T, Version, ClientId>::choose(elle::Option<T, Quorum> const& value)
    {
      return this->choose(Version(), std::move(value));
    }

    template <typename T, typename Version, typename ClientId>
    void
    Client<T, Version, ClientId>::_check_headcount(Quorum const& q,
                                                   int reached) const
    {
      ELLE_LOG_COMPONENT("athena.paxos.Client");
      ELLE_DEBUG("reached %s peers", reached);
      auto size = signed(q.size());
      if (reached <= size / 2)
      {
        ELLE_TRACE("too few peers to reach consensus: %s of %s", reached, size);
        throw TooFewPeers(reached, size);
      }
    }

    template <typename C, typename F>
    void
    for_each_parallel(C const& c, F const& f)
    {
      elle::With<reactor::Scope>() << [&] (reactor::Scope& scope)
      {
        for (auto& elt: c)
          scope.run_background(
            elle::sprintf("%s: %s",
                          reactor::scheduler().current()->name(),
                          elt),
            [&] { f(elt, scope); });
        reactor::wait(scope);
      };
    }

    template <typename T, typename Version, typename ClientId>
    boost::optional<typename Client<T, Version, ClientId>::Accepted>
    Client<T, Version, ClientId>::choose(
      typename elle::_detail::attribute_r_type<Version>::type version,
      elle::Option<T, Quorum> const& value)
    {
      ELLE_LOG_COMPONENT("athena.paxos.Client");
      ELLE_TRACE_SCOPE("%s: choose %s", *this, printer(value));
      Quorum q;
      for (auto const& peer: this->_peers)
        q.insert(peer->id());
      ELLE_DUMP("quorum: %s", q);
      boost::optional<Accepted> previous;
      while (true)
      {
        ++this->_round;
        Proposal proposal(std::move(version), this->_round, this->_id);
        ELLE_DEBUG("%s: send proposal: %s", *this, proposal)
        {
          int reached = 0;
          for_each_parallel(
            this->_peers,
            [&] (std::unique_ptr<Peer> const& peer,
                 reactor::Scope&) -> void
            {
              try
              {
                ELLE_DEBUG_SCOPE("%s: send proposal %s to %s",
                                 *this, proposal, *peer);
                if (auto p = peer->propose(q, proposal))
                  if (!previous || previous->proposal < p->proposal)
                  {
                    ELLE_DEBUG_SCOPE("%s: value already accepted at %s: %s",
                                     *this, p->proposal, printer(p->value));
                    previous = std::move(p);
                  }
                ++reached;
              }
              catch (typename Peer::Unavailable const& e)
              {
                ELLE_TRACE("%s: peer %s unavailable: %s",
                           *this, peer, e.what());
              }
            });
          this->_check_headcount(q, reached);
          if (previous)
          {
            ELLE_DEBUG("replace value with %s", printer(previous->value));
            if (previous->proposal.version > version)
            {
              ELLE_DEBUG("replace with newer version: %s",
                         printer(previous->value));
              version = previous->proposal.version;
              this->_round = 0;
            }
          }
        }
        ELLE_DEBUG("%s: send acceptation", *this)
        {
          int reached = 0;
          bool conflicted = false;
          for_each_parallel(
            this->_peers,
            [&] (std::unique_ptr<Peer> const& peer,
                 reactor::Scope& scope) -> void
            {
              try
              {
                ELLE_DEBUG_SCOPE("%s: send acceptation %s to %s",
                                 *this, proposal, *peer);
                auto minimum = peer->accept(
                  q, proposal, previous ? previous->value : value);
                // FIXME: If the majority doesn't conflict, the value was
                // still chosen - right ? Take that in account.
                if (proposal < minimum)
                {
                  ELLE_DEBUG("%s: conflicted proposal on peer %s: %s",
                             *this, peer, minimum);
                  this->_round = minimum.round;
                  conflicted = true;
                  scope.terminate_now();
                }
                ++reached;
              }
              catch (typename Peer::Unavailable const& e)
              {
                ELLE_TRACE("%s: peer %s unavailable: %s",
                           *this, peer, e.what());
              }
            });
          if (conflicted)
          {
            // FIXME: random wait to avoid livelock
            ELLE_DEBUG("%s: conflicted proposal, retry", *this);
            continue;
          }
          else
            this->_check_headcount(q, reached);
        }
        ELLE_TRACE("%s: chose %s", *this,
                   printer(previous ? previous->value : value));
        ELLE_DEBUG("%s: send confirmation", *this)
        {
          auto reached = 0;
          for_each_parallel(
            this->_peers,
            [&] (std::unique_ptr<Peer> const& peer,
                 reactor::Scope&) -> void
            {
              try
              {
                ELLE_DEBUG_SCOPE("%s: send confirmation %s to %s",
                                 *this, proposal, *peer);
                peer->confirm(q, proposal);
                ++reached;
              }
              catch (typename Peer::Unavailable const& e)
              {
                ELLE_TRACE("%s: peer %s unavailable: %s",
                           *this, peer, e.what());
              }
            });
          this->_check_headcount(q, reached);
        }
        break;
      }
      return previous;
    }

    template <typename T, typename Version, typename ClientId>
    boost::optional<T>
    Client<T, Version, ClientId>::get()
    {
      ELLE_LOG_COMPONENT("athena.paxos.Client");
      ELLE_TRACE_SCOPE("%s: get value", *this);
      Quorum q;
      for (auto const& peer: this->_peers)
        q.insert(peer->id());
      ELLE_DUMP("quorum: %s", q);
      auto reached = 0;
      boost::optional<typename Client<T, Version, ClientId>::Accepted> res;
      for_each_parallel(
        this->_peers,
        [&] (std::unique_ptr<Peer> const& peer,
             reactor::Scope&) -> void
        {
          try
          {
            ELLE_DEBUG_SCOPE("%s: get from %s", *this, *peer);
            auto accepted = peer->get(q);
            if (accepted)
              if (!res || res->proposal < accepted->proposal)
                res.emplace(std::move(accepted.get()));
            ++reached;
          }
          catch (typename Peer::Unavailable const& e)
          {
            ELLE_TRACE("%s: peer %s unavailable: %s",
                       *this, peer, e.what());
          }
        });
      this->_check_headcount(q, reached);
      if (res)
        return res->value.template get<T>();
      else
        return {};
    }

    /*----------.
    | Printable |
    `----------*/

    template <typename T, typename Version, typename ClientId>
    void
    Client<T, Version, ClientId>::print(std::ostream& output) const
    {
      elle::fprintf(output, "paxos::Client(%f)", this->_id);
    }

    /*------------.
    | Unavailable |
    `------------*/

    template <typename T, typename Version, typename ClientId>
    Client<T, Version, ClientId>::Peer::Unavailable::Unavailable()
      : elle::Error("paxos peer unavailable")
    {}
  }
}

#endif
