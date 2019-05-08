// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file var.h Evo Var class. */
#pragma once
#ifndef INCL_evo_var_h
#define INCL_evo_var_h

#include "string.h"
#include "substring.h"
#include "list.h"
#include "maplist.h"

#if defined(EVO_CPP11)
    #include <initializer_list>
#endif

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** General purpose variable that can hold a basic value (string, number, bool, etc) or children with nested values (object, list).
 - Useful for creating a general hierarchy of values without using explicit types
 - Use `operator[]()` to set children, or assignment for basic values (number, string, bool, etc)
 - Use `child()` methods to get object field or list item, or use `get_*()` methods to get value as given type
 - This is optimized to minimize memory allocations:
   - Creating an empty string, object, or list type doesn't allocate memory
   - This uses `union` internally to hold only one of the supported types:
     - MapList is used for objects
     - List is used for lists
     - String is used for strings
     - Primitives are used for other types (`int64`, `uint64`, `double`, `bool`)
   - Object, list, and string types use \ref Sharing
 - C++11:
   - Use an initialization list to create from an array of values, which may have nested lists or objects
   - Use VarObject to create an object from an initialization list of key/value pairs -- each key/value pair is a nested list of 2 items (where the key must be a string):
     \code
     Var var = VarObject{};
     \endcode
     \code
     Var var = VarObject{
        {"key1", 123},
        {"key2", 456},
     };
     \endcode
   - When nesting initializer lists inside objects you must specify the type each time (VarObject or \link VarList\endlink), unless it's a list in a list:
     \code
     Var var = VarObject{
        {"key1", VarList{1, 2, 3, {4, 5}}},
        {"key2", 456},
     };
     \endcode
   - Lists are assumed until you use a VarObject:
     \code
     Var var3 = {1, {2, 3}, 4};
     \endcode
   - If nesting isn't done correctly you'll get compiler errors
   - Only use VarObject and \link VarList\endlink with initializer lists -- do not use as variable or parameter type, use Var instead
 .

\par Example

\code
#include <evo/var.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Var var;
    var["name"]    = "John Doe";
    var["age"]     = 21;
    var["balance"] = 99.99;
    var["active"]  = true;
    var["nothing"] = vNULL;

    var["list"][0] = 1;
    var["list"][1] = 2;
    var["list"][2] = 3;

    var["object"]["field"] = "value";

    var.dump(con().out);

    return 0;
}
\endcode

\code{.unparsed}
{
  active:true,
  age:21,
  balance:99.99,
  list:[1,2,3],
  name:'John Doe',
  nothing:null,
  object:{
    field:'value'
  }
}
\endcode

\par Example with C++11 Initialier Lists

\code
#include <evo/var.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();

    // Create and dump an object with nested lists and object
    Var var1 = VarObject{
        {"key1", VarList{1, 2, 3, {4, 5}}},
        {"key2", VarObject{
            {"nested-key1", "value1"},
            {"nested-key2", "value2"},
        }},
    };
    var1.dump(c.out);

    // Create and dump a list with various values, including a nested object
    Var var2 = {1, "two", {3.0, 4}, 5, false, vNULL, VarObject{
        {"key1", "value1"},
        {"key2", "value2"},
    }};
    var2.dump(c.out);

    // Create and dump a list with nested lists
    Var var3 = {1, {2, 3, {4}}, "five"};
    var3.dump(c.out);

    return 0;
}
\endcode

Output:
\code{.unparsed}
{
  key1:[1,2,3,[4,5]],
  key2:{
    nested-key1:'value1',
    nested-key2:'value2'
  }
}
[1,'two',[3,4],5,false,null,{
  key1:'value1',
  key2:'value2'
}]
[1,[2,3,[4]],'five']
\endcode
*/
class Var {
public:
    typedef SizeT Size;                         ///< Size type used
    typedef MapList<String,Var> ObjectType;     ///< Object map type
    typedef List<Var> ListType;                 ///< Item list type

    /** Variable type. */
    enum Type {
        tOBJECT,    ///< Object with key/value fields
        tLIST,      ///< %List of values
        tSTRING,    ///< %String value
        tFLOAT,     ///< Floating-point value
        tUNSIGNED,  ///< Integer value (unsigned)
        tINTEGER,   ///< Integer value (signed)
        tBOOL,      ///< Boolean value
        tNULL       ///< %Null value
    };

#if defined(EVO_CPP11)
    /** Sequence constructor (C++11).
     \param  init  Initializer list, passed as comma-separated values in braces `{ }`
    */
    Var(std::initializer_list<Var> init) : type_(tNULL) {
        assert( init.size() < IntegerT<ListType::Size>::MAX );
        ListType& items = list();
        items.reserve((typename ListType::Size)init.size());
        for (const auto& val : init)
            items.add(val);
    }

