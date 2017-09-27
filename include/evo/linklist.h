// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file list.h Evo List container. */
#pragma once
#ifndef INCL_evo_linklist_h
#define INCL_evo_linklist_h

// Includes
#include "evo_config.h"
#include "impl/container.h"
#include "impl/iter.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Linked list type -- see LinkList template. */
enum LinkListType {
    LINK_SINGLE = 1,
    LINK_DOUBLE
};

/** Linked list container.
*/
template<class T,LinkListType Type=LINK_DOUBLE,class TSize=SizeT>
class LinkList
{
private:
    // Special pointer for empty unallocated list (undef'd below)
    /** \cond impl */
    #define EVO__PEMPTY ((List*)1)
    /** \endcond */

    struct NodeS {
        typedef NodeS Node;

        Node* next;
        T     value;

        NodeS() {
            next = NULL;
            DataOp<T>::defvalpod(value);
        }

        NodeS(Node* newprev) {
            next = NULL;
            newprev->next = this;
            DataOp<T>::defvalpod(value);
        }

        NodeS(Node* newprev, const T& data) : value(data) {
            next = NULL;
            newprev->next = this;
        }

        NodeS(const T& data) : value(data)
            { next = NULL; }

        Node* getprev(Node* first) {
            assert( first != NULL );
            if (first != this)
                for (Node* cur=first; cur != NULL; cur = cur->next)
                    if (cur->next == this)
                        return cur;
            return NULL;
        }

        void link(Node* next)
            { next = next; }

        void linkfirst()
            { }

        void insertafter(Node* node) {
            node->next = next;
            next = node;
        }

        typedef IteratorFw< LinkList<T,Type,TSize> > Iter;
        static const LinkListType TYPE = LINK_SINGLE;
    };

    struct NodeD {
        typedef NodeD Node;

        Node* next;
        Node* prev;
        T     value;

        NodeD() {
            next = prev = NULL;
            DataOp<T>::defvalpod(value);
        }

        NodeD(Node* newprev) {
            next = NULL;
            prev = newprev;
            newprev->next = this;
            DataOp<T>::defvalpod(value);
        }

        NodeD(Node* newprev, const T& data) : value(data) {
            next = NULL;
            prev = newprev;
            newprev->next = this;
        }

        NodeD(const T& data) : value(data)
            { next = prev = NULL; }

        Node* getprev(Node*)
            { return prev; }

        void link(Node* next) {
            next = next;
            next->prev = this;
        }

        void linkfirst()
            { prev = NULL; }

        void insertafter(Node* node) {
            node->next = next;
            node->prev = this;
            if (next != NULL)
                next->prev = node;
            next = node;
        }

        typedef IteratorBi< LinkList<T,Type,TSize> > Iter;
        static const LinkListType TYPE = LINK_DOUBLE;
    };

    typedef typename StaticIf<Type == NodeD::TYPE,NodeD,NodeS>::Type Node;

public:
    EVO_CONTAINER_TYPE;
    typedef TSize               Size;           ///< List size integer type
    typedef Size                Key;            ///< Key type (item index) // TODO
    typedef T                   Value;          ///< Value type (same as Item)
    typedef T                   Item;           ///< Item type (same as Value)

    typedef LinkList<T,Type,Size> ThisType;     ///< This list type

    // Iterator support types
    /** \cond impl */
    typedef Node* IterKey;
    typedef T     IterItem;
    /** \endcond */

    typedef typename Node::Iter::Const Iter;    ///< Iterator (const) - IteratorFw (LINK_SINGLE) or IteratorBi (LINK_DOUBLE)
    typedef typename Node::Iter        IterM;   ///< Iterator (mutable) - IteratorFw (LINK_SINGLE) or IteratorBi (LINK_DOUBLE)

    static const LinkListType TYPE = Node::TYPE;    ///< Link type

    /** Default constructor sets as null. */
    LinkList()
        { list_ = NULL; }

    /** Constructor to set as empty but not null.
     \param  val  vEmpty
    */
    explicit LinkList(const ValEmpty& val)
        { list_ = EVO__PEMPTY; }

