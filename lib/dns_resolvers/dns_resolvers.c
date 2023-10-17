/*
 * Copyright (C) 2023 by Ryan Dimsey <ryan@omnitouch.com.au>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <resolv.h>
#include "naptr.h"
#include "regex_extensions.h"
#include "dns_resolvers.h"
#include "ogs-dns-resolvers-logging.h"

enum { MAX_ANSWER_BYTES = 1024 };


static bool build_domain_name(ResolverContext * const context);
static naptr_resource_record * filter_nrrs(ResolverContext const * const context, naptr_resource_record *nrrs);
static bool should_remove(ResolverContext const * const context, naptr_resource_record *nrr);
static void transform_domain_name(naptr_resource_record *nrr, char * dname);
static int type_ip_query(char lookup_type, char * dname, char * buf, size_t buf_sz);
static void debug_print_nrr(naptr_resource_record *nrr);


bool resolve_naptr(ResolverContext * const context, char *buf, size_t buf_sz) {
    bool resolved = false;
    naptr_resource_record *nrr = NULL;
    naptr_resource_record *nrr_list = NULL;

    if ((NULL == context) || (NULL == buf)) return false;

    /* Build domain name */
    build_domain_name(context);
    ogs_debug("Built domain name : '%s'", context->_domain_name);

    /* Get all NRRs */
    nrr_list = naptr_query(context->_domain_name);
    if (NULL == nrr_list) return false;
    ogs_debug("NAPTR query returned %i results", naptr_resource_record_list_count(nrr_list));

    /* Remove all the NRRs that don't provide the desired service */
    nrr_list = filter_nrrs(context, nrr_list);
    ogs_debug("NAPTR list count after filter %i", naptr_resource_record_list_count(nrr_list));

    /* Sort the NRRs so that we can resolve them in order of priority */
    nrr_list = naptr_list_head(nrr_list);
    nrr = naptr_sort(&nrr_list);

    while (nrr != NULL) {
        /* Update domain name */
        transform_domain_name(nrr, context->_domain_name);

        /* Go through the NRRs until we get an IP */
        int num_ips = type_ip_query(nrr->flag, context->_domain_name, buf, buf_sz);

        if (0 < num_ips) {
            ogs_debug("Resolve successful, IP is '%s'", buf);
            resolved = true;
            break;
        }

        nrr = nrr->next;
    }

    naptr_free_resource_record_list(nrr_list);

    return resolved;
}

static bool build_domain_name(ResolverContext * const context) {
    int chars_written = 0;
    bool build_success = false;

    if (NULL == context) return false;

    /* If we don't have an APN specified we use the target */
    if (0 == strlen(context->apn)) {
        chars_written = snprintf(
            context->_domain_name,
            DNS_RESOLVERS_MAX_DOMAIN_NAME_STR,
            "epc.mnc%s.mcc%s.%s", 
            context->mnc,
            context->mcc,
            context->domain_suffix
        );
    } else {
        chars_written = snprintf(
            context->_domain_name,
            DNS_RESOLVERS_MAX_DOMAIN_NAME_STR,
            "%s.apn.epc.mnc%s.mcc%s.%s", 
            context->apn,
            context->mnc,
            context->mcc,
            context->domain_suffix
        );
    }

    if (chars_written < DNS_RESOLVERS_MAX_DOMAIN_NAME_STR) {
        build_success = true;
    }

    return build_success;
}

/**
 * Cases:
 *   1) If filter results in empty list NULL will be returned
 *   2) If filter results in non-empty list then the last node
 *      is returned.
 */
static naptr_resource_record * filter_nrrs(ResolverContext const * const context, naptr_resource_record *nrr) {
    naptr_resource_record *prev = NULL;
    naptr_resource_record *next = NULL;

    if ((NULL == context) || (NULL == nrr)) return NULL;

    nrr = naptr_list_head(nrr);

    while (NULL != nrr) {
        next = nrr->next;

        if (should_remove(context, nrr)) {
            nrr = naptr_remove_resource_record(nrr);
        }

        prev = nrr;
        nrr = next;
    }

    return prev;
}

static bool has_appropriate_services(ResolverContext const * const context, naptr_resource_record *nrr) {
    bool has_appropriate_services = false;
    enum { DESIRED_STR_LEN = 128 };
    char desired_target_string[DESIRED_STR_LEN] = "";
    char desired_service_string[DESIRED_STR_LEN] = "";

    if ((NULL == context) || (NULL == nrr)) return NULL;

    /* Build our desired services strings */
    snprintf(desired_target_string, DESIRED_STR_LEN, "x-3gpp-%s", context->target);
    snprintf(desired_service_string, DESIRED_STR_LEN, "x-%s-%s", context->interface, context->protocol);

    ogs_debug("Testing for appropriate service");
    ogs_debug("Interface string       : '%s'", context->interface);
    ogs_debug("Protocol string        : '%s'", context->protocol);
    ogs_debug("Desired service string : '%s'", desired_service_string);
    ogs_debug("Desired target string  : '%s'", desired_target_string);
    ogs_debug("Actual target string   : '%s'", context->target);

    if ((NULL != strstr(nrr->service, desired_service_string)) &&
        (NULL != strstr(nrr->service, desired_target_string))) {
        has_appropriate_services = true;
    }

    return has_appropriate_services;
}