    /** Move constructor (C++11).
     \param  src  Source to move
    */
    Var(Var&& src) {
        type_ = src.type_;
        switch (type_) {
            case tOBJECT:
                memcpy(buf_obj_, src.buf_obj_, sizeof(ObjectType));
                break;
            case tLIST:
                memcpy(buf_list_, src.buf_list_, sizeof(ListType));
                break;
            case tSTRING:
                memcpy(buf_str_, src.buf_str_, sizeof(String));
                break;
            case tFLOAT:
                buf_float_ = src.buf_float_;
                break;
            case tUNSIGNED:
                buf_uint_ = src.buf_uint_;
                break;
            case tINTEGER:
                buf_int_ = src.buf_int_;
                break;
            case tBOOL:
                buf_bool_ = src.buf_bool_;
                break;
            case tNULL:
                break;
        }
        src.type_ = tNULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    Var& operator=(Var&& src) {
        free();
        type_ = src.type_;
        switch (type_) {
            case tOBJECT:
                memcpy(buf_obj_, src.buf_obj_, sizeof(ObjectType));
                break;
            case tLIST:
                memcpy(buf_list_, src.buf_list_, sizeof(ListType));
                break;
            case tSTRING:
                memcpy(buf_str_, src.buf_str_, sizeof(String));
                break;
            case tFLOAT:
                buf_float_ = src.buf_float_;
                break;
            case tUNSIGNED:
                buf_uint_ = src.buf_uint_;
                break;
            case tINTEGER:
                buf_int_ = src.buf_int_;
                break;
            case tBOOL:
                buf_bool_ = src.buf_bool_;
                break;
            case tNULL:
                break;
        }
        src.type_ = tNULL;
        return *this;
    }
#endif

    /** Default constructor sets as null. */
    Var() : type_(tNULL) {
    }

    /** Copy constructor.
     - This uses \ref Sharing
    */
    Var(const Var& src) {
        type_ = src.type_;
        switch (type_) {
            case tOBJECT: {
                const ObjectType* src_ptr = (const ObjectType*)src.buf_obj_;
                new((ObjectType*)buf_obj_) ObjectType(*src_ptr);
                break;
            }
            case tLIST: {
                const ListType* src_ptr = (const ListType*)src.buf_list_;
                new((ListType*)buf_list_) ListType(*src_ptr);
                break;
            }
            case tSTRING: {
                const String* src_ptr = (const String*)src.buf_str_;
                new((String*)buf_str_) String(*src_ptr);
                break;
            }
            case tFLOAT:
                buf_float_ = src.buf_float_;
                break;
            case tUNSIGNED:
                buf_uint_ = src.buf_uint_;
                break;
            case tINTEGER:
                buf_int_ = src.buf_int_;
                break;
            case tBOOL:
                buf_bool_ = src.buf_bool_;
                break;
            case tNULL:
                break;
        }
    }

    /** Constructor to copy from an object map type.
     - This uses \ref Sharing
     .
     \param  val  Object map value to copy
    */
    Var(const ObjectType& val) : type_(tNULL) {
        object() = val;
    }

    /** Constructor to copy from an object list type.
     - This uses \ref Sharing
     .
     \param  val  %List value to copy
    */
    Var(const ListType& val) : type_(tNULL) {
        list() = val;
    }

    /** Constructor to copy from a string value.
     - This uses \ref Sharing
     .
     \param  val  %String to copy
    */
    Var(const String& val) : type_(tNULL) {
        string() = val;
    }

    /** Constructor to copy from a string value.
     - This makes a unique copy -- doesn't use \ref Sharing
     .
     \param  val  %String to copy
    */
    Var(const StringBase& val) : type_(tNULL) {
        string() = val;
    }

    /** Constructor to copy from an string value.
     - This uses \ref Sharing
     .
     \param  val  %String to copy, must be terminated, NULL for construct an empty string
    */
    Var(const char* val) : type_(tNULL) {
        String& str = string();
        if (val != NULL)
            str = val;
    }

    /** Constructor to initialize as floating-point value.
     \param  val  Value to copy
    */
    Var(double val) : type_(tNULL) {
        numf() = val;
    }

    /** Constructor to initialize as unsigned integer value (ushort).
     \param  val  Value to copy
    */
    Var(ushort val) : type_(tNULL) {
        numu() = val;
    }

    /** Constructor to initialize as unsigned integer value (uint).
     \param  val  Value to copy
    */
    Var(uint val) : type_(tNULL) {
        numu() = val;
    }

    /** Constructor to initialize as unsigned integer value (ulong).
     \param  val  Value to copy
    */
    Var(ulong val) : type_(tNULL) {
        numu() = val;
    }

    /** Constructor to initialize as unsigned integer value (ulongl).
     \param  val  Value to copy
    */
    Var(ulongl val) : type_(tNULL) {
        numu() = val;
    }

    /** Constructor to initialize as integer value (short).
     \param  val  Value to copy
    */
    Var(short val) : type_(tNULL) {
        numi() = val;
    }

    /** Constructor to initialize as integer value (int).
     \param  val  Value to copy
    */
    Var(int val) : type_(tNULL) {
        numi() = val;
    }

    /** Constructor to initialize as integer value (long).
     \param  val  Value to copy
    */
    Var(long val) : type_(tNULL) {
        numi() = val;
    }

    /** Constructor to initialize as integer value (longl).
     \param  val  Value to copy
    */
    Var(longl val) : type_(tNULL) {
        numi() = val;
    }

    /** Constructor to initialize as boolean value (bool).
     \param  val  Value to copy
    */
    Var(bool val) : type_(tNULL) {
        boolref() = val;
    }

    /** Constructor to initialize as as null with \link vNULL\endlink as argument. */
    Var(ValNull) : type_(tNULL) {
    }

    /** Destructor. */
    ~Var() {
        free();
    }

    /** Get current type.
     \return  Current type
    */
    Type type() const {
        return type_;
    }

    /** Get whether a container type (object or list).
     - %String is not considered a container here
     .
     \return  Whether a container type
    */
    bool is_container() const {
        switch (type_) {
            case tOBJECT:
            case tLIST:
                return true;
            default:
                break;
        }
        return false;
    }

    /** Get whether an object type (tOBJECT).
     - Note that the object (MapList) type can be explicitly set to null
     .
     \return  Whether an object type
    */
    bool is_object() const {
        return (type_ == tOBJECT);
    }

    /** Get whether a list type (tLIST).
     - Note that the List type can be explicitly set to null
     .
     \return  Whether a list type
    */
    bool is_list() const {
        return (type_ == tLIST);
    }

    /** Get whether a string type (tSTRING).
     - Note that the String type can be explicitly set to null
     .
     \return  Whether a string type
    */
    bool is_string() const {
        return (type_ == tSTRING);
    }

    /** Get whether a number type (tFLOAT, tUNSIGNED, tINTEGER).
     \return  Whether a number type
    */
    bool is_number() const {
        switch (type_) {
            case tFLOAT:
            case tUNSIGNED:
            case tINTEGER:
                return true;
            default:
                break;
        }
        return false;
    }

    /** Get whether a floating-point number type (tFLOAT).
     \return  Whether a floating-point number type
    */
    bool is_float() const {
        return (type_ == tFLOAT);
    }

    /** Get whether an integer number type (tUNSIGNED or tINTEGER).
     \return  Whether an integer number type
    */
    bool is_integer() const {
        switch (type_) {
            case tUNSIGNED:
            case tINTEGER:
                return true;
            default:
                break;
        }
        return false;
    }

    /** Get whether an unsigned integer number type (tUNSIGNED).
     \return  Whether an unsigned integer number type
    */
    bool is_unsigned() const {
        return (type_ == tUNSIGNED);
    }

    /** Get whether a signed integer number type (tINTEGER).
     \return  Whether a signed integer number type
    */
    bool is_signed() const {
        return (type_ == tINTEGER);
    }

    /** Get whether a boolean type (tBOOL).
     \return  Whether a boolean type
    */
    bool is_bool() const {
        return (type_ == tBOOL);
    }

    /** Get whether this has shared data.
     - Note that an unshared object or list may still have nested children that themselves are shared
     - See \ref Sharing
     .
     \return  Whether shared
    */
    bool shared() const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                return p->shared();
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                return p->shared();
            }
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->shared();
            }
            default:
                break;
        }
        return false;
    }

    /** Get whether this or any nested key or value has any shared data.
     - This recursively scans all children and returns true if any shared key or value is found
     - Use unshare_all() to make this value and all children unique
     - See \ref Sharing
     .
     \return  Whether this or any nested part is shared
    */
    bool shared_scan() const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                const ObjectType& obj = *p;
                if (obj.shared())
                    return true;
                for (ObjectType::Iter iter(obj); iter; ++iter)
                    if (iter->key().shared() || iter->value().shared_scan())
                        return true;
                break;
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                const ListType& list = *p;
                if (list.shared())
                    return true;
                for (ListType::Iter iter(list); iter; ++iter)
                    if (iter->shared_scan())
                        return true;
                break;
            }
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->shared();
            }
            default:
                break;
        }
        return false;
    }

    /** Get whether null.
     \return  Whether null, true if null type or if object, list, or string type was explicitly set to null
    */
    bool null() const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                return p->null();
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                return p->null();
            }
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->null();
            }
            case tNULL:
                return true;
            default:
                break;
        }
        return false;
    }

    /** Get whether empty.
     - This is considered empty if there aren't any children, i.e. size() is 0
     .
     \return  Whether empty
    */
    bool empty() const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                return p->empty();
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                return p->empty();
            }
            default:
                break;
        }
        return true;
    }

    /** Get size as number of children.
     \return  Number of children, 0 if not an object or list
    */
    Size size() const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                return p->size();
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                return p->size();
            }
            default:
                break;
        }
        return 0;
    }

    /** %Set as null type/value.
     \return  This
    */
    Var& set() {
        free();
        type_ = tNULL;
        return *this;
    }

    /** Clear current value.
     - This removes all children (if an object or list), or removes all characters (if string), or sets to 0 if a number, or sets to fale if bool
     .
     \return  This
    */
    Var& clear() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->clear();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->clear();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->clear();
                break;
            }
            case tFLOAT:
                buf_float_ = 0.0;
                break;
            case tUNSIGNED:
                buf_uint_ = 0;
                break;
            case tINTEGER:
                buf_int_ = 0;
                break;
            case tBOOL:
                buf_bool_ = false;
                break;
            default:
                break;
        }
        return *this;
    }

    /** Unshare current value.
     - Nested values may be still be shared, use unshare_all() to unshare all nested values too
     - See \ref Sharing
     .
     \return  This
    */
    Var& unshare() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->unshare();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->unshare();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->unshare();
                break;
            }
            default:
                break;
        }
        return *this;
    }

    /** Unshare current value and all nested objects, lists, and strings.
     - This recursively unshares all nested object keys and values
     - Use this before passing an instance to another thread
     - See \ref Sharing
     .
     \return  This
    */
    Var& unshare_all() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                ObjectType& obj = *p;
                obj.unshare();
                for (ObjectType::IterM iter(obj); iter; ++iter) {
                    ((String&)iter->key()).unshare(); // key is const to prevent reordering, need to override const to unshare()
                    iter->value().unshare_all();
                }
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                ListType& list = *p;
                list.unshare();
                for (ListType::IterM iter(list); iter; ++iter)
                    iter->unshare_all();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->unshare();
                break;
            }
            default:
                break;
        }
        return *this;
    }

    /** Get child in object using key, default to null instance (const).
     - This can be chained and let's you pretend there's always a child -- example:
       \code
       Var var; // no children
       int64 num = var.childref("top").childref("num").get_int();
       \endcode
     - To create children if needed, use operator[]()
     .
     \return  Reference to found child, or null var if not found
    */
    const Var& childref(const StringBase& key) const {
        if (type_ == tOBJECT) {
            const ObjectType* p = (ObjectType*)buf_obj_;
            const Var* v = p->find(key);
            if (v != NULL)
                return *v;
        }
        return null_var();
    }

    /** Get child in list using index, default to null instance (const).
     - This can be chained and let's you pretend there's always a child -- example:
       \code
       Var var; // no children
       int64 num = var.childref(0).childref(1).get_int();
       \endcode
     - To create children if needed, use operator[](Size)
     .
     \return  Reference to found child, or null var if not found
    */
    const Var& childref(Size index) const {
        if (type_ == tLIST) {
            const ListType* l = (ListType*)buf_list_;
            if (l != NULL && index < l->size())
                return l->item(index);
        }
        return null_var();
    }

    /** Find child in object using key (const).
     \return  Pointer to found child, NULL if not found or not an object
    */
    const Var* child(const StringBase& key) const {
        if (type_ == tOBJECT) {
            const ObjectType* p = (const ObjectType*)buf_obj_;
            return p->find(key);
        }
        return NULL;
    }

    /** Find child in list using index (const).
     \return  Pointer to found child, NULL if not found or not a list
    */
    const Var* child(Size index) const {
        if (type_ == tLIST) {
            const ListType* l = (const ListType*)buf_list_;
            if (l != NULL && index < l->size())
                return &l->item(index);
        }
        return NULL;
    }

    /** Find child in object using key (mutable).
     \return  Pointer to found child, NULL if not found or not an object
    */
    Var* childM(const StringBase& key) {
        if (type_ == tOBJECT) {
            ObjectType* p = (ObjectType*)buf_obj_;
            return p->findM(key);
        }
        return NULL;
    }

    /** Find child in list using index (mutable).
     \return  Pointer to found child, NULL if not found or not a list
    */
    Var* childM(Size index) {
        if (type_ == tLIST) {
            ListType* l = (ListType*)buf_list_;
            return &l->itemM(index);
        }
        return NULL;
    }

    /** Get object reference, recreate as object if needed.
     - If this isn't an object, this will recreate this as an empty object (previous value is lost)
     .
     \return  Object map reference for this, with field name for keys
    */
    ObjectType& object() {
        ObjectType* ptr = (ObjectType*)buf_obj_;
        switch (type_) {
            case tOBJECT:
                return *ptr;
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            default:
                break;
        }
        type_ = tOBJECT;
        new(ptr) ObjectType();
        ptr->setempty();
        return *ptr;
    }

    /** Get list reference, recreate as list if needed.
     - If this isn't a list, this will recreate this as an empty list (previous value is lost)
     .
     \return  %List reference for this
    */
    ListType& list() {
        ListType* ptr = (ListType*)buf_obj_;
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST:
                return *ptr;
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            default:
                break;
        }
        type_ = tLIST;
        new(ptr) ListType();
        ptr->setempty();
        return *ptr;
    }

    /** Get string reference, recreate as string if needed.
     - If this isn't a string, this will recreate this as an empty string (previous value is lost)
     .
     \return  Value string reference for this
    */
    String& string() {
        String* ptr = (String*)buf_str_;
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING:
                return *ptr;
            default:
                break;
        }
        type_ = tSTRING;
        new(ptr) String();
        ptr->setempty();
        return *ptr;
    }

    /** Get unsigned integer number reference, recreate as unsigned integer if not an integer.
     - If this is a signed integer, the type is set to unsigned and the reference is casted to unsigned so the value is accessed as-is
     - If this isn't any integer type, this will recreate this as an unsigned integer
     - When recreated, the previous value is converted if another number type (tFLOAT), otherwise previous value is lost and the new value is 0
     .
     \return  Value number reference for this
    */
    uint64& numu() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            case tFLOAT: {
                const uint64 num = (uint64)buf_float_;
                type_ = tUNSIGNED;
                buf_uint_ = num;
                return buf_uint_;
            }
            case tINTEGER:
                type_ = tUNSIGNED;
            case tUNSIGNED:
                return buf_uint_;
            case tBOOL:
            case tNULL:
                break;
        }
        type_ = tUNSIGNED;
        buf_uint_ = 0;
        return buf_uint_;
    }

    /** Get signed integer number reference, recreate as signed integer if not an integer.
     - If this is an unsigned integer, the type is set to signed and the reference is casted to signed so the value is accessed as-is
     - If this isn't any integer type, this will recreate this as a signed integer
     - When recreated, the previous value is converted if another number type (tFLOAT), otherwise previous value is lost and the new value is 0
     .
     \return  Value number reference for this
    */
    int64& numi() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            case tFLOAT: {
                const int64 num = (int64)buf_float_;
                type_ = tINTEGER;
                buf_int_ = num;
                return buf_int_;
            }
            case tUNSIGNED:
                type_ = tINTEGER;
            case tINTEGER:
                return buf_int_;
            case tBOOL:
            case tNULL:
                break;
        }
        type_ = tINTEGER;
        buf_int_ = 0;
        return buf_int_;
    }

    /** Get floating-point number reference, recreate as floating-point if needed.
     - If this isn't a floating-point number, this will recreate this as a floating-point number
     - When recreated, the previous value is converted if another number type (tINTEGER), otherwise previous value is lost and the new value is 0.0
     .
     \return  Value floating-point number reference for this
    */
    double& numf() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            case tFLOAT:
                return buf_float_;
            case tUNSIGNED: {
                const double num = (double)buf_uint_;
                type_ = tFLOAT;
                buf_float_ = num;
                return buf_float_;
            }
            case tINTEGER: {
                const double num = (double)buf_int_;
                type_ = tFLOAT;
                buf_float_ = num;
                return buf_float_;
            }
            case tBOOL:
            case tNULL:
                break;
        }
        type_ = tFLOAT;
        buf_float_ = 0.0;
        return buf_float_;
    }

    /** Get bool reference, recreate as bool if needed.
     - If this isn't a bool, this will recreate this as a bool (previous value is lost, new value is false)
     .
     \return  Value bool reference for this
    */
    bool& boolref() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            case tUNSIGNED:
            case tINTEGER:
            case tFLOAT:
            case tNULL:
                break;
            case tBOOL:
                return buf_bool_;
        }
        type_ = tBOOL;
        buf_bool_ = false;
        return buf_bool_;
    }

    /** Get value as a string (const).
     - This will format current value as a string if not a string and not an object or list
     - If current value is a string then it's copied with `operator=()` -- see \ref Sharing
     .
     \param  val  Stores result string value on success, set to null if current value is null (including null object/list)  [out]
     \return      Whether successful, false if an object or list or null
    */
    bool get_val(String& val) const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                if (p->null()) {
                    val.set();
                    return true;
                }
                break;
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                if (p->null()) {
                    val.set();
                    return true;
                }
                break;
            }
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                val = *p;
                return true;
            }
            case tFLOAT:
                val.setn(buf_float_);
                return true;
            case tUNSIGNED:
                val.setn(buf_uint_);
                return true;
            case tINTEGER:
                val.setn(buf_int_);
                return true;
            case tBOOL:
                if (buf_bool_)
                    val.set("true", 4);
                else
                    val.set("false", 5);
                return true;
            case tNULL:
                break;
        }
        return false;
    }

    /** Get read-only object reference (const).
     - If not an object, this returns a reference to static null object
     .
     \return  Object reference
    */
    const ObjectType& get_object() const {
        if (type_ != tOBJECT) {
            static const ObjectType DEF;
            return DEF;
        }
        const ObjectType* p = (const ObjectType*)buf_obj_;
        return *p;
    }

    /** Get read-only list reference (const).
     - If not a list, this returns a reference to static null list
     .
     \return  %List reference
    */
    const ListType& get_list() const {
        if (type_ != tLIST) {
            static const ListType DEF;
            return DEF;
        }
        const ListType* p = (const ListType*)buf_list_;
        return *p;
    }

    /** Get read-only string value reference (const).
     - If not a string, this returns a reference to static null string
     .
     \return  %String reference
    */
    const String& get_str() const {
        if (type_ != tSTRING) {
            static const String DEF;
            return DEF;
        }
        const String* p = (const String*)buf_str_;
        return *p;
    }

    /** Get value as a floating-point number (const).
     - For string, integer, or bool type this will return that value converted to floating-point
     - Strings are converted using String::getnumf<double>()
     .
     \return  Value as floating-point number, 0.0 if null or an object or list
    */
    double get_float() const {
        switch (type_) {
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->getnumf<double>();
            }
            case tFLOAT:
                return buf_float_;
            case tUNSIGNED:
                return (double)buf_uint_;
            case tINTEGER:
                return (double)buf_int_;
            case tBOOL:
                return (double)buf_bool_;
            default:
                break;
        }
        return 0.0;
    }

    /** Get unsigned integer number value (const).
     - For signed integer type, this will cast to an unsigned int
     - For string, floating-point, or bool type this will return that value converted to an integer
     - Strings are converted using String::getnum<int64>()
     .
     \return  Value as unsigned integer, 0 if null or an object or list
    */
    uint64 get_uint() const {
        switch (type_) {
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->getnum<uint64>();
            }
            case tFLOAT:
                return (uint64)buf_float_;
            case tUNSIGNED:
            case tINTEGER:
                return buf_uint_;
            case tBOOL:
                return (uint64)buf_bool_;
            default:
                break;
        }
        return 0;
    }

    /** Get signed integer number value (const).
     - For unsigned integer type, this will cast to a signed int
     - For string, floating-point, or bool type this will return that value converted to an integer
     - Strings are converted using String::getnum<int64>()
     .
     \return  Value as signed integer, 0 if null or an object or list
    */
    int64 get_int() const {
        switch (type_) {
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->getnum<int64>();
            }
            case tFLOAT:
                return (int64)buf_float_;
            case tUNSIGNED:
            case tINTEGER:
                return buf_int_;
            case tBOOL:
                return (int64)buf_bool_;
            default:
                break;
        }
        return 0;
    }

    /** Get boolean value (const).
     - For string, number, or bool type this will return that value converted to a bool -- 0 is false and non-zero is true
     - Strings are converted using String::getbool<bool>()
     .
     \return  Value as bool, false if null or an object or list
    */
    bool get_bool() const {
        switch (type_) {
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                return p->getbool<bool>();
            }
            case tFLOAT:
                return (buf_float_ != 0.0);
            case tUNSIGNED:
            case tINTEGER:
                return (buf_int_ != 0);
            case tBOOL:
                return buf_bool_;
            default:
                break;
        }
        return false;
    }

    /** Assignment copy operator.
     - This uses \ref Sharing
     .
     \param  src  Source to copy from
     \return      This
    */
    Var& operator=(const Var& src) {
        free();
        type_ = src.type_;
        switch (type_) {
            case tOBJECT: {
                const ObjectType* src_p = (const ObjectType*)src.buf_obj_;
                new((ObjectType*)buf_obj_) ObjectType(*src_p);
                break;
            }
            case tLIST: {
                const ListType* src_p = (const ListType*)src.buf_list_;
                new((ListType*)buf_list_) ListType(*src_p);
                break;
            }
            case tSTRING: {
                const String* src_p = (const String*)src.buf_str_;
                new((String*)buf_str_) String(*src_p);
                break;
            }
            case tFLOAT:
                buf_float_ = src.buf_float_;
                break;
            case tUNSIGNED:
                buf_uint_ = src.buf_uint_;
                break;
            case tINTEGER:
                buf_int_ = src.buf_int_;
                break;
            case tBOOL:
                buf_bool_ = src.buf_bool_;
                break;
            case tNULL:
                break;
        }
        return *this;
    }

    /** Assignment operator to copy from object map type.
     - This uses \ref Sharing
     .
     \param  val  Value to copy from
     \return      This
    */
    Var& operator=(const ObjectType& val) {
        object() = val;
        return *this;
    }

    /** Assignment operator to copy from list type.
     - This uses \ref Sharing
     .
     \param  val  Value to copy from
     \return      This
    */
    Var& operator=(const ListType& val) {
        list() = val;
        return *this;
    }

    /** Assignment operator to copy from string value.
     - This uses \ref Sharing
     .
     \param  val  Value to copy from
     \return      This
    */
    Var& operator=(const String& val) {
        string() = val;
        return *this;
    }

    /** Assignment operator to copy from string value.
     - This makes a unique copy -- doesn't use \ref Sharing
     .
     \param  val  Value to copy from
     \return      This
    */
    Var& operator=(const StringBase& val) {
        string() = val;
        return *this;
    }

    /** Assignment operator to copy from terminated string value.
     - This uses \ref Sharing
     .
     \param  val  Value to copy from, must be terminated string, NULL for empty string
     \return      This
    */
    Var& operator=(const char* val) {
        String& str = string();
        if (val == NULL)
            str.clear();
        else
            str = val;
        return *this;
    }

    /** Assignment operator to set as floating-point value.
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(double val) {
        numf() = val;
        return *this;
    }

    /** Assignment operator to set as unsigned integer value (ushort).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(ushort val) {
        numu() = val;
        return *this;
    }

    /** Assignment operator to set as unsigned integer value (uint).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(uint val) {
        numu() = val;
        return *this;
    }

    /** Assignment operator to set as unsigned integer value (ulong).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(ulong val) {
        numu() = val;
        return *this;
    }

    /** Assignment operator to set as unsigned integer value (ulongl).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(ulongl val) {
        numu() = val;
        return *this;
    }

    /** Assignment operator to set as signed integer value (short).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(short val) {
        numi() = val;
        return *this;
    }

    /** Assignment operator to set as signed integer value (int).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(int val) {
        numi() = val;
        return *this;
    }

    /** Assignment operator to set as signed integer value (long).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(long val) {
        numi() = val;
        return *this;
    }

    /** Assignment operator to set as signed integer value (longl).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(longl val) {
        numi() = val;
        return *this;
    }

    /** Assignment operator to set as boolean value (bool).
     \param  val  Value to copy
     \return      This
    */
    Var& operator=(bool val) {
        boolref() = val;
        return *this;
    }

    /** Assignment operator to set as null using \link vNULL\endlink.
     \return  This
    */
    Var& operator=(ValNull) {
        set();
        return *this;
    }

    /** Object field access operator.
     - This gets the value under `key`, creating a new value if it doesn't exist
     - If not an object, this recreates as an object first and previous value is lost
     .
     \param  key  Field key to use
     \return      Reference to field value
    */
    Var& operator[](const StringBase& key) {
        return object()[key];
    }

    /** %List item access operator.
     - This gets the value under `index`, creating a new value if it doesn't exist
     - If not a list, this recreates as a list first and previous value is lost
     .
     \param  index  Item index to use
     \return        Reference to list item value
    */
    Var& operator[](Size index) {
        return list().addmin(index + 1).itemM(index);
    }

    /** Dump all children and values to stream or string.
     - This dumps with a syntax similar to JSON but with \ref SmartQuoting on string keys and values
       - Key strings are unquoted if possible, string values are always quoted
     .
     \tparam  T  Output string or stream type -- inferred from `out` argument, usually String, Stream, or StreamOut
     \param  out      Stream or string to dump (write) to
     \param  newline  Newline type to use, \link NL\endlink for default
     \param  indent   Starting space indent count, usually 0
     \return          Reference to `out` param
    */
    template<class T>
    T& dump(T& out, const NewlineValue& newline=NL, uint indent=0) const {
        dump_impl(out, newline, indent);
        return out;
    }

