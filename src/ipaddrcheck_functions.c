/*
 * ipaddrcheck_functions.c: IPv4/IPv6 validation functions for ipaddrcheck
 *
 * Copyright (C) 2013 Daniil Baturin
 * Copyright (C) 2018-2024 VyOS maintainers and contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <netinet/in.h>
#include <assert.h>

#include "ipaddrcheck_functions.h"

/*
 * Address string functions
 *
 * Note that they perform format check only
 * and must not be used to deermine if it's
 * a valid address, only what type of address
 * format it is.
 *
 * The only reason they exist is that libcidr
 * is very liberal on its input format and
 * doesn't provide any information on what
 * the format was.
 */

int regex_matches(const char* regex, const char* str)
{
    int offsets[1];
    pcre *re;
    int rc;
    const char *error;
    int erroffset;

    re = pcre_compile(regex, 0, &error, &erroffset, NULL);
    assert(re != NULL);

    rc = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, 1);

    if( rc >= 0)
    {
        return RESULT_SUCCESS;
    }
    else
    {
        return RESULT_FAILURE;
    }
}


/* Does it contain more than one double colon?
   IPv6 addresses allow replacing no more than one group of zeros with a '::' shortcut. */
int duplicate_double_colons(char* address_str) {
    return regex_matches(".*(::).*\\1", address_str);
}

/* Is it an IPv4 address with prefix length (e.g., 192.0.2.1/24)? */
int is_ipv4_cidr(char* address_str)
{
    return regex_matches("^((([1-9]\\d{0,2}|0)\\.){3}([1-9]\\d{0,2}|0)\\/([1-9]\\d*|0))$",
        address_str);
}

/* Is it a single dotted decimal address? */
int is_ipv4_single(char* address_str)
{
    return regex_matches("^((([1-9]\\d{0,2}|0)\\.){3}([1-9]\\d{0,2}|0))$",
        address_str);
}

/* Is it an IPv6 address with prefix length (e.g., 2001:db8::1/64)? */
int is_ipv6_cidr(char* address_str)
{
    return regex_matches("^((([0-9a-fA-F\\:])+)(\\/\\d{1,3}))$", address_str);
}

/* Is it a single IPv6 address? */
int is_ipv6_single(char* address_str)
{
    return regex_matches("^(([0-9a-fA-F\\:])+)$", address_str);
}

