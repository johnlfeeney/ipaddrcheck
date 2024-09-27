#!/bin/bash
#
# integration_tests.sh: ipaddrcheck integration tests
#
# Copyright (C) 2013 Daniil Baturin
# Copyright (C) 2018-2024 VyOS maintainers and contributors
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 or later as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

. ./assert.sh

IPADDRCHECK=../src/ipaddrcheck

# test data
ipv4_single_positive=(
    192.0.2.1
    192.0.2.0
    0.0.0.0
    0.0.0.1
    255.255.255.255
)

ipv4_single_negative=(
    192.0.2.666
    500.0.2.1
)

ipv4_cidr_positive=(
    192.0.2.1/0
    192.0.2.1/32
    192.0.2.1/24
    192.0.2.0/26
)

ipv4_cidr_negative=(
    192.0.2.1/33
    192.0.2.666/32
)

ipv4_range_positive=(
    192.0.2.0-192.0.2.100
)

ipv4_range_negative=(
    192.0.2.-192.0.2.100
    192.0.2.0-
    192.0.2.200-192.0.2.100
    192.0.2.1-192.0.2.500
)

ipv6_range_positive=(
    2001:db8::1-2001:db8::99
)

ipv6_range_negative=(
    2001:db8:xx-2001:db8::99
    2001:db:-
    2001:db8::99-2001:db8::1
    2001::db8::1:1-2001::db8::1::10
    2001:db8:pqrs::1-2001:db8:uvwx::100
)

ipv6_single_positive=(
    2001:0db8:0000:0000:0000:ff00:0042:8329
    2001:db8:0:0:0:ff00:42:8329
    2001:db8::1
    ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
    0000::
    ::1
    ::
)

ipv6_single_negative=(
    gggg::ffff
    2001:db8::bad::f00d
)

ipv6_cidr_positive=(
    2001:db8::/0
    2001:db8::/128
    2001:db8::/56
    ::/0
)

ipv6_cidr_negative=(
    2001:db8::/129
)

ipv4_host_positive=(
    192.0.2.1/24
    192.0.2.100/32
    192.0.2.100/31
)

ipv4_host_negative=(
    192.0.2.0/24
    192.0.2.64/27
)

ipv6_host_positive=(
    2001:db8:1::1/64
    2001:db8:1::1/127
)

ipv6_host_negative=(
    2001:db8::/32
)

string="garbage"

# begin ipaddrcheck_integration

# --is-valid
for address in \
    ${ipv4_single_positive[*]} \
    ${ipv4_cidr_positive[*]} \
    ${ipv6_single_positive[*]} \
    ${ipv6_cidr_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-valid $address" 0
done

for address in \
    ${ipv4_single_negative[*]} \
    ${ipv4_cidr_negative[*]} \
    ${ipv6_single_negative[*]} \
    ${ipv6_cidr_negative[*]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-valid $address" 1
done

# --is-any-cidr
for address in \
    ${ipv4_cidr_positive[*]} \
    ${ipv6_cidr_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-any-cidr $address" 0
done

for address in \
    ${ipv4_single_positive[0]} \
    ${ipv4_cidr_negative[*]} \
    ${ipv6_single_positive[0]} \
    ${ipv6_cidr_negative[*]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-any-cidr $address" 1
done


# --is-any-single
for address in \
    ${ipv4_single_positive[*]} \
    ${ipv6_single_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-any-single $address" 0
done

for address in \
    ${ipv4_single_negative[*]} \
    ${ipv4_cidr_postive[0]} \
    ${ipv6_single_negative[*]} \
    ${ipv6_cidr_postitive[0]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-any-single $address" 1
done


# --is-ipv4
for address in \
    ${ipv4_single_positive[*]} \
    ${ipv4_cidr_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv4 $address" 0
done

for address in \
    ${ipv4_single_negative[*]} \
    ${ipv4_cidr_negative[*]} \
    ${ipv6_single_positive[0]} \
    ${ipv6_cidr_positive[0]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-ipv4 $address" 1
done

# --is-ipv4-cidr
for address in \
    ${ipv4_cidr_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv4-cidr $address" 0
done

for address in \
    ${ipv4_single_positive[0]} \
    ${ipv4_cidr_negative[*]} \
    ${ipv6_single_positive[0]} \
    ${ipv6_cidr_positive[0]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-ipv4-cidr $address" 1
done

# --is-ipv4-single
for address in \
    ${ipv4_single_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv4-single $address" 0
done

for address in \
    ${ipv4_single_negative[*]} \
    ${ipv4_cidr_postive[0]} \
    ${ipv6_single_positive[0]} \
    ${ipv6_cidr_positive[0]} \
    $string
do
    assert_raises "$IPADDRCHECK --is-ipv4-single $address" 1
done

# --is-any-host
for address in \
    ${ipv4_host_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-any-host $address" 0
done

for address in \
    ${ipv4_host_negative[*]}
do
    assert_raises "$IPADDRCHECK --is-any-host $address" 1
done

for address in \
    ${ipv6_host_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-any-host $address" 0
done

for address in \
    ${ipv6_host_negative[*]}
do
    assert_raises "$IPADDRCHECK --is-any-host $address" 1
done


# --is-any-net
# --is-ipv4-host
# --is-ipv4-net
# --is-ipv4-broadcast
# --is-ipv4-multicast
# --is-ipv4-loopback
# --is-ipv4-link-local
# --is-ipv4-rfc1918
# --is-ipv6
# --is-ipv6-cidr
# --is-ipv6-single
# --is-ipv6-host
# --is-ipv6-net
# --is-ipv6-multicast
# --is-ipv6-link-local
# --is-valid-intf-address

# --is-ipv4-range
for range in \
    ${ipv4_range_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv4-range $range" 0
done

for range in \
    ${ipv4_range_negative[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv4-range $range" 1
done

assert_raises "$IPADDRCHECK --range-prefix-length 24 --is-ipv4-range 10.0.0.1-10.0.0.10" 0
assert_raises "$IPADDRCHECK --range-prefix-length 29 --is-ipv4-range 10.0.0.1-10.0.0.10" 1

# --is-ipv6-range
for range in \
    ${ipv6_range_positive[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv6-range $range" 0
done

for range in \
    ${ipv6_range_negative[*]}
do
    assert_raises "$IPADDRCHECK --is-ipv6-range $range" 1
done

assert_raises "$IPADDRCHECK --range-prefix-length 64 --is-ipv6-range 2001:db8::1-2001:db8::100" 0
assert_raises "$IPADDRCHECK --range-prefix-length 64 --is-ipv6-range 2001:db8:aaaa::1-2001:db8:bbbb::1" 1

assert_end ipaddrcheck_integration