static bool has_replace_has_no_regex(ResolverContext const * const context, naptr_resource_record *nrr) {
    bool has_replace_has_no_regex = false;

    if ((NULL == context) || (NULL == nrr)) return NULL;

    ogs_debug("Replacement field is : '%s'\n", nrr->replacement);
    ogs_debug("Pattern field is     : '%s'\n", nrr->regex_pattern);

    /* Has replacement field */
    if ((0 < strlen(nrr->replacement)) &&
        (0 != strcmp(nrr->replacement, "."))) {

        /* Has no regex fields */
        if ((0 == strlen(nrr->regex_pattern)) &&
            (0 == strlen(nrr->regex_pattern))) {
            has_replace_has_no_regex = true;     
        }
    }

    return has_replace_has_no_regex;
}

static bool has_regex_match(ResolverContext const * const context, naptr_resource_record *nrr) {
    bool has_regex_match = false;

    if ((NULL == context) || (NULL == nrr)) return NULL;

    if (false == reg_match(nrr->regex_pattern, context->_domain_name)) {
        has_regex_match = true;
    }

    return has_regex_match;
}

/*
 * RFC 2915 (4. The Basic NAPTR Algorithm)
 * 
 * NAPTR records for this key are retrieved, those with unknown Flags or
 * inappropriate Services are discarded and the remaining records are
 * sorted by their Order field.  Within each value of Order, the records
 * are further sorted by the Preferences field.
 * 
 * The records are examined in sorted order until a matching record is
 * found.  A record is considered a match iff:
 *   - it has a Replacement field value instead of a Regexp field value.
 *   - or the Regexp field matches the string held by the client.
 * 
 * TLDR:
 *   We only keep if:
 *     - Known flag
 *     - Appropriate services
 *     - It has a replacement field AND no regex field
 *     - It has a regex field that matches the domain name
 */
static bool should_remove(ResolverContext const * const context, naptr_resource_record *nrr) {
    bool should_remove = false;

    if ((NULL == context) || (NULL == nrr)) return true;

    if (false == has_appropriate_services(context, nrr)) {
        ogs_debug("Excluding this peer due to not handling desired services");
        should_remove = true;
    }
    
    else if (true == has_replace_has_no_regex(context, nrr)) {
        ogs_debug("Peer is valid as it has a replacement field AND no regex field");
        should_remove = false;
    } else if (true == has_regex_match(context, nrr)) {
        ogs_debug("Peer is valid as it has a regex field that matches the domain name");
        should_remove = false;
    } else {
        ogs_debug("Excluding this peer as it has a replacement field AND a regex field OR a regex field that doesn't match");
        should_remove = true;
    }

    if (should_remove) {
        ogs_debug("Filtering following NAPTR record:");
        debug_print_nrr(nrr);
    }

    return should_remove;
}

/*
 * Example (regex replace):
 *   Input:
 *     nrr->regex_pattern = "([a-z0-9]+)(..*)"
 *     nrr->regex_replace = "\\1.apn.epc.mnc999.mcc999.3gppnetwork.org"
 *     nrr->replacement = "."
 *     dname = "mms.apn.epc.mnc001.mcc001.3gppnetwork.org.nickvsnetworking.com"
 *   Output:
 *     dname = "mms.apn.epc.mnc999.mcc999.3gppnetwork.org"
 * 
 * Example (replace):
 *   Input:
 *     nrr->regex_pattern = ""
 *     nrr->regex_replace = ""
 *     nrr->replacement = "www.google.com"
 *     dname = "mms.apn.epc.mnc001.mcc001.3gppnetwork.org.nickvsnetworking.com"
 *   Output:
 *     dname = "www.google.com"
 * 
 * Notes:
 *   If any of the pointers are NULL then the function will immediately return
 *   without making any changes.
 *   We assume that all strings (char*) are correctly terminated.
 *   Assuming that dname is of size DNS_RESOLVERS_MAX_DOMAIN_NAME_STR.
 */
