// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
//
// Ice version 3.5.1
//
// <auto-generated>
//
// Generated from file `IdentityObjectInfoDict.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//


// Freeze types in this file:
// name="IceGrid::IdentityObjectInfoDict", key="Ice::Identity", value="IceGrid::ObjectInfo"

#ifndef __IdentityObjectInfoDict_h__
#define __IdentityObjectInfoDict_h__

#include <Freeze/Map.h>
#include <Ice/Identity.h>
#include <IceGrid/Admin.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 305
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 > 50
#       error Beta header file detected
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace IceGrid
{

class IdentityObjectInfoDictKeyCodec
{
public:

    static void write(const ::Ice::Identity&, Freeze::Key&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
    static void read(::Ice::Identity&, const Freeze::Key&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
    static const std::string& typeId();
};

class IdentityObjectInfoDictValueCodec
{
public:

    static void write(const ::IceGrid::ObjectInfo&, Freeze::Value&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
    static void read(::IceGrid::ObjectInfo&, const Freeze::Value&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
    static const std::string& typeId();
};

class IdentityObjectInfoDict : public Freeze::Map< ::Ice::Identity, ::IceGrid::ObjectInfo, IdentityObjectInfoDictKeyCodec, IdentityObjectInfoDictValueCodec, Freeze::IceEncodingCompare >
{
public:


    class TypeIndex : public Freeze::MapIndex< ::std::string, TypeIndex, Freeze::IceEncodingCompare >
    {
    public:

        TypeIndex(const std::string&, const Freeze::IceEncodingCompare& = Freeze::IceEncodingCompare());

        static void write(const ::std::string&, Freeze::Key&, const Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
        static void read(::std::string&, const Freeze::Key&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);

    protected:

        virtual void marshalKey(const Freeze::Value&, Freeze::Key&) const;
    };

    IdentityObjectInfoDict(const Freeze::ConnectionPtr&, const std::string&, bool = true, const Freeze::IceEncodingCompare& = Freeze::IceEncodingCompare());

    template <class _InputIterator>
    IdentityObjectInfoDict(const Freeze::ConnectionPtr& __connection, const std::string& __dbName, bool __createDb, _InputIterator __first, _InputIterator __last, const Freeze::IceEncodingCompare& __compare = Freeze::IceEncodingCompare())
        : Freeze::Map< ::Ice::Identity, ::IceGrid::ObjectInfo, IdentityObjectInfoDictKeyCodec, IdentityObjectInfoDictValueCodec, Freeze::IceEncodingCompare >(__connection->getCommunicator(), __connection->getEncoding())
    {
        Freeze::KeyCompareBasePtr __keyCompare = new Freeze::KeyCompare< ::Ice::Identity, IdentityObjectInfoDictKeyCodec, Freeze::IceEncodingCompare >(__compare, this->_communicator, this->_encoding);
        std::vector<Freeze::MapIndexBasePtr> __indices;
        __indices.push_back(new TypeIndex("type"));
        this->_helper.reset(Freeze::MapHelper::create(__connection, __dbName, IdentityObjectInfoDictKeyCodec::typeId(), IdentityObjectInfoDictValueCodec::typeId(), __keyCompare, __indices, __createDb));
        while(__first != __last)
        {
            put(*__first);
            ++__first;
        }
    }
    static void recreate(const Freeze::ConnectionPtr&, const std::string&, const Freeze::IceEncodingCompare& = Freeze::IceEncodingCompare());


    iterator findByType(const ::std::string&, bool = true);
    const_iterator findByType(const ::std::string&, bool = true) const;
    iterator beginForType();
    const_iterator beginForType() const;
    iterator endForType();
    const_iterator endForType() const;
    iterator lowerBoundForType(const ::std::string&);
    const_iterator lowerBoundForType(const ::std::string&) const;
    iterator upperBoundForType(const ::std::string&);
    const_iterator upperBoundForType(const ::std::string&) const;
    std::pair<iterator, iterator> equalRangeForType(const ::std::string&);
    std::pair<const_iterator, const_iterator> equalRangeForType(const ::std::string&) const;
    int typeCount(const ::std::string&) const;
};

}

#endif
