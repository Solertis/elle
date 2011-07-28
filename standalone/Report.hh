//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// file          /home/mycure/infinit/elle/standalone/Report.hh
//
// created       julien quintard   [sun oct 28 19:12:38 2007]
// updated       julien quintard   [wed jul 27 09:25:25 2011]
//

#ifndef ELLE_STANDALONE_REPORT_HH
#define ELLE_STANDALONE_REPORT_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/core/String.hh>
#include <elle/core/Void.hh>

#include <elle/radix/Status.hh>
#include <elle/radix/Meta.hh>

#include <elle/package/Archivable.hh>

#include <elle/io/Dumpable.hh>

#include <elle/concurrency/Phase.hh>

#include <elle/idiom/Close.hh>
# include <sstream>
# include <list>
# include <ostream>
# include <iostream>
# include <string.h>
# include <assert.h>
# include <errno.h>
# include <stdlib.h>
# include <time.h>
#include <elle/idiom/Open.hh>

namespace elle
{

  namespace package
  {

//
// ---------- forward declarations --------------------------------------------
//

    ///
    /// the Archive must be forward declared to prevent conflicts.
    ///
    class Archive;
  }

  namespace concurrency
  {

//
// ---------- forward declarations --------------------------------------------
//

    ///
    /// the Fiber class must be forward declared to prevent conflicts.
    ///
    class Fiber;
  }

  using namespace core;
  using namespace radix;
  using namespace package;
  using namespace io;
  using namespace concurrency;

  namespace standalone
  {

//
// ---------- forward declarations --------------------------------------------
//

    ///
    /// the region needs to be forward declared.
    ///
    class Region;

//
// ---------- class -----------------------------------------------------------
//

    ///
    /// this classes represents a log report, composed of a set of
    /// error messages, along with the location of the error and
    /// the time at which the error occured.
    ///
    /// note that contrary to the standalone philosophy, this class
    /// actually inherits a few classes outside of standalone. note however
    /// that these classes are interfaces which depend on almost anything
    /// themselves.
    ///
    /// this class also provides object-like capabilities such as
    /// assignment, copy and so on.
    ///
    /// noteworthy is that, contrary to objects, the Report class does
    /// not derive Identifiable or Fileable because doing so whould result
    /// in many conflicts.
    ///
    class Report:
      public Meta,
      public virtual Dumpable, public virtual Archivable
    {
    public:
      //
      // structures
      //

      ///
      /// this structure represent an entry in the log.
      ///
      /// note that everything is kept simple to prevent relying on
      /// other functionalities on the library. for instance the time
      /// is represented as a String rather that a Time object.
      ///
      struct Entry
      {
      public:
	//
	// attributes
	//
	String		location;
	String		time;
	String		message;
      };

      //
      // types
      //
      typedef std::list<Entry*>			Container;
      typedef Container::iterator		Iterator;
      typedef Container::const_iterator		Scoutor;

      //
      // static methods
      //
      static Status	Initialize();
      static Status	Clean();

      static Status	Instance(Report*&);

      static Status	Govern(const Phase&,
			       Fiber*&);

      //
      // static attributes
      //
      static Report*		Current;

      //
      // constructors & destructors
      //
      Report();
      Report(const Report&);
      ~Report();

      //
      // methods
      //
      Void		Flush();

      Void		Record(const String&,
			       const String&,
			       const String&);
      Void		Record(const Report&);

      //
      // interfaces
      //

      // dumpable
      Status		Dump(const Natural32 = 0) const;

      // archivable
      Status		Serialize(Archive&) const;
      Status		Extract(Archive&);

      // object-like
      template <typename T>
      Status		Recycle(const T* = NULL);

      virtual Status	Imprint(Natural32&) const;
      virtual Status	Clone(Report*&) const;

      virtual Boolean	operator==(const Report&) const;
      virtual Boolean	operator<(const Report&) const;
      virtual Boolean	operator>(const Report&) const;

      virtual Report&	operator=(const Report&);
      virtual Boolean	operator!=(const Report&) const;
      virtual Boolean	operator<=(const Report&) const;
      virtual Boolean	operator>=(const Report&) const;

      //
      // attributes
      //
      Container		container;
    };

  }
}

//
// ---------- templates -------------------------------------------------------
//

#include <elle/standalone/Report.hxx>

#endif
