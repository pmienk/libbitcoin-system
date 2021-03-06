/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_EC_PRIVATE_HPP
#define LIBBITCOIN_SYSTEM_EC_PRIVATE_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <bitcoin/system/compat.hpp>
#include <bitcoin/system/define.hpp>
#include <bitcoin/system/math/checksum.hpp>
#include <bitcoin/system/math/ec_scalar.hpp>
#include <bitcoin/system/math/elliptic_curve.hpp>
#include <bitcoin/system/math/hash.hpp>
#include <bitcoin/system/wallet/ec_public.hpp>

namespace libbitcoin {
namespace system {
namespace wallet {

class payment_address;

/// Private keys with public key compression metadata:
static BC_CONSTEXPR size_t wif_uncompressed_size = 37u;
typedef byte_array<wif_uncompressed_size> wif_uncompressed;

static BC_CONSTEXPR size_t wif_compressed_size = wif_uncompressed_size + 1u;
typedef byte_array<wif_compressed_size> wif_compressed;

/// Use to pass an ec secret with compression and version information.
class BC_API ec_private
  : public ec_scalar
{
public:
    static const uint8_t compressed_sentinel;

    // WIF carries a compression flag for payment address generation but
    // assumes a mapping to payment address version. This is insufficient
    // as a parameterized mapping is required, so we use the same technique as
    // with hd keys, merging the two necessary values into one version.
    static const uint8_t mainnet_wif;
    static const uint8_t mainnet_p2kh;
    static const uint16_t mainnet;

    static const uint8_t testnet_wif;
    static const uint8_t testnet_p2kh;
    static const uint16_t testnet;

    static uint8_t to_address_prefix(uint16_t version)
    {
        return version & 0x00FF;
    }

    static uint8_t to_wif_prefix(uint16_t version)
    {
        return version >> 8;
    }

    // Unfortunately can't use this below to define mainnet (MSVC).
    static uint16_t to_version(uint8_t address, uint8_t wif)
    {
        return uint16_t(wif) << 8 | address;
    }

    /// Constructors.
    ec_private();
    ec_private(const ec_private& other);
    ec_private(const ec_scalar& scalar, uint8_t version=mainnet_p2kh);
    ec_private(const data_chunk& seed, uint8_t version=mainnet_p2kh);
    ec_private(const std::string& wif, uint8_t version=mainnet_p2kh);
    ec_private(const wif_compressed& wif, uint8_t version=mainnet_p2kh);
    ec_private(const wif_uncompressed& wif, uint8_t version=mainnet_p2kh);

    /// The version is 16 bits. The most significant byte is the WIF prefix and
    /// the least significant byte is the address perfix. 0x8000 by default.
    ec_private(const ec_secret& secret, uint16_t version=mainnet,
        bool compress=true);

    /// Operators.
    ec_private& operator=(ec_private other);
    bool operator<(const ec_private& other) const;
    bool operator==(const ec_private& other) const;
    bool operator!=(const ec_private& other) const;
    friend std::istream& operator>>(std::istream& in, ec_private& to);
    friend std::ostream& operator<<(std::ostream& out, const ec_private& of);

    // Swap implementation required to properly handle base class.
    friend void swap(ec_private& left, ec_private& right);

    /// Serializer.
    std::string encoded() const;

    /// Accessors.
    uint16_t version() const;
    uint8_t payment_version() const;
    uint8_t wif_version() const;
    bool compressed() const;

    /// Methods.
    ec_public to_public() const;
    payment_address to_payment_address() const;

private:
    /// Validators.
    static bool is_wif(const data_slice& decoded);

    /// Factories.
    static ec_private from_seed(const data_chunk& seed, uint8_t address_version);
    static ec_private from_string(const std::string& wif, uint8_t version);
    static ec_private from_compressed(const wif_compressed& wif, uint8_t version);
    static ec_private from_uncompressed(const wif_uncompressed& wif, uint8_t version);

    /// Members.
    /// These should be const, apart from the need to implement assignment.
    bool compress_;
    uint16_t version_;
};

} // namespace wallet
} // namespace system
} // namespace libbitcoin

#endif