static void transform_domain_name(naptr_resource_record *nrr, char * dname) {
    if ((NULL == nrr) || (NULL == dname)) return;

    /* If a Regex Replaces is set on the DNS entry then evaluate it and apply it */
    if ((0 < strlen(nrr->regex_pattern)) &&
        (0 < strlen(nrr->regex_replace))) {
        int reg_replace_res;
        char temp[DNS_RESOLVERS_MAX_DOMAIN_NAME_STR] = "";

        reg_replace_res = reg_replace(nrr->regex_pattern, nrr->regex_replace, dname, temp, DNS_RESOLVERS_MAX_DOMAIN_NAME_STR);

        if (1 == reg_replace_res) {
            strncpy(dname, temp, DNS_RESOLVERS_MAX_DOMAIN_NAME_STR - 1);
        } else {
            ogs_error("Failed to preform regex replace!");
        }
    } else if (0 != strcmp(nrr->replacement, ".")) {
        strncpy(dname, nrr->replacement, DNS_RESOLVERS_MAX_DOMAIN_NAME_STR - 1);
    } else {
        /* No changes made to domain name */
    }
}

static int type_ip_query(char lookup_type, char * dname, char * buf, size_t buf_sz) {
    int ip_count = 0;
    int resolv_lookup_type; 
    int bytes_received;
    int i;
    int res;
    unsigned char answer[MAX_ANSWER_BYTES];
    ns_rr record;
    ns_msg handle;

    if ((NULL == dname) || (NULL == buf)) return 0;

    if (('a' == lookup_type) || (0 == lookup_type)) {
        resolv_lookup_type = T_A; 
    } else if ('s' == lookup_type) {
        resolv_lookup_type = T_SRV; 
    } else {
        ogs_error("Unsupported lookup type '%c', onlt support 'a' and 's' types", lookup_type);
        return 0;
    }


    /* Send DNS query for lookup type */
    bytes_received = res_query(dname, C_IN, resolv_lookup_type, answer, MAX_ANSWER_BYTES);
    ogs_debug("[%c-lookup] Query for '%s' resulted in %i bytes received", lookup_type, dname, bytes_received);
    if (bytes_received < 0) {
        return 0;
    }

    /* Initialize message handle */
    res = ns_initparse(answer, bytes_received, &handle);
    if (res < 0) {
        ogs_error("Failed to parse query result");
        return 0;
    }

    /* Resolve and return A and SRV records.
     * The last valid one is the one that is
     * returned via buf */
    ogs_debug("[%c-lookup] Looping through %i results to resolve IP", lookup_type, ns_msg_count(handle, ns_s_an));
    for (i = 0; i < ns_msg_count(handle, ns_s_an); i++) {
        res = ns_parserr(&handle, ns_s_an, i, &record);
        if (res < 0) {
            ogs_error("Failed to parse query result");
            return 0;
        }

        if (ns_rr_type(record) == T_A) {
            ogs_debug("Successful parse of A lookup result");
            inet_ntop(AF_INET, ns_rr_rdata(record), buf, buf_sz);
            ++ip_count;
        } else if (ns_rr_type(record) == T_SRV) {
            ogs_debug("Successful parse of SRV lookup result");
            /* Note: SRV is not fully implemented. 
             * We make no effort to order and pick */
            enum { SRV_DATA_TARGET_OFFSET = 6,
                   MAX_TARGET_STR_UNCOMPRESSED = 64 };
            const unsigned char *start = ns_rr_rdata(record);
            const unsigned char *end = start + ns_rr_rdlen(record);
            const unsigned char *target_start = ns_rr_rdata(record) + SRV_DATA_TARGET_OFFSET;
            char target_uncompressed[MAX_TARGET_STR_UNCOMPRESSED] = "";

            int bytes_uncompressed = ns_name_uncompress(
                start,                      /* Start of compressed buffer */
                end,                        /* End of compressed buffer */
                target_start,               /* Where to start decompressing */
                target_uncompressed,        /* Where to store decompressed value */
                MAX_TARGET_STR_UNCOMPRESSED /* Number of bytes that can be stored in output buffer */
            );

            /* Preform an A query based on SRV target */
            if (0 < bytes_uncompressed) {
                ip_count += type_ip_query('a', target_uncompressed, buf, buf_sz);
            }
        }
    }

    return ip_count;
}

static void debug_print_nrr(naptr_resource_record *nrr) {
    ogs_debug("preference    : %i",   nrr->preference);
    ogs_debug("order         : %i",   nrr->order);
    ogs_debug("flag          : '%c'", nrr->flag);
    ogs_debug("service       : '%s'", nrr->service);
    ogs_debug("regex_pattern : '%s'", nrr->regex_pattern);
    ogs_debug("regex_replace : '%s'", nrr->regex_replace);
    ogs_debug("replacement   : '%s'", nrr->replacement);
}
