#ifndef NUCLEUS_PROTON_NODULE_HXX
# define NUCLEUS_PROTON_NODULE_HXX

# include <nucleus/proton/Handle.hh>
# include <nucleus/proton/Footprint.hh>

# include <elle/log.hh>

namespace nucleus
{
  namespace proton
  {

//
// ---------- static methods --------------------------------------------------
//

    ///
    /// this method takes a given seam _right_ and moves inlets from the _left_
    /// seam in order to transfer an amount equal or greater that _size_
    /// bytes.
    ///
    /// note that the _right_ seam is supposed to contain higher keys
    /// so that the inlets from the _left_ seam with the highest keys
    /// are moved to _right_ in order to maintain consistency.
    ///
    /// XXX comments -seam +nodule
    /// XXX redundant with Value::Export (Catalog etc.)
    ///
    template <typename T>
    template <typename X>
    void
    Nodule<T>::transfer_right(X* left,
                              X* right,
                              Extent const size)
    {
      ELLE_LOG_COMPONENT("infinit.nucleus.proton.Nodule");

      static Footprint const initial = elle::serialize::footprint<X>();
      auto end = left->container().end();
      auto i = left->container().begin();
      Extent const extent = left->nest().limits().extent();
      Footprint footprint(initial);

      ELLE_TRACE_SCOPE("transfer_right(%s, %s, %s)",
                       left, right, size);

      assert(left->nest().limits().extent() == right->nest().limits().extent());

      // go through the left seam's inlets until the future size is reached
      // after which all the remaining inlets will be moved to the right
      // seam.
      for (; i != end; ++i)
        {
          typename X::I* inlet = i->second;

          // check whether the container's size has been reached.
          if (footprint >= size)
            break;

          //
          // otherwise, leave this inlet in the left seam.
          //

          // note however that another check is performed in order to make
          // sure that adding this inlet will not make the container too large.
          if ((footprint + inlet->footprint()) > extent)
            break;

          // add the inlet's footprint to the footprint.
          footprint += inlet->footprint();
        }

      // go through the remaining inlets in order to move them to
      // the right seam.
      for (auto j = i; j != end; ++j)
        {
          typename X::I* inlet = j->second;

          // substract the inlet's footprint from the left seam since
          // it is getting moved to the right one.
          left->footprint(left->footprint() - inlet->footprint());

          // insert the inlet into the right seam.
          right->insert(inlet);

          // make sure the operation is valid.
          assert(inlet->capacity() <= left->capacity());

          // adjust the capacities.
          left->capacity(left->capacity() - inlet->capacity());
          right->capacity(right->capacity() + inlet->capacity());
        }

      // remove the moved inlets from the this seam.
      left->container().erase(i, end);
    }

    ///
    /// this method does the opposite of Export(), moving at least _size_
    /// bytes of inlets from the _right_ seam to the _left_ one.
    ///
    /// note that the inlets with the lowest keys are moved in order to
    /// maintain consistency.
    ///
    /// XXX comments -seam +nodule
    ///
    template <typename T>
    template <typename X>
    void
    Nodule<T>::transfer_left(X* left,
                             X* right,
                             Extent const size)
    {
      ELLE_LOG_COMPONENT("infinit.nucleus.proton.Nodule");

      static Footprint const initial = elle::serialize::footprint<X>();
      auto rend = right->container().rend();
      auto i = right->container().rbegin();
      Extent const extent = left->nest().limits().extent();
      Footprint footprint(initial);

      ELLE_TRACE_SCOPE("transfer_left(%s, %s, %s)",
                       left, right, size);

      assert(left->nest().limits().extent() == right->nest().limits().extent());

      // go through the right seam's inlets until the future size is reached
      // after which all the remaining inlets will be moved to the left seam.
      for (; i != rend; ++i)
        {
          typename X::I* inlet = i->second;

          // check whether the container's size has been reached.
          if (footprint >= size)
            break;

          //
          // otherwise, leave this inlet in the seam.
          //

          // note however that another check is performed in order to make
          // sure that adding this inlet will not make the container too large.
          if ((footprint + inlet->footprint()) > extent)
            break;

          // add the inlet's footprint to the footprint.
          footprint += inlet->footprint();
        }

      // go through the remaining inlets in order to move them to
      // the left seam.
      for (auto j = i; j != rend; ++j)
        {
          typename X::I* inlet = j->second;

          // substract the inlet's footprint from the right seam since
          // it is getting moved to the left one.
          right->footprint(right->footprint() - inlet->footprint());

          // insert the inlet into the left seam.
          left->insert(inlet);

          // make sure the operation is valid.
          assert(inlet->capacity() <= right->capacity());

          // adjust the capacities.
          left->capacity(left->capacity() + inlet->capacity());
          right->capacity(right->capacity() - inlet->capacity());
        }

      // remove the moved inlets from the right nodule.
      right->container().erase(right->container().begin(), i.base());
    }

