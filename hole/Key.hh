#ifndef HOLE_KEY_HH
# define HOLE_KEY_HH

# include <QtGlobal>
# include "hole/FreeList.hh"

class QDataStream;

namespace hole
{
  struct Key : public FreeList<Key>
  {
    quint8 data[160];
  };
  QDataStream & operator<<(QDataStream & ds, const Key & key);
}

#endif /* !HOLE_KEY_HH */