    /** Constructor to copy from array.
     \param  data   Data to copy
     \param  size   Size as item count to copy, 0 for none
    */
    LinkList(const T* data, Size size) {
        list_ = NULL;
        set(data, size);
    }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
    */
    LinkList(const ThisType& data) {
        if (data.list_ > EVO__PEMPTY) {
            if (data.list_->size > 0) {
                list_ = data.list_;
                ++list_->refs;
            } else
                list_ = EVO__PEMPTY;
        } else
            list_ = data.list_;
    }

    /** Destructor. */
    ~LinkList()
        { free(); }

    // SET

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    ThisType& clear() {
        if (list_ > EVO__PEMPTY) {
            if (list_->refs > 1) {
                --list_->refs;
                list_ = EVO__PEMPTY;
            } else {
                list_->free();
                list_->first = list_->last = NULL;
                list_->size = 0;
            }
        }
        return *this;
    }

    /** Set as null and empty.
     \return  This
    */
    ThisType& set() {
        free();
        list_ = NULL;
        return *this;
    }

    /** Set as copy of array.
     \param  data   Data to copy
     \param  size   Size as item count to copy, 0 for none
     \return        This
    */
    ThisType& set(const T* data, Size size) {
        if (size > 0) {
            const T* end = data + size;
            Node* node;
            if (list_ > EVO__PEMPTY) {
                if (list_->refs > 1) {
                    // Detach from shared
                    --list_->refs;
                } else {
                    // Overwrite existing nodes
                    Node* save = NULL;
                    node = list_->first;
                    while (data < end) {
                        if (node == NULL) {
                            // Copy remaining nodes
                            node = save;
                            if (node == NULL) {
                                // Empty, copy first node
                                node = new Node(*data);
                                list_->first = list_->last = node;
                                ++list_->size;
                                ++data;
                            }
                            while (data < end) {
                                node = new Node(node, *data);
                                ++list_->size;
                                ++data;
                            }
                            list_->last = node;
                            return *this;
                        }
                        node->value = *data;
                        save = node;
                        node = node->next;
                        ++data;
                    }

                    // Remove remaining nodes
                    if (node != NULL) {
                        assert( list_->first != NULL );
                        assert( save != NULL );
                        list_->last = save;
                        do {
                            save = node->next;
                            delete node;
                            node = save;
                        } while (node != NULL);
                        list_->last->next = NULL;
                    }
                    list_->size = size;
                    return *this;
                }
            }

            // New list
            list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
            assert( (node = new Node(*data)) );
            assert( (list_ = new List(node)) ); // TODO: node leak if this fails
            while (++data < end) {
                assert( (node = new Node(node, *data)) );
                ++list_->size;
                list_->last = node;
            }
        } else
            clear();
        return *this;
    }

    /** Set from another list.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
     \return       This
    */
    ThisType& set(const ThisType& data) {
        if (list_ != data.list_) {
            if (data.list_ > EVO__PEMPTY) {
                if (data.list_->size > 0) {
                    free();
                    list_ = data.list_;
                    ++list_->refs;
                } else if (list_ == NULL)
                    list_ = EVO__PEMPTY;
                else
                    clear();
            } else if (data.list_ == NULL) {
                free();
                list_ = NULL;
            } else {
                clear();
                list_ = data.list_;
            }
        }
        return *this;
    }

    /** Set as empty but not null.
     \return  This
    */
    ThisType& setempty() {
        if (list_ == NULL)
            list_ = EVO__PEMPTY;
        else
            clear();
        return *this;
    }

    // INFO

    /** Get whether null.
     - Always empty when null
     .
     \return  Whether null
    */
    bool null() const
        { return (list_ == NULL); }

    /** Get whether empty.
     - Empty when size() is 0
     .
     \return  Whether empty
    */
    bool empty() const
        { return (list_ <= EVO__PEMPTY || list_->size == 0); }

    /** Get size.
     \return  Size as item count
    */
    Size size() const
        { return (list_ > EVO__PEMPTY ? list_->size : 0); }

    /** Get whether shared.
     - List is shared when referencing a copy of another list (reference count > 1)
     .
     \return  Whether shared
    */
    bool shared() const
        { return (list_ > EVO__PEMPTY && list_->refs > 1); }

    /** Get pointer to first item.
     \return  First item, NULL if empty
    */
    const Item* first() const
        { return (list_ > EVO__PEMPTY && list_->first != NULL ? &list_->first->value : NULL); }

