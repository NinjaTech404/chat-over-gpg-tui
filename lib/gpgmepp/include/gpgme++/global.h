/*
  global.h - global gpgme functions and enums
  Copyright (C) 2003, 2007 Klarälvdalens Datakonsult AB
  2016 Bundesamt für Sicherheit in der Informationstechnik
  Software engineering by Intevation GmbH

  This file is part of GPGME++.

  GPGME++ is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  GPGME++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with GPGME++; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

// -*- c++ -*-
#ifndef __GPGMEPP_GLOBAL_H__
#define __GPGMEPP_GLOBAL_H__

#include "flags.h"
#include "gpgmefw.h"
#include "gpgmepp_export.h"

#include <iosfwd>
#include <cstring>

namespace GpgME
{
class Error;
class EngineInfo;
class Context;
}

struct _GIOChannel;
typedef struct _GIOChannel      GIOChannel;
class QIODevice;

namespace GpgME
{

GPGMEPP_EXPORT void initializeLibrary();
/*!
  Initializes the library, returns Error::code() ==
  GPG_ERR_USER_1 if underlying gpgme is too old.
*/
GPGMEPP_EXPORT Error initializeLibrary(int);

enum Protocol { OpenPGP, CMS, UnknownProtocol };

enum Engine { GpgEngine, GpgSMEngine, GpgConfEngine, UnknownEngine, AssuanEngine, G13Engine, SpawnEngine };

enum KeyListMode {
    Local = 0x1,
    Extern = 0x2,
    Locate = Local|Extern,
    Signatures = 0x4,
    SignatureNotations = 0x8,
    Validate = 0x10,
    Ephemeral = 0x20,
    WithTofu = 0x40,
    WithKeygrip = 0x80,
    WithSecret = 0x100,
    ForceExtern = 0x200,
    LocateExternal = Locate|ForceExtern,

    KeyListModeMask = 0x3ff
};

enum SignatureMode {
    NormalSignatureMode = 0,
    Detached = 1,
    Clearsigned = 2,
    SignArchive = 4,
    SignFile = 8,
};

enum class RevocationReason {
    Unspecified = 0,
    Compromised = 1,
    Superseded = 2,
    NoLongerUsed = 3
};

enum class DeletionFlag : unsigned int {
    // Keep in line with GPGME_DELETE_* flags
    AllowSecret = (1 << 0),
    Force = (1 << 1),
};
using DeletionFlags = Flags<DeletionFlag>;
GPGMEPP_DEFINE_ENUM_FLAG_OPERATORS(DeletionFlags)

GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, Protocol proto);
GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, Engine eng);
GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, KeyListMode mode);
GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, SignatureMode mode);

GPGMEPP_EXPORT Error setDefaultLocale(int category, const char *value);

GPGMEPP_EXPORT Context *wait(Error &e, bool hang = true);
typedef void (*IdleFunction)(void);
GPGMEPP_EXPORT IdleFunction registerIdleFunction(IdleFunction idleFunction);

typedef void (*IOCallback)(void *data, int fd);

GPGMEPP_EXPORT EngineInfo engineInfo(Protocol proto);
GPGMEPP_EXPORT EngineInfo engineInfo(Engine engine);
/** Wrapper around gpgme_get_dirinfo. What can be:
homedir, sysconfdir, bindir, libexecdir, libdir,
datadir, localedir, agent-socket, agent-ssh-socket,
dirmngr-socket, uiserver-socket, gpgconf-name, gpg-name,
gpgsm-name, g13-name, keyboxd-name, agent-name, scdaemon-name,
dirmngr-name, pinentry-name, socketdir, gpg-wks-client-name, gpgtar-name.

This may be extended in the future.
*/
GPGMEPP_EXPORT const char *dirInfo(const char *what);

GPGMEPP_EXPORT Error checkEngine(Protocol proto);
GPGMEPP_EXPORT Error checkEngine(Engine engine);

/* Wrapper for gpgme_set_global_flag */
GPGMEPP_EXPORT int setGlobalFlag(const char *name, const char *value);

GPGMEPP_EXPORT GIOChannel *getGIOChannel(int fd);
GPGMEPP_EXPORT QIODevice   *getQIODevice(int fd);

} // namespace GpgME

# ifndef GPGMEPP_MAKE_STD_SWAP_SPECIALIZATION
#  define GPGMEPP_MAKE_STD_SWAP_SPECIALIZATION( Class ) \
    namespace std { template <> inline void swap< GpgME::Class >( GpgME::Class & lhs, GpgME::Class & rhs ) { lhs.swap( rhs ); } }
# endif

inline int _gpgmepp_strcmp(const char *s1, const char *s2)
{
    return s1 ? s2 ? std::strcmp(s1, s2) : 1 : s2 ? -1 : 0;
}

#define _GPGMEPP_MAKE_STRCMP( Name, expr, cmp )                     \
    template <template <typename U> class Op>                           \
    struct Name {                                                       \
        typedef bool result_type;                                       \
        \
        bool operator()( const char * lhs, const char * rhs ) const {   \
            return Op<int>()( cmp, 0 );                                 \
        }                                                               \
        \
        bool operator()( const std::string & lhs, const std::string & rhs ) const { \
            return operator()( lhs.c_str(), rhs.c_str() );              \
        }                                                               \
        bool operator()( const char * lhs, const std::string & rhs ) const { \
            return operator()( lhs, rhs.c_str() );                      \
        }                                                               \
        bool operator()( const std::string & lhs, const char * rhs ) const { \
            return operator()( lhs.c_str(), rhs );                      \
        }                                                               \
        \
        template <typename T>                                           \
        bool operator()( const T & lhs, const T & rhs ) const {         \
            return operator()( (lhs expr), (rhs expr) );                \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const char * rhs ) const {      \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const char * lhs, const T & rhs ) const {      \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const std::string & rhs ) const { \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const std::string & lhs, const T & rhs ) const {    \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
    }

#define GPGMEPP_MAKE_STRCMP( Name, expr )                          \
    _GPGMEPP_MAKE_STRCMP( Name, expr, _gpgmepp_strcmp( lhs, rhs ) )


#endif // __GPGMEPP_GLOBAL_H__