private:
    Type type_;

    union {
        // Map/List size not affected by item/value size, sizeof(Var) not yet known
        char buf_obj_[sizeof(MapList<String,char>)];
        char buf_list_[sizeof(List<char>)];
        char buf_str_[sizeof(String)];
        double buf_float_;
        uint64 buf_uint_;
        int64  buf_int_;
        bool   buf_bool_;
    };

    void free() {
        switch (type_) {
            case tOBJECT: {
                ObjectType* p = (ObjectType*)buf_obj_;
                p->~ObjectType();
                break;
            }
            case tLIST: {
                ListType* p = (ListType*)buf_list_;
                p->~ListType();
                break;
            }
            case tSTRING: {
                String* p = (String*)buf_str_;
                p->~String();
                break;
            }
            default:
                break;
        }
    }

    const Var& null_var() const {
        static const Var NULL_VAR;
        return NULL_VAR;
    }

    template<class T>
    void dump_val(T& out, char end_delim) const {
        switch (type_) {
            case tSTRING: {
                const String* p = (const String*)buf_str_;
                const String& str = *p;
                out.writequoted(str.data(), str.size(), end_delim);
                break;
            }
            case tFLOAT:
                out << buf_float_;
                break;
            case tUNSIGNED:
                out << buf_uint_;
                break;
            case tINTEGER:
                out << buf_int_;
                break;
            case tBOOL:
                if (buf_bool_)
                    out.writetext("true", 4);
                else
                    out.writetext("false", 5);
                break;
            case tNULL:
                out.writetext("null", 4);
                break;
            default:
                break; // COV: never happens
        }
    }

    template<class T, class TNL>
    void dump_impl(T& out, const TNL& newline, uint indent, bool first=true) const {
        switch (type_) {
            case tOBJECT: {
                const ObjectType* p = (const ObjectType*)buf_obj_;
                const ObjectType& obj = *p;
                if (obj.empty()) {
                    out << "{}";
                } else {
                    indent += 2;
                    out << '{' << newline;
                    char delim = ',';
                    typename ObjectType::Iter iter(obj);
                    if (iter) {
                        for (;;) {
                            const typename ObjectType::Iter::Item& item = *iter;
                            if (!++iter)
                                delim = '}';

                            out.writechar(' ', indent);
                            const String& key = item.key();
                            out.writequoted(key.data(), key.size(), ':', true);
                            out << ':';

                            const Var& val = item.value();
                            if (val.is_container())
                                val.dump_impl(out, newline, indent, false);
                            else
                                val.dump_val(out, delim);

                            if (delim != ',')
                                break;
                            out << ',' << newline;
                        }
                    }
                    out << newline;
                    indent -= 2;
                    out.writechar(' ', indent);
                    out << '}';
                }
                break;
            }
            case tLIST: {
                const ListType* p = (const ListType*)buf_list_;
                const ListType& list = *p;
                if (list.empty()) {
                    out << "[]";
                } else {
                    out << '[';
                    char delim = ',';
                    typename ListType::Iter iter(list);
                    if (iter) {
                        for (;;) {
                            const Var& val = *iter;
                            if (!++iter)
                                delim = ']';
                            if (val.is_container())
                                val.dump_impl(out, newline, indent, false);
                            else
                                val.dump_val(out, delim);

                            if (delim != ',')
                                break;
                            out << ',';
                        }
                    }
                    out << ']';
                }
                break;
            }
            default:
                dump_val(out, ',');
                break;
        }
        if (first)
            out << newline;
    }
};