    /** Get pointer to last item.
     \return  Last item, NULL if empty
    */
    const Item* last() const
        { return (list_ > EVO__PEMPTY && list_->last != NULL ? &list_->last->value : NULL); }

    // TODO: hash()

    // COMPARE

    // FIND

    // INFO_SET

    ThisType& unshare() {
        if (list_ > EVO__PEMPTY && list_->refs > 1) {
            --list_->refs;
            Node* datanode = list_->first;
            Node* lastnode;
            list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
            assert( (lastnode = new Node(datanode->value)) );
            assert( (list_ = new List(lastnode)) );
            while ( (datanode = datanode->next) != NULL) {
                assert( (lastnode = new Node(lastnode, datanode->value)) );
                ++list_->size;
                list_->last = lastnode;
            }
        }
        return *this;
    }

    // COPY

    /** Set as full (unshared) copy of another list (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to copy
     \return       This
    */
    ThisType& copy(const ThisType& data) {
        if (&data != this) {
            if (data.list_ == NULL) {
                set();
            } else if (data.list_ == EVO__PEMPTY || data.list_->size <= 0) {
                setempty();
            } else {
                Node* lastnode;
                Node* datanode = data.list_->first;
                assert( datanode );
                if (list_ > EVO__PEMPTY) {
                    if (list_->refs > 1) {
                        // Detach from shared
                        --list_->refs;
                        list_ = EVO__PEMPTY;
                    } else {
                        // Overwrite existing nodes
                        Node* save = NULL;
                        lastnode = list_->first;
                        while (datanode != NULL) {
                            if (lastnode == NULL) {
                                // Copy remaining nodes
                                lastnode = save;
                                if (lastnode == NULL) {
                                    // Empty, copy first node
                                    lastnode = new Node(datanode->value);
                                    list_->first = list_->last = lastnode;
                                    ++list_->size;
                                    datanode = datanode->next;
                                }
                                while (datanode != NULL) {
                                    lastnode = new Node(lastnode, datanode->value);
                                    ++list_->size;
                                    datanode = datanode->next;
                                }
                                list_->last = lastnode;
                                return *this;
                            }
                            save = lastnode;
                            lastnode->value = datanode->value;
                            lastnode = lastnode->next;
                            datanode = datanode->next;
                        }

                        // Remove remaining nodes
                        if (lastnode != NULL) {
                            assert( list_->first != NULL );
                            if (save != NULL)
                                save->next = NULL;
                            list_->last = save;
                            do {
                                --list_->size;
                                save = lastnode->next;
                                delete lastnode;
                                lastnode = save;
                            } while (lastnode != NULL);
                        }
                        return *this;
                    }
                }

                // New list
                list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                assert( (lastnode = new Node(datanode->value)) );
                assert( (list_ = new List(lastnode)) );
                datanode = datanode->next;
                for (Size i=1, size=data.list_->size; i < size; ++i, ++list_->size) {
                    assert( (lastnode = new Node(lastnode, datanode->value)) );
                    datanode = datanode->next;
                }
                list_->last = lastnode;
            }
        }
        return *this;
    }

    // ADD

    // TODO: exception safe if T throws?
    // TODO: easy iterator to new items

    /** Append new items (modifier).
     - New POD items are zeroed
     - Effectively calls unshare()
     .
     \param  size  Size as item count to append
     \return       This
    */
    ThisType& addnew(Size size=1) {
        if (size > 0) {
            Size i = 0;
            Node* lastnode;
            if (list_ <= EVO__PEMPTY) {
                list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                assert( (lastnode = new Node) );
                list_ = new List(lastnode);
                ++i;
            } else if (list_->first == NULL) {
                assert( (lastnode = new Node) );
                list_->first = list_->last = lastnode;
                list_->size = 1;
                ++i;
            } else
                lastnode = list_->last;

            for (; i < size; ++i, ++list_->size)
                assert( (lastnode = new Node(lastnode)) );
            list_->last = lastnode;
        }
        return *this;
    }

