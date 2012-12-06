#include <nucleus/proton/Handle.hh>
#include <nucleus/proton/Porcupine.hh>

namespace nucleus
{
  namespace proton
  {

    /*----------.
    | Functions |
    `----------*/

    /// This method returns the default secret key to be used whenever
    /// a new handle is created.
    static
    cryptography::SecretKey
    generate_secret()
    {
      /// This constant represents the hard-coded length of the secrets
      /// used to encrypt the blocks composing porcupines.
      /// XXX in bytes & to move in Contents where the encryption actually
      ///     takes place?
      static const elle::Natural32 secret_length = 256;
      // XXX[pass info so as to parameter the future size of the key]
      static cryptography::SecretKey secret{
        cryptography::SecretKey::generate(secret_length)};

      return (secret);
    }

    /*--------.
    | Globals |
    `--------*/

    // XXX[box]
    Handle Handle::Null; // XXX[use a Valid section]

//
// ---------- constructors & destructors --------------------------------------
//

    Handle::Handle():
      _address(Address::null())
    {
    }

    Handle::Handle(Address const& address,
                   cryptography::SecretKey const& secret):
      _address(address),
      _secret(secret)
    {
    }

    Handle::Handle(Placement const& placement,
                   Address const& address,
                   cryptography::SecretKey const& secret):
      _placement(placement),
      _address(address),
      _secret(secret)
    {
    }

    Handle::Handle(Placement const& placement,
                   Address const& address):
      _placement(placement),
      _address(address),
      _secret(generate_secret())
      // XXX[Porcupine::secret() n'est appelle que de la. il pourrait etre
      //     specifique dans Nest]
    {
    }

    /// XXX[needless if it does not do anything special!]
    /// the copy constructor.
    ///
    /// note that the block is not retrieved from the other in order
    /// to force the cloned handle to be loaded before being used.
    ///
    Handle::Handle(Handle const& other):
      _placement(other.placement()),
      _address(other.address()),
      _secret(other.secret())
    {
    }

//
// ---------- methods ---------------------------------------------------------
//

    Placement const&
    Handle::placement() const
    {
      return (this->_placement);
    }

    Address const&
    Handle::address() const
    {
      return (this->_address);
    }

    void
    Handle::address(Address const& address)
    {
      assert(this->_placement != Placement::Null);

      this->_address = address;
    }

    cryptography::SecretKey const&
    Handle::secret() const
    {
      return (this->_secret);
    }

    void
    Handle::secret(cryptography::SecretKey const& secret)
    {
      this->_secret = secret;
    }

//
// ---------- object ----------------------------------------------------------
//

    Handle&
    Handle::operator=(Handle const& object)
    {
      if (this == &object)
        return (*this);

      this->~Handle();
      new (this) Handle(object);

      return (*this);
    }

    elle::Boolean
    Handle::operator!=(Handle const& object) const
    {
      return (!(this->operator==(object)));
    }

    elle::Boolean
    Handle::operator==(Handle const& other) const
    {
      // check the address as this may actually be the same object.
      if (this == &other)
        return true;

      // compare the placements, if possible.
      if ((this->_placement != Placement::Null) &&
          (other.placement() != Placement::Null))
        {
          //
          // in this case, both handles reference blocks which have been
          // retrieved from the network.
          //
          // however, that does not necessarily mean that the blocks have
          // been loaded in main memory. therefore, the block addresses
          // cannot be used for comparing the blocks.
          //
          // thus, the placements are compared as representing unique
          // identifiers.
          //

          if (this->_placement != other.placement())
            return false;
        }
      else
        {
          //
          // otherwise, one of the two handles has not been retrieved from
          // the network.
          //
          // if both blocks have been created, they would both have had
          // a placement. as a result, one or more of the handles represent
          // a block which resides on the network storage layer.
          //
          // since a created block would have a special address (i.e
          // Address::some) which would differ from normal addresses,
          // one can rely on the address field for comparing the handles.
          //
          // in other words, addresses could be null in which case,
          // if both are, they would be equal and everything would be fine.
          //
          // if both addresses are set to 'some', then, the handles would
          // represent created blocks in which case both would have a placement
          // and we would not be discussing in the branch of the condition.
          //
          // finally, if one handle is null or 'some' and the other one is
          // not, then, the comparison will result in handles being different
          // which they are.
          //

          if (this->_address != other.address())
            return false;
        }

      return true;
    }

//
// ---------- dumpable --------------------------------------------------------
//

    elle::Status            Handle::Dump(const elle::Natural32      margin) const
    {
      elle::String          alignment(margin, ' ');

      std::cout << alignment << "[Handle]" << std::endl;

      // dump the placement.
      if (this->_placement.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the placement");

      // dump the address.
      if (this->_address.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the address");

      // dump the secret.
      if (this->_secret.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the secret");

      return elle::Status::Ok;
    }

    /*----------.
    | Printable |
    `----------*/

    void
    Handle::print(std::ostream& stream) const
    {
      stream << "("
        // XXX << this->_placement ???
        // XXX << ", "
             << this->_address
        // XXX << ", "
        // XXX ??? << this->_secret
             << ")";
    }
  }
}