///////////////////////////////////////////////////////////////////////////////
#if defined(EVO_CPP11)

/** Explicit variant of Var used for initializing as an object with an initializer list of key/value pairs (C++11).
 - Only use this with initializer lists -- avoid defining a variable or parameter with this type
 - See Var examples
*/
class VarObject : public Var {
public:
    /** Initializer key/value pair. */
    struct InitPair {
        String key;     ///< Field key string
        Var value;      ///< Field value

        template<class T>
        InitPair(const String& key, const T& value) : key(key), value(value) {
        }

        template<class T>
        InitPair(const StringBase& key, const T& value) : key(key), value(value) {
        }

        template<class T>
        InitPair(const char* key, const T& value) : key(key), value(value) {
        }
    };

    /** Default constructor creates an empty object. */
    VarObject() {
        object();
    }

    /** Sequence constructor for object initializer.
     \param  init  Initializer list of key/value pairs, each key/value pair is a nested intializer list with 2 items (key and value, where key is a string)
    */
    VarObject(std::initializer_list<InitPair> init) {
        assert( init.size() < IntegerT<ObjectType::Size>::MAX );
        ObjectType& map = object();
        map.reserve((typename ObjectType::Size)init.size());
        for (const auto& val : init)
            map[val.key] = val.value;
    }

    /** Move constructor.
     \param src  Source to move from
    */
    VarObject(VarObject&& src) : Var(std::move(src)) {
    }

    /** Move assignment operator.
     \param src  Source to move from
     \return     This
    */
    Var& operator=(VarObject&& src) {
        return Var::operator=(std::move(src));
    }

    // Disable copying
    VarObject(const VarObject&) = delete;
    VarObject& operator=(const VarObject&) = delete;
};

/** Alias for Var, useful for explicitly passing a list to a VarObject field value (C++11).
 - Only use this with initializer lists -- avoid defining a variable or parameter with this type
 - See Var examples
*/
typedef Var VarList;
#endif
///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
