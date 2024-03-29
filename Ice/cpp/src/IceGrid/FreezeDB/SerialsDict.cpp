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
// Generated from file `SerialsDict.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//


// Freeze types in this file:
// name="IceGrid::SerialsDict", key="string", value="long"

#include <Ice/BasicStream.h>
#include <IceUtil/StringUtil.h>
#include <IceGrid/FreezeDB/SerialsDict.h>

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

void
IceGrid::SerialsDictKeyCodec::write(const ::std::string& v, Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get(), encoding, true);
    stream.write(v);
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
IceGrid::SerialsDictKeyCodec::read(::std::string& v, const Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get(), encoding, &bytes[0], &bytes[0] + bytes.size());
    stream.read(v);
}

namespace
{
    const ::std::string __IceGrid__SerialsDictKeyCodec_typeId = "string";
}

const ::std::string&
IceGrid::SerialsDictKeyCodec::typeId()
{
    return __IceGrid__SerialsDictKeyCodec_typeId;
}

void
IceGrid::SerialsDictValueCodec::write(::Ice::Long v, Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get(), encoding, true);
    stream.startWriteEncaps();
    stream.write(v);
    stream.endWriteEncaps();
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
IceGrid::SerialsDictValueCodec::read(::Ice::Long& v, const Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get(), encoding, &bytes[0], &bytes[0] + bytes.size());
    stream.startReadEncaps();
    stream.read(v);
    stream.endReadEncaps();
}

namespace
{
    const ::std::string __IceGrid__SerialsDictValueCodec_typeId = "long";
}

const ::std::string&
IceGrid::SerialsDictValueCodec::typeId()
{
    return __IceGrid__SerialsDictValueCodec_typeId;
}
