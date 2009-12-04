//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit (c)
//
// file          /home/mycure/infinit/etoile/core/State.hh
//
// created       julien quintard   [fri aug 21 22:37:24 2009]
// updated       julien quintard   [tue dec  1 02:27:32 2009]
//

#ifndef ETOILE_CORE_STATE_HH
#define ETOILE_CORE_STATE_HH

namespace etoile
{
  namespace core
  {

//
// ---------- enumerations ----------------------------------------------------
//

    ///
    /// this enumeration is used to specify if an block has been modified;
    /// hence update needs to be pushed into the network.
    ///
    enum State
      {
	StateClean = 0,
	StateDirty
      };

  }
}

#endif
