#ifndef HORIZON_MACOSX_FUKER_HH
# define HORIZON_MACOSX_FUKER_HH

# ifndef FUSE_USE_VERSION
#  define FUSE_USE_VERSION               26
# endif

# include <elle/types.hh>
# include <elle/system/Platform.hh>

# include <elle/idiom/Close.hh>
#  define _GNU_SOURCE
#  include <fuse.h>
#  if defined(HAVE_SETXATTR)
#   include <sys/xattr.h>
#  endif
#  include <pthread.h>
# include <elle/idiom/Open.hh>

namespace horizon
{
  namespace macosx
  {

    ///
    /// this class abstracts the notion of FUSE-Broker i.e FUker.
    ///
    /// the broker is responsible for converting the spawn thread-based
    /// FUSE upcalls into events for the main event loop.
    ///
    /// XXX[rewrite the part below since it has changed: now fibers are
    ///     directly spawned]
    /// the way to do this is to inject an event into the event loop by
    /// passing the upcall argument through the Event instance. then,
    /// the upcall-specific thread blocks on a semaphore because it must
    /// not return befor the event has been treated. once the event treated,
    /// i.e through the Broker, the semaphore is unlocked, enabling the
    /// thread to resume and terminate by returning to the kernel the
    /// result of the operation.
    ///
    class FUker
    {
    public:
      //
      // static methods
      //
      static elle::Status       Initialize();
      static void run();
      static void*              Setup(void*);
      static elle::Status       Clean();

      //
      // static attributes
      //
      static ::pthread_t        Thread;
      static ::fuse*            FUSE;
    };
  }
}

#endif
