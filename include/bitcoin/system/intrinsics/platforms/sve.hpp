/**
 * Copyright (c) 2011-2025 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without any implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_INTRINSICS_PLATFORMS_SVE_HPP
#define LIBBITCOIN_SYSTEM_INTRINSICS_PLATFORMS_SVE_HPP

#include <bitcoin/system/define.hpp>

#if defined(HAVE_SVE)
#include <arm_sve.h>

namespace libbitcoin {
namespace system {
} // namespace system
} // namespace libbitcoin

#endif // HAVE_SVE

#endif