    /** Append new items copied from data pointer (modifier).
     - Effectively calls unshare()
     .
     \param  data   Data to append
     \param  size   Size as item count to append
     \return        This
    */
    ThisType& add(const Item* data, Size size) {
        if (size > 0) {
            Size i = 0;
            Node* lastnode;
            if (list_ <= EVO__PEMPTY) {
                list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                assert( (lastnode = new Node(data[0])) );
                assert( (list_ = new List(lastnode)) );
                ++i;
            } else if (list_->first == NULL) {
                assert( (lastnode = new Node(data[0])) );
                list_->first = list_->last = lastnode;
                list_->size = 1;
                ++i;
            } else
                lastnode = list_->last;

            for (; i < size; ++i, ++list_->size)
                assert( (lastnode = new Node(lastnode, data[i])) );
            list_->last = lastnode;
        }
        return *this;
    }

    /** Append new items from another list (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to append
     \return       This
    */
    ThisType& add(const ThisType& data) {
        if (data.list_ != NULL && data.list_->size > 0) {
            Size i = 0;
            Node* lastnode;
            Node* datanode = data.list_->first;
            assert( datanode );
            if (list_ <= EVO__PEMPTY) {
                list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                assert( (lastnode = new Node(datanode->value)) );
                assert( (list_ = new List(lastnode)) );
                ++i;
                datanode = datanode->next;
            } else if (list_->first == NULL) {
                assert( (lastnode = new Node(datanode->value)) );
                list_->first = list_->last = lastnode;
                list_->size = 1;
                ++i;
                datanode = datanode->next;
            } else
                lastnode = list_->last;

            for (Size size=data.list_->size; i < size; ++i, ++list_->size) {
                assert( (lastnode = new Node(lastnode, datanode->value)) );
                datanode = datanode->next;
            }
            list_->last = lastnode;
        }
        return *this;
    }

    /** Append new item (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to append
     \return       This
    */
    ThisType& add(const Item& data) {
        Node* node = new Node(data);
        assert( node );
        if (list_ <= EVO__PEMPTY) {
            assert( (list_ = new List(node)) );
        } else if (list_->first == NULL) {
            list_->first = list_->last = node;
            ++list_->size;
        } else {
            assert( list_->last != NULL );
            list_->last->insertafter(node);
            list_->last = node;
            ++list_->size;
        }
        return *this;
    }

    // PREPEND

    // INSERT

    // REMOVE

    /** Remove one or more items using iterator (modifier).
     - For best performance with single linked list (LINK_SINGLE) use after=true -- otherwise a scan is done to find previous item
     \param  iter   Iterator to use, set to next item after remove (must refer to this list)  [in/out]
     \param  count  Item count to remove
     \param  after  Whether to remove items AFTER item indicated by iterator
     \return        Item count removed
    */
    Size remove(Iter& iter, Size count=1, bool after=false) {
        Size remcount = 0;
        if (count > 0 && iter && list_ > EVO__PEMPTY && list_->size > 0) {
            assert( iter.getParent() && iter.getParent()->list_ == list_ ); // make sure iterator references this list
            Node* node = iter.getKey();
            if (list_->refs > 1) {
                // Shared, new list
                --list_->refs;
                Node* datanode = list_->first;
                if (datanode == node) {
                    // Remove from first
                    if (count >= list_->size) {
                        // Removing all, set empty
                        remcount = list_->size;
                        list_ = EVO__PEMPTY;
                        iter.set(NULL, NULL);
                    } else {
                        // Skip removed items
                        do {
                            ++remcount;
                            assert( (datanode = datanode->next) != NULL );
                        } while (remcount < count);

                        // Copy remaining into new list
                        list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                        Node* lastnode;
                        assert( (lastnode = new Node(datanode->value)) );
                        assert( (list_ = new List(lastnode)) );
                        node = lastnode;
                        while ( (datanode = datanode->next) != NULL) {
                            assert( (lastnode = new Node(lastnode, datanode->value)) );
                            ++list_->size;
                            list_->last = lastnode;
                        }
                        iter.set(node, &node->value);
                    }
                } else {
                    // Remove after first
                    list_ = EVO__PEMPTY; // start as empty in case creating first node throws exception
                    Node* lastnode;
                    assert( (lastnode = new Node(datanode->value)) );
                    assert( (list_ = new List(lastnode)) );
                    while ( (datanode = datanode->next) != NULL) {
                        if (datanode == node) {
                            // Skip items to remove
                            do {
                                ++remcount;
                                if ( (datanode = datanode->next) == NULL ) {
                                    iter.set(NULL, NULL);
                                    return remcount;
                                }
                            } while (remcount < count);

                            assert( (lastnode = new Node(lastnode, datanode->value)) );
                            ++list_->size;
                            list_->last = lastnode;
                            iter.set(lastnode, &lastnode->value);
                        } else {
                            assert( (lastnode = new Node(lastnode, datanode->value)) );
                            ++list_->size;
                            list_->last = lastnode;
                        }
                    }
                }
            } else {
                // Not shared, remove items
                node = (after ? list_->removeafter(node, count) : list_->remove(node, count));
                if (node == NULL)
                    iter.set(NULL, NULL);
                else
                    iter.set(node, &node->value);
            }
        }
        return remcount;
    }

