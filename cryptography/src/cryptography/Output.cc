#include <cryptography/Output.hh>

#include <elle/serialization/SerializerIn.hh>
#include <elle/serialization/SerializerOut.hh>

#include <cryptography/Input.hh>

namespace infinit
{
  namespace cryptography
  {
    /*-------------.
    | Construction |
    `-------------*/

    Output::Output(elle::Natural64 const size):
      _buffer(size)
    {
    }

    Output::Output(elle::Buffer const& buffer):
      _buffer(buffer.contents(), buffer.size())
    {
    }

    Output::Output(elle::Buffer&& buffer):
      _buffer(std::move(buffer))
    {
    }

    Output::Output(Output const& other):
      _buffer(other._buffer.contents(), other._buffer.size())
    {
    }

    Output::Output(Output&& other):
      _buffer(std::move(other._buffer))
    {
    }

    ELLE_SERIALIZE_CONSTRUCT_DEFINE(Output,
                                    _buffer)
    {
    }

    /*----------.
    | Operators |
    `----------*/

    elle::Boolean
    Output::operator ==(Output const& other) const
    {
      if (this == &other)
        return (true);

      return (this->_buffer == other._buffer);
    }

    elle::Boolean
    Output::operator <(Output const& other) const
    {
      if (this == &other)
        return (false);

      return (this->_buffer < other._buffer);
    }

    elle::Boolean
    Output::operator <=(Output const& other) const
    {
      if (this == &other)
        return (true);

      return (this->_buffer <= other._buffer);
    }

    elle::Boolean
    Output::operator ==(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) == other.buffer());
    }

    elle::Boolean
    Output::operator <(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) < other.buffer());
    }

    elle::Boolean
    Output::operator <=(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) <= other.buffer());
    }

    /*----------.
    | Printable |
    `----------*/

    void
    Output::print(std::ostream& stream) const
    {
      stream << this->_buffer;
    }

    /*--------------.
    | Serialization |
    `--------------*/

    Output::Output(elle::serialization::SerializerIn& input)
    {
      this->serialize(input);
    }

    void
    Output::serialize(elle::serialization::Serializer& s)
    {
      s.serialize("data", this->_buffer);
    }
  }
}