/* Is it a CIDR-formatted IPv4 or IPv6 address? */
int is_any_cidr(char* address_str)
{
    int result;

    if( (is_ipv4_cidr(address_str) == RESULT_SUCCESS) ||
        (is_ipv6_cidr(address_str) == RESULT_SUCCESS) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it a single IPv4 or IPv6 address? */
int is_any_single(char* address_str)
{
    int result;

    if( (is_ipv4_single(address_str) == RESULT_SUCCESS) ||
        (is_ipv6_single(address_str) == RESULT_SUCCESS) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/*
 * Address checking functions that rely on libcidr
 */

/* Does it look like a valid address of any protocol? */
int is_valid_address(CIDR *address)
{
     int result;

     if( cidr_get_proto(address) != INVALID_PROTO )
     {
          result = RESULT_SUCCESS;
     }
     else
     {
          result = RESULT_FAILURE;
     }

     return(result);
}

/* Is it a correct IPv4 host or subnet address
   with or without net mask */
int is_ipv4(CIDR *address)
{
     int result;

     if( cidr_get_proto(address) == CIDR_IPV4 )
     {
          result = RESULT_SUCCESS;
     }
     else
     {
          result = RESULT_FAILURE;
     }

     return(result);
}

/* Is it a correct IPv4 host address (i.e., not a network address)? */
int is_ipv4_host(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        ((cidr_equals(address, cidr_addr_network(address)) < 0) ||
        (cidr_get_pflen(address) >= 31)) )
    {
         result = RESULT_SUCCESS;
    }
    else
    {
         result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it a correct IPv4 network address? */
int is_ipv4_net(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        (cidr_equals(address, cidr_addr_network(address)) == 0) )
    {
         result = RESULT_SUCCESS;
    }
    else
    {
         result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 broadcast address? */
int is_ipv4_broadcast(CIDR *address)
{
    int result;

    /* The very concept of broadcast address doesn't apply to
       IPv6 and point-to-point (/31) or isolated (/32) IPv4 addresses. */
    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        (cidr_equals(address, cidr_addr_broadcast(address)) == 0 ) &&
        (cidr_get_pflen(address) < 31) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 multicast address? */
int is_ipv4_multicast(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        (cidr_contains(cidr_from_str(IPV4_MULTICAST), address) == 0) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 loopback address? */
int is_ipv4_loopback(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        (cidr_contains(cidr_from_str(IPV4_LOOPBACK), address) == 0) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 link-local address? */
int is_ipv4_link_local(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        (cidr_contains(cidr_from_str(IPV4_LINKLOCAL), address) == 0) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it a private (RFC 1918) IPv4 address? */
int is_ipv4_rfc1918(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV4) &&
        ( (cidr_contains(cidr_from_str(IPV4_RFC1918_A), address) == 0) ||
        (cidr_contains(cidr_from_str(IPV4_RFC1918_B), address) == 0) ||
        (cidr_contains(cidr_from_str(IPV4_RFC1918_C), address) == 0) ) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* is it a correct IPv6 host or a subnet address, with or without network mask? */
int is_ipv6(CIDR *address)
{
     int result;

     if( cidr_get_proto(address) == CIDR_IPV6 )
     {
          result = RESULT_SUCCESS;
     }
     else
     {
          result = RESULT_FAILURE;
     }

     return(result);
}

/* Is it a correct IPv6 host address? */
int is_ipv6_host(CIDR *address)
{
    int result;

    /* We reuse the same logic that prevents IPv4 network addresses
       from being assigned to interfaces (address == network_address),
       but the reason is slightly differnt.

       As per https://www.rfc-editor.org/rfc/rfc4291 section 2.6.1,
       >[Subnet-Router anycast address] is syntactically
       >the same as a unicast address for an interface on the link with the
       >interface identifier set to zero.

       So, the first address of the subnet must not be used for link addresses,
       even if the semantic reason is different.
       There's absolutely nothing wrong with assigning the last address, though,
       since there's no broadcast in IPv6.
      */

    if( (cidr_get_proto(address) == CIDR_IPV6) &&
        ((cidr_equals(address, cidr_addr_network(address)) < 0) ||
        (cidr_get_pflen(address) >= 127)) )
    {
         result = RESULT_SUCCESS;
    }
    else
    {
         result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it a correct IPv6 network address? */
int is_ipv6_net(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV6) &&
        (cidr_equals(address, cidr_addr_network(address)) == 0) )
    {
         result = RESULT_SUCCESS;
    }
    else
    {
         result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv6 multicast address? */
int is_ipv6_multicast(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV6) &&
        (cidr_contains(cidr_from_str(IPV6_MULTICAST), address) == 0) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv6 link-local address? */
int is_ipv6_link_local(CIDR *address)
{
    int result;

    if( (cidr_get_proto(address) == CIDR_IPV6) &&
        (cidr_contains(cidr_from_str(IPV6_LINKLOCAL), address) == 0) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an address that can be assigned to a network interface?
   (i.e., is it a host address that is not reserved for any special use)
 */
int is_valid_intf_address(CIDR *address, char* address_str, int allow_loopback)
{
    int result;

    if( (is_ipv4_broadcast(address) == RESULT_FAILURE) &&
        (is_ipv4_multicast(address) == RESULT_FAILURE) &&
        (is_ipv6_multicast(address) == RESULT_FAILURE) &&
        ((is_ipv4_loopback(address) == RESULT_FAILURE) || (allow_loopback == LOOPBACK_ALLOWED)) &&
        (cidr_equals(address, cidr_from_str(IPV6_LOOPBACK)) != 0) &&
        (cidr_equals(address, cidr_from_str(IPV4_UNSPECIFIED)) != 0) &&
        (cidr_contains(cidr_from_str(IPV4_THIS), address) != 0) &&
        (cidr_equals(address, cidr_from_str(IPV4_LIMITED_BROADCAST)) != 0) &&
        (is_any_host(address) == RESULT_SUCCESS) &&
        (is_any_cidr(address_str) == RESULT_SUCCESS) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 or IPv6 host address? */
int is_any_host(CIDR *address)
{
    int result;

    if( (is_ipv4_host(address) == RESULT_SUCCESS) ||
        (is_ipv6_host(address) == RESULT_SUCCESS) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Is it an IPv4 or IPv6 network address? */
int is_any_net(CIDR *address)
{
    int result;

    if( (is_ipv4_net(address) == RESULT_SUCCESS) ||
        (is_ipv6_net(address) == RESULT_SUCCESS) )
    {
        result = RESULT_SUCCESS;
    }
    else
    {
        result = RESULT_FAILURE;
    }

    return(result);
}

/* Split a hyphen-separated range into its left and right components.
 * This function is patently unsafe,
 * whether it's safe to do what it does should be determined by its callers.
 */
void split_range(char* range_str, char* left, char* right)
{
    char* ptr = left;
    int length = strlen(range_str);
    int pos = 0;
    int index = 0;
    while(pos < length)
    {
        if( range_str[pos] == '-' )
        {
            ptr[index] = '\0';
            ptr = right;
            index = 0;
        }
        else
        {
            ptr[index] = range_str[pos];
            index++;
        }

        pos++;
    }
    ptr[index] = '\0';

    return;
}

/* in6_addr fields are byte arrays, so we cannot compare them as numbers
 * and needs custom comparison logic
 */
int compare_ipv6(struct in6_addr *left, struct in6_addr *right)
{
    int i = 0;
    for( i = 0; i < 16; i++ )
    {
        if (left->s6_addr[i] < right->s6_addr[i])
            return -1;
        else if (left->s6_addr[i] > right->s6_addr[i])
            return 1;
    }
    return 0;
}

/* Is it a valid IPv4 address range? */
int is_ipv4_range(char* range_str, int prefix_length, int verbose)
{
    int result = RESULT_SUCCESS;

    int regex_check_res = regex_matches("^([0-9\\.]+\\-[0-9\\.]+)$", range_str);

    if( !regex_check_res )
    {
        if( verbose )
        {
            fprintf(stderr, "Malformed range %s: must be a pair of hyphen-separated IPv4 addresses\n", range_str);
        }
        result = RESULT_FAILURE;
    }
    else
    {
       /* Extract sub-components from the range string. */

        /* Allocate memory for the components of the range.
           We need at most 15 characters for an IPv4 address, plus space for the terminating null byte. */
        char left[16];
        char right[16];

        /* Split the string at the hyphen.
           If the regex check succeeded, we know the hyphen is there. */
        split_range(range_str, left, right);

        CIDR* left_addr = cidr_from_str(left);
        CIDR* right_addr = cidr_from_str(right);

        if( !(is_ipv4_single(left) && is_valid_address(left_addr)) )
        {
            if( verbose )
            {
                fprintf(stderr, "Malformed range %s: %s is not a valid IPv4 address\n", range_str, left);
            }
            result = RESULT_FAILURE;
        }
        else if( !(is_ipv4_single(right) && is_valid_address(right_addr)) )
        {
            if( verbose )
            {
                fprintf(stderr, "Malformed range %s: %s is not a valid IPv4 address\n", range_str, right);
            }
            result = RESULT_FAILURE;
        }
        else
        {
            struct in_addr* left_in_addr = cidr_to_inaddr(left_addr, NULL);
            struct in_addr* right_in_addr = cidr_to_inaddr(right_addr, NULL);

            if( left_in_addr->s_addr <= right_in_addr->s_addr )
            {
                /* If non-zero prefix_length is given,
                   check if the right address is within the network of the first one. */
                if( prefix_length > 0 )
                {
                    char left_pref_str[19];

                    /* XXX: Prefix length size is checked elsewhere, so it can't be more than 2 characters (32)
                       and overflow cannot occur.
                     */
                    sprintf(left_pref_str, "%s/%u", left, prefix_length);
                    CIDR* left_addr_with_pref = cidr_from_str(left_pref_str);
                    CIDR* left_net = cidr_addr_network(left_addr_with_pref);
                    if( cidr_contains(left_net, right_addr) == 0 )
                    {
                        result = RESULT_SUCCESS;
                    }
                    else
                    {
                        result = RESULT_FAILURE;
                    }
                    cidr_free(left_addr_with_pref);
                    cidr_free(left_net);
                }
                else
                {
                    result = RESULT_SUCCESS;
                }
            }
            else
            {
                if( verbose )
                {
                    fprintf(stderr, "Malformed IPv4 range %s: its first address is greater than the last\n", range_str);
                }
                result = RESULT_FAILURE;
            }

        }
        cidr_free(left_addr);
        cidr_free(right_addr);
    }

    return(result);
}

/* Is it a valid IPv6 address range? */
int is_ipv6_range(char* range_str, int prefix_length, int verbose)
{
    int result = RESULT_SUCCESS;

    int regex_check_res = regex_matches("^([0-9a-fA-F:]+\\-[0-9a-fA-F:]+)$", range_str);

    if( !regex_check_res )
    {
        if( verbose )
        {
            fprintf(stderr, "Malformed range %s: must be a pair of hyphen-separated IPv6 addresses\n", range_str);
        }
        result = RESULT_FAILURE;
    }
    else
    {
       /* Extract sub-components from the range string. */

        /* Allocate memory for the components of the range.
           We need at most 39 characters for an IPv6 address, plus space for the terminating null byte. */
        char left[40];
        char right[40];

        /* Split the string at the hyphen.
           If the regex check succeeded, we know the hyphen is there. */
        split_range(range_str, left, right);

        CIDR* left_addr = cidr_from_str(left);
        CIDR* right_addr = cidr_from_str(right);

        if( !(is_ipv6_single(left) &&
              is_valid_address(left_addr) && !duplicate_double_colons(left)) )
        {
            if( verbose )
            {
                fprintf(stderr, "Malformed range %s: %s is not a valid IPv6 address\n", range_str, left);
            }
            result = RESULT_FAILURE;
        }
        else if( !(is_ipv6_single(right) &&
                   is_valid_address(right_addr) && !duplicate_double_colons(right)) )
        {
            if( verbose )
            {
                fprintf(stderr, "Malformed range %s: %s is not a valid IPv6 address\n", range_str, right);
            }
            result = RESULT_FAILURE;
        }
        else
        {
            struct in6_addr* left_in6_addr = cidr_to_in6addr(left_addr, NULL);
            struct in6_addr* right_in6_addr = cidr_to_in6addr(right_addr, NULL);

            if( compare_ipv6(left_in6_addr, right_in6_addr) <= 0 )
            {
                /* If non-zero prefix_length is given,
                   check if the right address is within the network of the first one. */
                if( prefix_length > 0 )
                {
                    char left_pref_str[44];

                    /* XXX: Prefix length size is checked elsewhere, so it can't be more than 3 characters (128)
                       and overflow cannot occur.
                     */
                    sprintf(left_pref_str, "%s/%u", left, prefix_length);
                    CIDR* left_addr_with_pref = cidr_from_str(left_pref_str);
                    CIDR* left_net = cidr_addr_network(left_addr_with_pref);
                    if( cidr_contains(left_net, right_addr) == 0 )
                    {
                        result = RESULT_SUCCESS;
                    }
                    else
                    {
                        result = RESULT_FAILURE;
                    }
                    cidr_free(left_addr_with_pref);
                    cidr_free(left_net);
                }
                else
                {
                    result = RESULT_SUCCESS;
                }
            }
            else
            {
                if( verbose )
                {
                    fprintf(stderr, "Malformed IPv6 range %s: its first address is greater than the last\n", range_str);
                }
                result = RESULT_FAILURE;
            }
        }
        cidr_free(left_addr);
        cidr_free(right_addr);
    }

    return(result);
}

