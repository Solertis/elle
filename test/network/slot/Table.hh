//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// file          /home/mycure/infinit/elle/test/network/slot/Table.hh
//
// created       julien quintard   [wed mar 17 13:07:53 2010]
// updated       julien quintard   [sun may  2 22:58:20 2010]
//

#ifndef ELLE_TEST_NETWORK_SLOT_TABLE_HH
#define ELLE_TEST_NETWORK_SLOT_TABLE_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/Elle.hh>

#include <elle/idiom/Close.hh>
# include <list>
#include <elle/idiom/Open.hh>

namespace elle
{
  namespace test
  {

//
// ---------- forward declarations --------------------------------------------
//

    class Node;
    class Neighbour;

//
// ---------- classes ---------------------------------------------------------
//

    class Table:
      public Object
    {
    public:
      //
      // types
      //
      typedef std::list<Neighbour*>		Container;
      typedef Container::iterator		Iterator;
      typedef Container::const_iterator		Scoutor;

      //
      // methods
      //
      Status		Create(Node*);

      Status		Update(const Address&,
			       const String&);
      Status		Refresh(const Address&);
      Status		Remove(const Address&);
      Status		Locate(const Address&,
			       Iterator&);

      Status		Merge(const Table&);

      //
      // interfaces
      //

      // dumpable
      Status		Dump(const Natural32 = 0) const;

      // archivable
      Status		Serialize(Archive&) const;
      Status		Extract(Archive&);

      //
      // attributes
      //
      Node*		node;

      Timer		timer;

      Container		container;

      //
      // callbacks
      //
      Status		Refresh();
    };

  }
}

//
// ---------- includes --------------------------------------------------------
//

#include <elle/test/network/slot/Node.hh>
#include <elle/test/network/slot/Neighbour.hh>

#endif