    ///
    /// XXX si X = Seam<T> alors X::V = Nodule<T> (Quill<T>)
    ///     si X = Quill<T> alors X::V = T
    ///     => X::V = Value or ChildType
    ///
    template <typename T>
    template <typename X>
    void
    Nodule<T>::optimize(X* nodule,
                        typename T::K const& k)
    {
      ELLE_LOG_COMPONENT("infinit.nucleus.proton.Nodule");

      static Footprint const initial = elle::serialize::footprint<X>();
      typename X::I* inlet;

      ELLE_TRACE_SCOPE("optimize(%s, %s)", nodule, k);

      // look up the entry responsible for this key.
      auto iterator = nodule->lookup_iterator(k);

      // retrieve the inlet.
      inlet = iterator->second;

      Ambit<typename X::V> current(nodule->nest(), inlet->value());

      // load the current child nodule.
      current.load();

      // check if the future nodule's footprint---i.e should the inlet
      // be inserted---would exceed the extent.
      if ((current()->footprint()) > nodule->nest().limits().extent())
        {
          //
          // in this case, the nodule needs to be split.
          //
          Handle orphan;
          struct
          {
            typename T::K current;
            typename T::K newright;
          } mayor;
          struct
          {
            Capacity newright;
          } capacity;
          struct
          {
            State newright;
          } state;

          ELLE_TRACE("nodule's extent high limit reached: "
                     "%s > %s",
                     current()->footprint(),
                     nodule->nest().limits().extent());

          // split the current current nodule.
          orphan = current()->split();

          Ambit<typename X::V> newright(nodule->nest(), orphan);

          // load the new right nodule.
          newright.load();

          //
          // first, retrieve the mayor for both the current and new
          // right nodules.
          //

          mayor.current = current()->mayor();
          mayor.newright = newright()->mayor();

          // also retrieve the _newright_ capacity and state.
          capacity.newright = newright()->capacity();
          state.newright = newright()->state();

          // unload the new right nodule.
          newright.unload();

          //
          // now, should the current current mayor have changed,
          // update the current seam so as to reference it properly.
          //
          if (inlet->key() != mayor.current)
            nodule->refresh(iterator, mayor.current);

          // update the current's capacity.
          inlet->capacity(current()->capacity());

          // unload the current child nodule.
          current.unload();

          //
          // at this point, the new i.e orphan nodule must be inserted
          // into the current seam.
          //

          // create an inlet.
          auto il =
            std::unique_ptr<typename X::I>(
              new typename X::I(mayor.newright, orphan));

          // insert the inlet to the left nodule.
          nodule->insert(il.get());

          // update the inlet with the proper capacity and state.
          il.get()->capacity(capacity.newright);
          il.get()->state(state.newright);

          // release the tracking.
          il.release();
        }
      else if (current()->footprint() <
               (nodule->nest().limits().extent() *
                nodule->nest().limits().balancing()))
        {
          ELLE_TRACE("nodule's extent low limit reached: "
                     "%s < %s",
                     current()->footprint(),
                     nodule->nest().limits().extent() *
                     nodule->nest().limits().balancing());

          if (current()->empty() == true)
            {
              ELLE_TRACE("nodule is empty");

              //
              // if the nodule became empty, it can as well be removed
              // from the current seam.
              //

              // at this point, the capacity of current nodule must
              // be zero.
              assert(inlet->capacity() == 0);
              assert(current()->capacity() == 0);

              // finally, since the current child is now empty and unreferenced,
              // it needs to be detached from the porcupine so its memory
              // gets released.
              //
              // note that the block is unloaded at the end of this method.
              nodule->nest().detach(current.handle());

              // unload the current nodule.
              current.unload();

              // delete the entry from the current seam.
              //
              // note that this must be done after the handle has been
              // used for both detaching the block and unloading the handle.
              nodule->erase(iterator);
            }
          else
            {
              ELLE_TRACE("nodule not empty but could be balanced");

              //
              // the nodule may not be empty but could have reached a size
              // small enough for the tree to be simplified.
              //
              // indeed, neighbour nodes could be balanced in order to better
              // support future insertions and deletions or could even be
              // merged in order to potentially reduce the tree complexity
              // and height.
              //
              typename X::Iterator::Forward previous = iterator;
              typename X::Iterator::Forward next = iterator;

              // move the iterators so as select the previous and next entries
              // within the current seam.
              previous--;
              next++;

              //
              // if the iterators are valid, consider it.
              //
              Handle l;
              Handle r;

              if ((iterator != nodule->container().begin()) &&
                  (previous != nodule->container().end()))
                l = previous->second->value();

              if (next != nodule->container().end())
                r = next->second->value();

              //
              // load the left/right nodules.
              //
              Ambit<typename X::V> left(nodule->nest(), l);
              Ambit<typename X::V> right(nodule->nest(), r);

              // XXX expliquer que la on se base sur le pointeur
              // plutot que le handle.

              if (left.handle() != Handle::Null)
                {
                  ELLE_TRACE("left neighbour present");
                  left.load();
                }

              if (right.handle() != Handle::Null)
                {
                  ELLE_TRACE("right neighbour present");
                  right.load();
                }

              // XXX expliquer que plus bas on reteste le left/right.handle pour
              // savoir si le neighbour existe et donc a ete loade avant d'essayer
              // d'y acceder.

              //
              // act depending on the context i.e the left node has enough space,
              // or the right exists and has enough space etc.
              //
              // note that, usually, the handles are compared in order to know
              // whether a nodule exists. however, in the present case, the left
              // and right nodules have been loaded anyway so that the pointers
              // can be compared more efficiently.
              //
              if ((left.handle() != Handle::Null) &&
                  (left()->state() == StateDirty) &&
                  ((left()->footprint() + current()->footprint() - initial) <
                   nodule->nest().limits().extent()))
                {
                  //
                  // this case is a specialization of another one below in
                  // which merging is privileged should the neighbour nodule
                  // be already dirty.
                  //
                  // this scenario is considered first because it can reduce
                  // the number of modified nodules i.e blocks which would
                  // then need to be pushed onto the network.
                  //
                  goto _merge_left;
                }
              else if ((right.handle() != Handle::Null) &&
                       (right()->state() == StateDirty) &&
                       ((current()->footprint() - initial + right()->footprint()) <
                        nodule->nest().limits().extent()))
                {
                  //
                  // this case is similar to the previous but for the right
                  // neighbour nodule.
                  //
                  goto _merge_right;
                }
              else if ((left.handle() != Handle::Null) &&
                       ((left()->footprint() + current()->footprint() - initial) <
                        nodule->nest().limits().extent()))
                {
                  //
                  // in this case the left neighbour nodule seems to be present.
                  //
                _merge_left:

                  ELLE_TRACE("merging with the left neighbour");

                  // merge the nodule.
                  left()->merge(current.handle());

                  // update the inlet referencing left.
                  typename X::I* il = previous->second;
                  il->capacity(left()->capacity());
                  il->state(left()->state());

                  // at this point, current is empty and unreferenced.
                  // therefore, detach the block from the porcupine.
                  //
                  // note that the block is finally unloaded at the end of
                  // this method.
                  nodule->nest().detach(current.handle());

                  // unload the current nodule.
                  current.unload();

                  // delete the entry from the current seam.
                  //
                  // note that this must be done after the handle has been
                  // used for both detaching the block and unloading the handle.
                  nodule->erase(iterator);

                  // finally, update the current seam so as to reference the new
                  // mayor key for the left nodule since it has received
                  // all the entries of the current child.
                  nodule->refresh(previous, left()->mayor());
                }
              else if ((right.handle() != Handle::Null) &&
                       ((current()->footprint() - initial +
                         right()->footprint()) <
                        nodule->nest().limits().extent()))
                {
                  //
                  // this case is identical to the previous one except that
                  // merging occurs between the current and right nodules.
                  //
                _merge_right:

                  ELLE_TRACE("merging with the right neighbour");

                  // merge the nodule.
                  right()->merge(current.handle());

                  // update the inlet referencing right.
                  typename X::I* il = next->second;
                  il->capacity(right()->capacity());
                  il->state(right()->state());

                  // at this point, current is empty and unreferenced.
                  // therefore, detach the block from the porcupine.
                  //
                  // note that the block is finally unloaded at the end of
                  // this method.
                  nodule->nest().detach(current.handle());

                  // unload the current nodule.
                  current.unload();

                  // delete the entry from the current seam.
                  //
                  // note that this must be done after the handle has been
                  // used for both detaching the block and unloading the handle.
                  nodule->erase(iterator);

                  // note that the current seam does not need to be updated
                  // regarding the new inlets merged into the right nodule
                  // because the added inlets have lower keys than the already
                  // existing ones in the right nodule.
                }
              else
                {
                  //
                  // in this case, the nodule is small enough for being merged
                  // or balanced but, unfortunately, the neighbours nodules
                  // are not in a state to accept such operations or do not
                  // belong to the same seam.
                  //
                  // therefore, the nodule is left as such, taking care however
                  // to update the current seam for consistency.
                  //

                  // update the current seam so as to reference the new
                  // mayor key.
                  nodule->refresh(iterator, current()->mayor());

                  // unload the current nodule.
                  current.unload();
                }

              //
              // unload the left/right nodules, if necessary.
              //

              if (left.handle() != Handle::Null)
                left.unload();

              if (right.handle() != Handle::Null)
                right.unload();
            }
        }
      else
        current.unload();
    }

//
// ---------- dumpable --------------------------------------------------------
//

    ///
    /// this method dumps the nodule.
    ///
    template <typename T>
    elle::Status
    Nodule<T>::Dump(const elle::Natural32 margin) const
    {
      elle::String alignment(margin, ' ');

      std::cout << alignment << "[Nodule]" << std::endl;

      if (Node::Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the node");

      return elle::Status::Ok;
    }

  }
}

//
// ---------- serialize -------------------------------------------------------
//

# include <elle/serialize/Serializer.hh>

ELLE_SERIALIZE_SIMPLE_T1(nucleus::proton::Nodule,
                         archive,
                         value,
                         version)
{
  enforce(version == 0);

  archive & base_class<nucleus::proton::Node>(value);
}

#endif
