/*
  randomresults.h - wrappers of the gpgme random results
  Copyright (C) 2025 g10 Code GmbH
  Software engineering by Ingo Klöcker <dev@ingo-kloecker.de>

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
  along with this program; if not, see <https://gnu.org/licenses/>.
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __GPGMEPP_RANDOMRESULTS_H__
#define __GPGMEPP_RANDOMRESULTS_H__

#include "result.h"

#include <vector>

namespace GpgME
{

class GPGMEPP_EXPORT RandomBytesResult : public Result
{
public:
    using value_type = std::vector<unsigned char>;
    RandomBytesResult()
        : Result{GPG_ERR_NO_VALUE}
    {
    }
    explicit RandomBytesResult(const value_type &value)
        : mValue{value}
    {
    }
    explicit RandomBytesResult(value_type &&value)
        : mValue{std::move(value)}
    {
    }
    explicit RandomBytesResult(const Error &err)
        : Result{err}
    {
    }

    const value_type &value() const
    {
        return mValue;
    }

private:
    value_type mValue;
};

class GPGMEPP_EXPORT RandomValueResult : public Result
{
public:
    RandomValueResult()
        : Result{GPG_ERR_NO_VALUE}
    {
    }
    explicit RandomValueResult(unsigned int value)
        : mValue{value}
    {
    }
    explicit RandomValueResult(const Error &err)
        : Result{err}
    {
    }

    unsigned int value() const
    {
        return mValue;
    }

private:
    unsigned int mValue = 0;
};

class GPGMEPP_EXPORT RandomZBase32StringResult : public Result
{
public:
    RandomZBase32StringResult()
        : Result{GPG_ERR_NO_VALUE}
    {
    }
    explicit RandomZBase32StringResult(const std::string &value)
        : mValue{value}
    {
    }
    explicit RandomZBase32StringResult(std::string &&value)
        : mValue{std::move(value)}
    {
    }
    explicit RandomZBase32StringResult(const Error &err)
        : Result{err}
    {
    }

    const std::string &value() const
    {
        return mValue;
    }

private:
    std::string mValue;
};

} // namespace GpgME

#endif /* __GPGMEPP_RANDOMRESULTS_H__ */