    // POP

    // ITERATORS

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { /*unshare();*/ }
    const IterItem* iterFirst(IterKey& key) const {
        if (list_ > EVO__PEMPTY) {
            key = list_->first;
            if (key != NULL)
                return &key->value;
        }
        return NULL;
    }
    const IterItem* iterNext(IterKey& key) const {
        if (key != NULL) {
            key = key->next;
            if (key != NULL)
                return &key->value;
        }
        return NULL;
    }
    const IterItem* iterNext(Size count, IterKey& key) const {
        if (count > 0) {
            while (key != NULL && count > 0) {
                key = key->next;
                --count;
            }
            if (count == 0 && key != NULL)
                return &key->value;
        }
        return NULL;
    }
    const IterItem* iterLast(IterKey& key) const {
        if (list_ > EVO__PEMPTY) {
            key = list_->last;
            if (key != NULL)
                return &key->value;
        }
        return NULL;
    }
    const IterItem* iterPrev(IterKey& key) const {
        if (key != NULL) {
            key = key->prev;
            if (key != NULL)
                return &key->value;
        }
        return NULL;
    }
    const IterItem* iterPrev(Size count, IterKey& key) const {
        if (count > 0) {
            while (key != NULL && count > 0) {
                key = key->prev;
                --count;
            }
            if (count == 0 && key != NULL)
                return &key->value;
        }
        return NULL;
    }
    Size iterCount() const
        { return (list_ > EVO__PEMPTY ? list_->size : 0); }
    /** \endcond */

private:
    struct List {
        Node* first;
        Node* last;
        Size  size;
        Size  refs;

        List() {
            first = last = NULL;
            size = 0;
            refs = 1;
        }

        List(Node* newnode) {
            first = last = newnode;
            size = 1;
            refs = 1;
        }

        ~List()
            { free(); }

        Size removeafter(Node*& node, Size count) {
            assert( node != NULL );
            assert( count > 0 );
            assert( size > 0 );

            Node* prevnode = node;
            if ( (node = node->next) == NULL)
                return 0;

            Size remcount = 0;
            {
                Node* nextnode;
                while (remcount < count && node != NULL) {
                    nextnode = node->next;
                    delete node;
                    node = nextnode;
                    ++remcount;
                }
            }
            if (node == NULL) {
                // Removed after first, through last
                last = prevnode;
                node = last->next = NULL;
            } else
                // Removed after first, before last
                prevnode->link(node);
            size -= remcount;
            return remcount;
        }

        Size remove(Node*& node, Size count) {
            assert( node != NULL );
            assert( count > 0 );
            assert( size > 0 );

            Size remcount = 0;
            Node* prevnode = node->getprev(first);
            {
                Node* nextnode;
                while (remcount < count && node != NULL) {
                    nextnode = node->next;
                    delete node;
                    node = nextnode;
                    ++remcount;
                }
            }
            if (prevnode == NULL) {
                // Removed from first
                first = node;
                if (node == NULL)
                    last = NULL;
                else
                    node->linkfirst();
            } else if (node == NULL) {
                // Removed after first, through last
                last = prevnode;
                node = last->next = NULL;
            } else
                // Removed after first, before last
                prevnode->link(node);
            size -= remcount;
            return remcount;
        }

        void free() {
            Node* node = first;
            Node* next;
            while (node != NULL) {
                next = node->next;
                delete node;
                node = next;
            }
        }
    };

    List* list_;

    void free() {
        if (list_ > EVO__PEMPTY && --list_->refs == 0)
            delete list_;
    }

    #undef EVO__PEMPTY
};

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
