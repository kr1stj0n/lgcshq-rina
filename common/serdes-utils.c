/*
 * Utilities to serialize and deserialize
 *
 *    Eduard Grasa <eduard.grasa@i2cat.net>
 *    Vincenzo Maffione <v.maffione@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "irati/serdes-utils.h"

#define RINA_PREFIX "serdes-utils"

#ifdef __KERNEL__

#include <linux/string.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "logs.h"
#include "rds/rmem.h"
#include "rds/rstr.h"
#include "common.h"

#define COMMON_ALLOC(_sz, _sl)      rkzalloc(_sz, _sl ? GFP_KERNEL : GFP_ATOMIC)
#define COMMON_FREE(_p)             rkfree(_p)
#define COMMON_STRDUP(_s, _sl)      rkstrdup_gfp(_s, _sl ? GFP_KERNEL : GFP_ATOMIC)
#define COMMON_EXPORT(_n)           EXPORT_SYMBOL(_n)
#define COMMON_STATIC

#else /* user-space */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "librina/logs.h"

#define COMMON_ALLOC(_sz, _unused)  malloc(_sz)
#define COMMON_FREE(_p)             free(_p)
#define COMMON_STRDUP(_s, _unused)  strdup(_s)
#define COMMON_EXPORT(_n)
#define COMMON_STATIC               static

#endif

/* Serialize a numeric variable _v of type _t. */
#define serialize_obj(_p, _t, _v)       \
        do {                            \
            *((_t *)_p) = _v;           \
            _p += sizeof(_t);           \
        } while (0)

/* Deserialize a numeric variable of type _t from _p into _r. */
#define deserialize_obj(_p, _t, _r)     \
        do {                            \
            *(_r) = *((_t *)_p);        \
            _p += sizeof(_t);           \
        } while (0)

int rina_sername_valid(const char *str)
{
	const char *orig_str = str;
	int cnt = 0;

	if (!str || strlen(str) == 0) {
		return 0;
	}

	while (*str != '\0') {
		if (*str == ':') {
			if (++ cnt > 3) {
				return 0;
			}
		}
		str ++;
	}

	return (*orig_str == ':') ? 0 : 1;
}
COMMON_EXPORT(rina_sername_valid);

/* Size of a serialized string, not including the storage for the size
 * field itself. */
static unsigned int string_prlen(const char *s)
{
	unsigned int slen;

	slen = s ? strlen(s) : 0;

	return slen > 65535 ? 65535 : slen;
}

/* Size of a serialized IRATI name. */
unsigned int
rina_name_serlen(const struct name *name)
{
	unsigned int ret = 4 * sizeof(uint16_t);

	if (!name) {
		return ret;
	}

	return ret + string_prlen(name->process_name)
			+ string_prlen(name->process_instance)
			+ string_prlen(name->entity_name)
			+ string_prlen(name->entity_instance);
}

/* Serialize a C string. */
void serialize_string(void **pptr, const char *s)
{
	uint16_t slen;

	slen = string_prlen(s);
	serialize_obj(*pptr, uint16_t, slen);

	memcpy(*pptr, s, slen);
	*pptr += slen;
}

/* Deserialize a C string. */
int deserialize_string(const void **pptr, char **s)
{
	uint16_t slen;

	deserialize_obj(*pptr, uint16_t, &slen);
	if (slen) {
		*s = COMMON_ALLOC(slen + 1, 1);
		if (!(*s)) {
			return -1;
		}

		memcpy(*s, *pptr, slen);
		(*s)[slen] = '\0';
		*pptr += slen;
	} else {
		*s = NULL;
	}

	return 0;
}

/* Serialize a RINA name. */
void serialize_rina_name(void **pptr, const struct name *name)
{
	serialize_string(pptr, name->process_name);
	serialize_string(pptr, name->process_instance);
	serialize_string(pptr, name->entity_name);
	serialize_string(pptr, name->entity_instance);
}

/* Deserialize a RINA name. */
int deserialize_rina_name(const void **pptr, struct name *name)
{
	int ret;

	memset(name, 0, sizeof(*name));

	ret = deserialize_string(pptr, &name->process_name);
	if (ret) {
		return ret;
	}

	ret = deserialize_string(pptr, &name->process_instance);
	if (ret) {
		return ret;
	}

	ret = deserialize_string(pptr, &name->entity_name);
	if (ret) {
		return ret;
	}

	ret = deserialize_string(pptr, &name->entity_instance);

	return ret;
}

void rina_name_free(struct name *name)
{
	if (!name) {
		return;
	}

	if (name->process_name) {
		COMMON_FREE(name->process_name);
		name->process_name = NULL;
	}

	if (name->process_instance) {
		COMMON_FREE(name->process_instance);
		name->process_instance = NULL;
	}

	if (name->entity_name) {
		COMMON_FREE(name->entity_name);
		name->entity_name = NULL;
	}

	if (name->entity_instance) {
		COMMON_FREE(name->entity_instance);
		name->entity_instance = NULL;
	}

	COMMON_FREE(name);
}
COMMON_EXPORT(rina_name_free);

void rina_name_move(struct name *dst, struct name *src)
{
	if (!dst || !src) {
		return;
	}

	dst->process_name = src->process_name;
	src->process_name = NULL;

	dst->process_instance = src->process_instance;
	src->process_instance = NULL;

	dst->entity_name = src->entity_name;
	src->entity_name = NULL;

	dst->entity_instance = src->entity_instance;
	src->entity_instance = NULL;
}
COMMON_EXPORT(rina_name_move);

int rina_name_copy(struct name *dst, const struct name *src)
{
	if (!dst || !src) {
		return 0;
	}

#if 0
	rina_name_free(dst);
#endif

	dst->process_name = src->process_name ?
			COMMON_STRDUP(src->process_name, 1) : NULL;
	dst->process_instance = src->process_instance ?
			COMMON_STRDUP(src->process_instance, 1) : NULL;
	dst->entity_name = src->entity_name ?
			COMMON_STRDUP(src->entity_name, 1) : NULL;
	dst->entity_instance = src->entity_instance ?
			COMMON_STRDUP(src->entity_instance, 1) : NULL;

	return 0;
}
COMMON_EXPORT(rina_name_copy);

COMMON_STATIC char *
__rina_name_to_string(const struct name *name, int maysleep)
{
	char *str = NULL;
	char *cur;
	unsigned int apn_len;
	unsigned int api_len;
	unsigned int aen_len;
	unsigned int aei_len;

	if (!name) {
		return NULL;
	}

	apn_len = name->process_name ? strlen(name->process_name) : 0;
	api_len = name->process_instance ? strlen(name->process_instance) : 0;
	aen_len = name->entity_name ? strlen(name->entity_name) : 0;
	aei_len = name->entity_instance ? strlen(name->entity_instance) : 0;

	str = cur = COMMON_ALLOC(apn_len + 1 + api_len + 1 +
			aen_len + 1 + aei_len + 1, maysleep);
	if (!str) {
		return NULL;
	}

	memcpy(cur, name->process_name, apn_len);
	cur += apn_len;

	*cur = ':';
	cur++;

	memcpy(cur, name->process_instance, api_len);
	cur += api_len;

	*cur = ':';
	cur++;

	memcpy(cur, name->entity_name, aen_len);
	cur += aen_len;

	*cur = ':';
	cur++;

	memcpy(cur, name->entity_instance, aei_len);
	cur += aei_len;

	*cur = '\0';

	return str;
}
COMMON_EXPORT(__rina_name_to_string);

char * rina_name_to_string(const struct name *name)
{
    return __rina_name_to_string(name, 1);
}
COMMON_EXPORT(rina_name_to_string);

COMMON_STATIC int
__rina_name_fill(struct name *name, const char *apn,
		const char *api, const char *aen, const char *aei,
		int maysleep)
{
	if (!name) {
		return -1;
	}

	name->process_name = (apn && strlen(apn)) ?
			COMMON_STRDUP(apn, maysleep) : NULL;
	name->process_instance = (api && strlen(api)) ?
			COMMON_STRDUP(api, maysleep) : NULL;
	name->entity_name = (aen && strlen(aen)) ?
			COMMON_STRDUP(aen, maysleep) : NULL;
	name->entity_instance = (aei && strlen(aei)) ?
			COMMON_STRDUP(aei, maysleep) : NULL;

	if ((apn && strlen(apn) && !name->process_name) ||
			(api && strlen(api) && !name->process_instance) ||
			(aen && strlen(aen) && !name->entity_name) ||
			(aei && strlen(aei) && !name->entity_instance)) {
		rina_name_free(name);
		PE("FAILED\n");
		return -1;
	}

	return 0;
}
COMMON_EXPORT(__rina_name_fill);

int
rina_name_fill(struct name *name, const char *apn,
		const char *api, const char *aen, const char *aei)
{
	return __rina_name_fill(name, apn, api, aen, aei, 1);
}
COMMON_EXPORT(rina_name_fill);

COMMON_STATIC int
__rina_name_from_string(const char *str, struct name *name, int maysleep)
{
	char *apn, *api, *aen, *aei;
	char *strc = COMMON_STRDUP(str, maysleep);
	char *strc_orig = strc;
	char **strp = &strc;

	memset(name, 0, sizeof(*name));

	if (!strc) {
		return -1;
	}

	apn = strsep(strp, ":");
	api = strsep(strp, ":");
	aen = strsep(strp, ":");
	aei = strsep(strp, ":");

	if (!apn) {
		/* The ':' are not necessary if some of the api, aen, aei
		 * are not present. */
		COMMON_FREE(strc_orig);
		return -1;
	}

	__rina_name_fill(name, apn, api, aen, aei, maysleep);
	COMMON_FREE(strc_orig);

	return 0;
}
COMMON_EXPORT(__rina_name_from_string);

int
rina_name_from_string(const char *str, struct name *name)
{
	return __rina_name_from_string(str, name, 1);
}

int
rina_name_cmp(const struct name *one, const struct name *two)
{
	if (!one || !two) {
		return !(one == two);
	}

	if (!!one->process_name ^ !!two->process_name) {
		return -1;
	}
	if (one->process_name &&
			strcmp(one->process_name, two->process_name)) {
		return -1;
	}

	if (!!one->process_instance ^ !!two->process_instance) {
		return -1;
	}
	if (one->process_instance &&
			strcmp(one->process_instance, two->process_instance)) {
		return -1;
	}

	if (!!one->entity_name ^ !!two->entity_name) {
		return -1;
	}
	if (one->entity_name && strcmp(one->entity_name, two->entity_name)) {
		return -1;
	}

	if (!!one->entity_instance ^ !!two->entity_instance) {
		return -1;
	}
	if (one->entity_instance &&
			strcmp(one->entity_instance, two->entity_instance)) {
		return -1;
	}

	return 0;
}
COMMON_EXPORT(rina_name_cmp);

int
rina_name_valid(const struct name *name)
{
	if (!name || !name->process_name || strlen(name->process_name) == 0) {
		return 0;
	}

	return 1;
}
COMMON_EXPORT(rina_name_valid);

int flow_spec_serlen(const struct flow_spec * fspec)
{
	return 8 * sizeof(uint32_t) + sizeof(int32_t) + 2* sizeof(bool);
}

void serialize_flow_spec(void **pptr, const struct flow_spec *fspec)
{
	serialize_obj(*pptr, uint32_t, fspec->average_bandwidth);
	serialize_obj(*pptr, uint32_t, fspec->average_sdu_bandwidth);
	serialize_obj(*pptr, uint32_t, fspec->delay);
	serialize_obj(*pptr, uint32_t, fspec->jitter);
	serialize_obj(*pptr, int32_t, fspec->max_allowable_gap);
	serialize_obj(*pptr, uint32_t, fspec->max_sdu_size);
	serialize_obj(*pptr, bool, fspec->ordered_delivery);
	serialize_obj(*pptr, bool, fspec->partial_delivery);
	serialize_obj(*pptr, uint32_t, fspec->peak_bandwidth_duration);
	serialize_obj(*pptr, uint32_t, fspec->peak_sdu_bandwidth_duration);
	serialize_obj(*pptr, int32_t, fspec->undetected_bit_error_rate);
}

int deserialize_flow_spec(const void **pptr, struct flow_spec *fspec)
{
	memset(fspec, 0, sizeof(*fspec));

	deserialize_obj(*pptr, uint32_t, fspec->average_bandwidth);
	deserialize_obj(*pptr, uint32_t, fspec->average_sdu_bandwidth);
	deserialize_obj(*pptr, uint32_t, fspec->delay);
	deserialize_obj(*pptr, uint32_t, fspec->jitter);
	deserialize_obj(*pptr, int32_t, fspec->max_allowable_gap);
	deserialize_obj(*pptr, uint32_t, fspec->max_sdu_size);
	deserialize_obj(*pptr, bool, fspec->ordered_delivery);
	deserialize_obj(*pptr, bool, fspec->partial_delivery);
	deserialize_obj(*pptr, uint32_t, fspec->peak_bandwidth_duration);
	deserialize_obj(*pptr, uint32_t, fspec->peak_sdu_bandwidth_duration);
	deserialize_obj(*pptr, int32_t, fspec->undetected_bit_error_rate);
}

void flow_spec_free(struct flow_spec * fspec)
{
	if (!fspec)
		return;

	COMMON_FREE(fspec);
}

int policy_parm_serlen(const struct policy_parm * prm)
{
	unsigned int ret = 2 * sizeof(uint16_t);

	if (!prm) {
		return ret;
	}

	return ret + string_prlen(prm->name) + string_prlen(prm->value);
}

void serialize_policy_parm(void **pptr, const struct policy_parm *prm)
{
	serialize_string(pptr, prm->name);
	serialize_string(pptr, prm->value);
}

int deserialize_policy_parm(const void **pptr, struct policy_parm *prm)
{
	int ret;

	memset(prm, 0, sizeof(*prm));

	ret = deserialize_string(pptr, &prm->name);
	if (ret) {
		return ret;
	}

	ret = deserialize_string(pptr, &prm->value);

	return ret;
}

void policy_parm_free(const struct policy_parm * prm)
{
	if (!prm) {
		return;
	}

	if (prm->name) {
		COMMON_FREE(prm->name);
		prm->name = NULL;
	}

	if (prm->value) {
		COMMON_FREE(prm->value);
		prm->value = NULL;
	}

	COMMON_FREE(prm);
}

int policy_serlen(const struct policy * policy)
{
	struct policy_parm * pos;

	unsigned int ret = 2 * sizeof(uint16_t);

	if (!policy) {
		return ret;
	}

	ret = ret + string_prlen(policy->name)
		  + string_prlen(policy->version)
		  + sizeof(uint16_t);

        list_for_each_entry(pos, &(policy->params), next) {
                ret = ret + policy_parm_serlen(pos);
        }

        return ret;
}

void serialize_policy(void **pptr, const struct policy *policy)
{
	struct policy_parm * pos;
	uint16_t num_parms;

	serialize_string(pptr, policy->name);
	serialize_string(pptr, policy->version);

	num_parms = 0;
        list_for_each_entry(pos, &(policy->params), next) {
                num_parms ++;
        }

        serialize_obj(*pptr, uint16_t, num_parms);

        list_for_each_entry(pos, &(policy->params), next) {
        	serialize_policy_parm(pos);
        }
}

int deserialize_policy(const void **pptr, struct policy *policy)
{
	int ret;
	uint16_t num_attrs;
	struct policy_parm * pos;

	memset(policy, 0, sizeof(*policy));

	ret = deserialize_string(pptr, &policy->name);
	if (ret) {
		return ret;
	}

	ret = deserialize_string(pptr, &policy->version);
	if (ret) {
		return ret;
	}

	deserialize_obj(*pptr, uint16_t, &num_attrs);
	for(int i = 0; i < num_attrs; i++) {
		pos = COMMON_ALLOC(sizeof(struct policy_parm), 1);
		if (!pos) {
			return -1;
		}

		INIT_LIST_HEAD(&pos->next);
		ret = deserialize_policy_parm(pptr, pos);
		if (ret) {
			return ret;
		}

		list_add_tail(&pos->next, &policy->params);
	}

	return ret;
}

void policy_free(const struct policy * policy)
{
	struct policy_parm * pos, * npos;

	if (!policy)
		return;

	if (policy->name) {
		COMMON_FREE(policy->name);
		policy->name = NULL;
	}

	if (policy->version) {
		COMMON_FREE(policy->version);
		policy->version = NULL;
	}

	list_for_each_entry_safe(pos, npos, &policy->params, next) {
		list_del(&pos->next);
		policy_parm_free(pos);
	}

	COMMON_FREE(policy);
}

int dtp_config_serlen(const struct dtp_config * dtp_config)
{
	return 4 * sizeof(bool) + sizeof(int) + sizeof(timeout_t)
	         + sizeof(seq_num_t) + policy_serlen(dtp_config->dtp_ps);
}

void serialize_dtp_config(void **pptr, const struct dtp_config *dtp_config)
{
	serialize_obj(*pptr, bool, dtp_config->dtcp_present);
	serialize_obj(*pptr, int, dtp_config->seq_num_ro_th);
	serialize_obj(*pptr, timeout_t, dtp_config->initial_a_timer);
	serialize_obj(*pptr, bool, dtp_config->partial_delivery);
	serialize_obj(*pptr, bool, dtp_config->incomplete_delivery);
	serialize_obj(*pptr, bool, dtp_config->in_order_delivery);
	serialize_obj(*pptr, seq_num_t, dtp_config->max_sdu_gap);
	serialize_policy(pptr, dtp_config->dtp_ps);
}

int deserialize_dtp_config(const void **pptr, struct dtp_config *dtp_config)
{
	int ret;

	memset(dtp_config, 0, sizeof(*dtp_config));

	deserialize_obj(*pptr, bool, dtp_config->dtcp_present);
	deserialize_obj(*pptr, int, dtp_config->seq_num_ro_th);
	deserialize_obj(*pptr, timeout_t, dtp_config->initial_a_timer);
	deserialize_obj(*pptr, bool, dtp_config->partial_delivery);
	deserialize_obj(*pptr, bool, dtp_config->incomplete_delivery);
	deserialize_obj(*pptr, bool, dtp_config->in_order_delivery);
	deserialize_obj(*pptr, seq_num_t, dtp_config->max_sdu_gap);

	dtp_config->dtp_ps = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dtp_config->dtp_ps) {
		return -1;
	}

	INIT_LIST_HEAD(&dtp_config->dtp_ps->params);
	return deserialize_policy(pptr, dtp_config->dtp_ps);
}

void dtp_config_free(struct dtp_config * dtp_config)
{
	if (!dtp_config)
		return;

	if (dtp_config->dtp_ps) {
		policy_free(dtp_config->dtp_ps);
		dtp_config->dtp_ps = 0;
	}

	COMMON_FREE(dtp_config);
}

int window_fctrl_config_serlen(const struct window_fctrl_config * wfc)
{
	return 2 * sizeof(uint32_t) + policy_serlen(wfc->rcvr_flow_control)
				    + policy_serlen(wfc->tx_control);
}

void serialize_window_fctrl_config(void **pptr,
				   const struct window_fctrl_config *wfc)
{
	serialize_obj(*pptr, uint32_t, wfc->initial_credit);
	serialize_obj(*pptr, uint32_t, wfc->max_closed_winq_length);
	serialize_policy(pptr, wfc->rcvr_flow_control);
	serialize_policy(pptr, wfc->tx_control);
}

int deserialize_window_fctrl_config(const void **pptr,
				    struct window_fctrl_config *wfc)
{
	int ret;

	memset(wfc, 0, sizeof(*wfc));

	deserialize_obj(*pptr, uint32_t, wfc->initial_credit);
	deserialize_obj(*pptr, uint32_t, wfc->max_closed_winq_length);

	wfc->rcvr_flow_control = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!wfc->rcvr_flow_control) {
		return -1;
	}

	INIT_LIST_HEAD(&wfc->rcvr_flow_control->params);
	ret = deserialize_policy(pptr, wfc->rcvr_flow_control);
	if (ret) {
		return ret;
	}

	wfc->tx_control = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!wfc->tx_control) {
		return -1;
	}

	INIT_LIST_HEAD(&wfc->tx_control->params);
	return deserialize_policy(pptr, wfc->tx_control);
}

void window_fctrl_config_free(struct window_fctrl_config * wfc)
{
	if (!wfc)
		return;

	if (wfc->rcvr_flow_control) {
		policy_free(wfc->rcvr_flow_control);
		wfc->rcvr_flow_control = 0;
	}

	if (wfc->tx_control) {
		policy_free(wfc->tx_control);
		wfc->tx_control = 0;
	}

	COMMON_FREE(wfc);
}

int rate_fctrl_config_serlen(const struct rate_fctrl_config * rfc)
{
	return 2 * sizeof(uint32_t) + policy_serlen(rfc->rate_reduction)
                 + policy_serlen(rfc->no_rate_slow_down)
		 + policy_serlen(rfc->no_override_default_peak);
}

void serialize_rate_fctrl_config(void **pptr, const struct rate_fctrl_config *rfc)
{
	serialize_obj(*pptr, uint32_t, rfc->sending_rate);
	serialize_obj(*pptr, uint32_t, rfc->time_period);
	serialize_policy(pptr, rfc->no_override_default_peak);
	serialize_policy(pptr, rfc->no_rate_slow_down);
	serialize_policy(pptr, rfc->rate_reduction);
}

int deserialize_rate_fctrl_config(const void **pptr, struct rate_fctrl_config *rfc)
{
	int ret;

	memset(rfc, 0, sizeof(*rfc));

	deserialize_obj(*pptr, uint32_t, rfc->sending_rate);
	deserialize_obj(*pptr, uint32_t, rfc->time_period);

	rfc->no_override_default_peak = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rfc->no_override_default_peak) {
		return -1;
	}

	INIT_LIST_HEAD(&rfc->no_override_default_peak->params);
	ret = deserialize_policy(pptr, rfc->no_override_default_peak);
	if (ret) {
		return ret;
	}

	rfc->no_rate_slow_down = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rfc->no_rate_slow_down) {
		return -1;
	}

	INIT_LIST_HEAD(&rfc->no_rate_slow_down->params);
	ret = deserialize_policy(pptr, rfc->no_rate_slow_down);
	if (ret) {
		return ret;
	}

	rfc->rate_reduction = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rfc->rate_reduction) {
		return -1;
	}

	INIT_LIST_HEAD(&rfc->rate_reduction->params);
	return deserialize_policy(pptr, rfc->rate_reduction);
}

void rate_fctrl_config_free(struct rate_fctrl_config * rfc)
{
	if (!rfc)
		return;

	if (rfc->no_override_default_peak) {
		policy_free(rfc->no_override_default_peak);
		rfc->no_override_default_peak = 0;
	}

	if (rfc->no_rate_slow_down) {
		policy_free(rfc->no_rate_slow_down);
		rfc->no_rate_slow_down = 0;
	}

	if (rfc->rate_reduction) {
		policy_free(rfc->rate_reduction);
		rfc->rate_reduction = 0;
	}

	COMMON_FREE(rfc);
}

int dtcp_fctrl_config_serlen(const struct dtcp_fctrl_config * dfc)
{
	int ret;

	ret = 6 * sizeof(uint32_t) + 2 * sizeof(bool) +
		 + policy_serlen(dfc->closed_window)
                 + policy_serlen(dfc->receiving_flow_control)
		 + policy_serlen(dfc->reconcile_flow_conflict);

	if (dfc->window_based_fctrl)
		ret = ret + window_fctrl_config_serlen(dfc->wfctrl_cfg);

	if (dfc->rate_based_fctrl)
		ret = ret + rate_fctrl_config_serlen(dfc->rfctrl_cfg);

	return ret;
}

void serialize_dtcp_fctrl_config(void **pptr, const struct dtcp_fctrl_config *dfc)
{
	serialize_obj(*pptr, uint32_t, dfc->rcvd_buffers_th);
	serialize_obj(*pptr, uint32_t, dfc->rcvd_bytes_percent_th);
	serialize_obj(*pptr, uint32_t, dfc->rcvd_bytes_th);
	serialize_obj(*pptr, uint32_t, dfc->sent_buffers_th);
	serialize_obj(*pptr, uint32_t, dfc->sent_bytes_percent_th);
	serialize_obj(*pptr, uint32_t, dfc->sent_bytes_th);
	serialize_obj(*pptr, bool, dfc->window_based_fctrl);
	serialize_obj(*pptr, bool, dfc->rate_based_fctrl);
	serialize_policy(pptr, dfc->closed_window);
	serialize_policy(pptr, dfc->receiving_flow_control);
	serialize_policy(pptr, dfc->reconcile_flow_conflict);

	if (dfc->window_based_fctrl)
		serialize_window_fctrl_config(pptr, dfc->wfctrl_cfg);

	if (dfc->rate_based_fctrl)
		serialize_rate_fctrl_config(pptr, dfc->rfctrl_cfg);
}

int deserialize_dtcp_fctrl_config(const void **pptr, struct dtcp_fctrl_config *dfc)
{
	int ret;

	memset(dfc, 0, sizeof(*dfc));

	deserialize_obj(*pptr, uint32_t, dfc->rcvd_buffers_th);
	deserialize_obj(*pptr, uint32_t, dfc->rcvd_bytes_percent_th);
	deserialize_obj(*pptr, uint32_t, dfc->rcvd_bytes_th);
	deserialize_obj(*pptr, uint32_t, dfc->sent_buffers_th);
	deserialize_obj(*pptr, uint32_t, dfc->sent_bytes_percent_th);
	deserialize_obj(*pptr, uint32_t, dfc->sent_bytes_th);
	deserialize_obj(*pptr, bool, dfc->window_based_fctrl);
	deserialize_obj(*pptr, bool, dfc->rate_based_fctrl);

	dfc->closed_window = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dfc->closed_window) {
		return -1;
	}

	INIT_LIST_HEAD(&dfc->closed_window->params);
	ret = deserialize_policy(pptr, dfc->closed_window);
	if (ret) {
		return ret;
	}

	dfc->receiving_flow_control = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dfc->receiving_flow_control) {
		return -1;
	}

	INIT_LIST_HEAD(&dfc->receiving_flow_control->params);
	ret = deserialize_policy(pptr, dfc->receiving_flow_control);
	if (ret) {
		return ret;
	}

	dfc->reconcile_flow_conflict = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dfc->reconcile_flow_conflict) {
		return -1;
	}

	INIT_LIST_HEAD(&dfc->reconcile_flow_conflict->params);
	ret = deserialize_policy(pptr, dfc->reconcile_flow_conflict);
	if (ret) {
		return ret;
	}

	if (dfc->window_based_fctrl) {
		dfc->wfctrl_cfg = COMMON_ALLOC(sizeof(struct window_fctrl_config), 1);
		if (!dfc->wfctrl_cfg) {
			return -1;
		}

		ret = deserialize_window_fctrl_config(pptr, dfc->wfctrl_cfg);
		if (ret) {
			return ret;
		}
	}

	if (dfc->rate_based_fctrl) {
		dfc->rfctrl_cfg = COMMON_ALLOC(sizeof(struct rate_fctrl_config), 1);
		if (!dfc->rfctrl_cfg) {
			return -1;
		}

		ret = deserialize_rate_fctrl_config(pptr, dfc->rfctrl_cfg);
		if (ret) {
			return ret;
		}
	}

	return ret;
}

void dtcp_fctrl_config_free(struct dtcp_fctrl_config * dfc)
{
	if (!dfc)
		return;

	if (dfc->closed_window) {
		policy_free(dfc->closed_window);
		dfc->closed_window = 0;
	}

	if (dfc->receiving_flow_control) {
		policy_free(dfc->receiving_flow_control);
		dfc->receiving_flow_control = 0;
	}

	if (dfc->reconcile_flow_conflict) {
		policy_free(dfc->reconcile_flow_conflict);
		dfc->reconcile_flow_conflict = 0;
	}

	if (dfc->wfctrl_cfg) {
		window_fctrl_config_free(dfc->wfctrl_cfg);
		dfc->wfctrl_cfg = 0;
	}

	if (dfc->rfctrl_cfg) {
		rate_fctrl_config_free(dfc->rfctrl_cfg);
		dfc->rfctrl_cfg = 0;
	}

	COMMON_FREE(dfc);
}

int dtcp_rxctrl_config_serlen(const struct dtcp_rxctrl_config * rxfc)
{
	return 3 * sizeof(uint32_t)
		 + policy_serlen(rxfc->rcvr_ack)
                 + policy_serlen(rxfc->rcvr_control_ack)
		 + policy_serlen(rxfc->receiving_ack_list)
		 + policy_serlen(rxfc->retransmission_timer_expiry)
		 + policy_serlen(rxfc->sender_ack)
		 + policy_serlen(rxfc->sending_ack);
}

void serialize_dtcp_rxctrl_config(void **pptr, const struct dtcp_rxctrl_config *rxfc)
{
	serialize_obj(*pptr, uint32_t, rxfc->data_retransmit_max);
	serialize_obj(*pptr, uint32_t, rxfc->initial_tr);
	serialize_obj(*pptr, uint32_t, rxfc->max_time_retry);
	serialize_policy(pptr, rxfc->rcvr_ack);
	serialize_policy(pptr, rxfc->rcvr_control_ack);
	serialize_policy(pptr, rxfc->receiving_ack_list);
	serialize_policy(pptr, rxfc->retransmission_timer_expiry);
	serialize_policy(pptr, rxfc->sender_ack);
	serialize_policy(pptr, rxfc->sending_ack);
}

int deserialize_dtcp_rxctrl_config(const void **pptr, struct dtcp_rxctrl_config *rxfc)
{
	int ret;

	memset(rxfc, 0, sizeof(*rxfc));

	deserialize_obj(*pptr, uint32_t, rxfc->data_retransmit_max);
	deserialize_obj(*pptr, uint32_t, rxfc->initial_tr);
	deserialize_obj(*pptr, uint32_t, rxfc->max_time_retry);

	rxfc->rcvr_ack = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->rcvr_ack) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->rcvr_ack->params);
	ret = deserialize_policy(pptr, rxfc->rcvr_ack);
	if (ret) {
		return ret;
	}

	rxfc->rcvr_control_ack = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->rcvr_control_ack) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->rcvr_control_ack->params);
	ret = deserialize_policy(pptr, rxfc->rcvr_control_ack);
	if (ret) {
		return ret;
	}

	rxfc->receiving_ack_list = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->receiving_ack_list) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->receiving_ack_list->params);
	ret = deserialize_policy(pptr, rxfc->receiving_ack_list);
	if (ret) {
		return ret;
	}

	rxfc->retransmission_timer_expiry = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->retransmission_timer_expiry) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->retransmission_timer_expiry->params);
	ret = deserialize_policy(pptr, rxfc->retransmission_timer_expiry);
	if (ret) {
		return ret;
	}

	rxfc->sender_ack = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->sender_ack) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->sender_ack->params);
	ret = deserialize_policy(pptr, rxfc->sender_ack);
	if (ret) {
		return ret;
	}

	rxfc->sending_ack = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!rxfc->sending_ack) {
		return -1;
	}

	INIT_LIST_HEAD(&rxfc->sending_ack->params);
	return deserialize_policy(pptr, rxfc->sending_ack);
}

void dtcp_rxctrl_config_free(struct dtcp_rxctrl_config * rxfc)
{
	if (!rxfc)
		return;

	if (rxfc->rcvr_ack) {
		policy_free(rxfc->rcvr_ack);
		rxfc->rcvr_ack = 0;
	}

	if (rxfc->rcvr_control_ack) {
		policy_free(rxfc->rcvr_control_ack);
		rxfc->rcvr_control_ack = 0;
	}

	if (rxfc->receiving_ack_list) {
		policy_free(rxfc->receiving_ack_list);
		rxfc->receiving_ack_list = 0;
	}

	if (rxfc->retransmission_timer_expiry) {
		policy_free(rxfc->retransmission_timer_expiry);
		rxfc->retransmission_timer_expiry = 0;
	}

	if (rxfc->sender_ack) {
		policy_free(rxfc->sender_ack);
		rxfc->sender_ack = 0;
	}

	if (rxfc->sending_ack) {
		policy_free(rxfc->sending_ack);
		rxfc->sending_ack = 0;
	}

	COMMON_FREE(rxfc);
}

int dtcp_config_serlen(const struct dtcp_config * dtcp_config)
{
	int ret;

	ret = 2 * sizeof(bool)
		+ policy_serlen(dtcp_config->dtcp_ps)
		+ policy_serlen(dtcp_config->lost_control_pdu)
		+ policy_serlen(dtcp_config->rtt_estimator);

	if (dtcp_config->flow_ctrl)
		ret = ret + dtcp_fctrl_config_serlen(dtcp_config->fctrl_cfg);

	if (dtcp_config->rtx_ctrl)
		ret = ret + dtcp_rxctrl_config_serlen(dtcp_config->rxctrl_cfg);
}

void serialize_dctp_config(void **pptr, const struct dtcp_config *dtcp_config)
{
	serialize_obj(*pptr, bool, dtcp_config->flow_ctrl);
	serialize_obj(*pptr, bool, dtcp_config->rtx_ctrl);
	serialize_policy(pptr, dtcp_config->dtcp_ps);
	serialize_policy(pptr, dtcp_config->lost_control_pdu);
	serialize_policy(pptr, dtcp_config->rtt_estimator);

	if (dtcp_config->flow_ctrl)
		serialize_dtcp_fctrl_config(dtcp_config->fctrl_cfg);

	if (dtcp_config->rtx_ctrl)
		serialize_dtcp_rxctrl_config(dtcp_config->rxctrl_cfg);
}

int deserialize_dtcp_config(const void **pptr, struct dtcp_config *dtcp_config)
{
	int ret;

	memset(dtcp_config, 0, sizeof(*dtcp_config));

	deserialize_obj(*pptr, bool, dtcp_config->flow_ctrl);
	deserialize_obj(*pptr, bool, dtcp_config->rtx_ctrl);

	dtcp_config->dtcp_ps = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dtcp_config->dtcp_ps) {
		return -1;
	}

	INIT_LIST_HEAD(&dtcp_config->dtcp_ps->params);
	ret = deserialize_policy(pptr, dtcp_config->dtcp_ps);
	if (ret) {
		return ret;
	}

	dtcp_config->lost_control_pdu = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dtcp_config->lost_control_pdu) {
		return -1;
	}

	INIT_LIST_HEAD(&dtcp_config->lost_control_pdu->params);
	ret = deserialize_policy(pptr, dtcp_config->lost_control_pdu);
	if (ret) {
		return ret;
	}

	dtcp_config->rtt_estimator = COMMON_ALLOC(sizeof(struct policy), 1);
	if (!dtcp_config->rtt_estimator) {
		return -1;
	}

	INIT_LIST_HEAD(&dtcp_config->rtt_estimator->params);
	ret = deserialize_policy(pptr, dtcp_config->rtt_estimator);
	if (ret) {
		return ret;
	}

	if (dtcp_config->flow_ctrl) {
		dtcp_config->fctrl_cfg = COMMON_ALLOC(sizeof(struct dtcp_fctrl_config), 1);
		if (!dtcp_config->fctrl_cfg)
			return -1;

		ret = deserialize_dtcp_fctrl_config(pptr, dtcp_config->fctrl_cfg);
		if (ret)
			return ret;
	}

	if (dtcp_config->rtx_ctrl) {
		dtcp_config->rxctrl_cfg = COMMON_ALLOC(sizeof(struct dtcp_rxctrl_config), 1);
		if (!dtcp_config->rxctrl_cfg)
			return -1;

		ret = deserialize_dtcp_rxctrl_config(pptr, dtcp_config->rxctrl_cfg);
		if (ret)
			return ret;
	}

	return ret;
}

void dtcp_config_free(struct dtcp_config * dtcp_config)
{
	if (!dtcp_config)
		return;

	if (dtcp_config->dtcp_ps) {
		policy_free(dtcp_config->dtcp_ps);
		dtcp_config->dtcp_ps = 0;
	}

	if (dtcp_config->lost_control_pdu) {
		policy_free(dtcp_config->lost_control_pdu);
		dtcp_config->lost_control_pdu = 0;
	}

	if (dtcp_config->rtt_estimator) {
		policy_free(dtcp_config->rtt_estimator);
		dtcp_config->rtt_estimator = 0;
	}

	if (dtcp_config->fctrl_cfg) {
		dtcp_fctrl_config_free(dtcp_config->fctrl_cfg);
		dtcp_config->fctrl_cfg = 0;
	}

	if (dtcp_config->rxctrl_cfg) {
		dtcp_rxctrl_config_free(dtcp_config->rxctrl_cfg);
		dtcp_config->rxctrl_cfg = 0;
	}

	COMMON_FREE(dtcp_config);
}

unsigned int serialize_irati_msg(struct irati_msg_layout *numtables,
				 size_t num_entries,
				 void *serbuf,
				 const struct irati_msg_base *msg)
{
	void *serptr = serbuf;
	unsigned int serlen;
	unsigned int copylen;
	struct rina_name *name;
	string_t *str;
	const struct buffer *bf;
	struct flow_spec *fspec;
	struct dif_config *dif_config;
	struct dtp_config *dtp_config;
	struct dtcp_config *dtcp_config;
	int i;

	if (msg->msg_type >= num_entries) {
		LOG_ERR("Invalid numtables access [msg_type=%u]\n", msg->msg_type);
		return -1;
	}

	copylen = numtables[msg->msg_type].copylen;
	memcpy(serbuf, msg, copylen);

	serptr = serbuf + copylen;
	name = (struct name *)(((void *)msg) + copylen);
	for (i = 0; i < numtables[msg->msg_type].names; i++, name++) {
		serialize_rina_name(&serptr, name);
	}

	str = (string_t *)(name);
	for (i = 0; i < numtables[msg->msg_type].strings; i++, str++) {
		serialize_string(&serptr, *str);
	}

	fspec = (struct fspec *)str;
	for (i = 0; i < numtables[msg->msg_type].flow_specs; i++, fspec++) {
		serialize_flow_spec(&serptr, fspec);
	}

	dif_config = (struct dif_config *)fspec;
	for (i = 0; i < numtables[msg->msg_type].dif_configs; i++, dif_config++) {
		serialize_dif_config(&serptr, dif_config);
	}

	dtp_config = (struct dtp_config*)dif_config;
	for (i = 0; i < numtables[msg->msg_type].dtp_configs; i++, dtp_config++) {
		serialize_dtp_config(&serptr, dtp_config);
	}

	dtcp_config = (struct dtcp_config*)dtp_config;
	for (i = 0; i < numtables[msg->msg_type].dtcp_configs; i++, dtcp_config++) {
		serialize_dtcp_config(&serptr, dtcp_config);
	}

	bf = (const struct buffer *)dtcp_config;
	for (i = 0; i < numtables[msg->msg_type].buffers; i++, bf++) {
		serialize_buffer(&serptr, bf);
	}

	serlen = serptr - serbuf;

	return serlen;
}
COMMON_EXPORT(serialize_irati_msg);

int deserialize_irati_msg(struct irati_msg_layout *numtables, size_t num_entries,
                          const void *serbuf, unsigned int serbuf_len,
                          void *msgbuf, unsigned int msgbuf_len)
{
	struct irati_msg_base *bmsg = IRATI_MB(serbuf);
	struct name *name;
	string_t *str;
	struct buffer *bf;
	struct flow_spec *fspec;
	struct dif_config *dif_config;
	struct dtp_config *dtp_config;
	struct dtcp_config *dtcp_config;
	unsigned int copylen;
	const void *desptr;
	int ret;
	int i;

	if (bmsg->msg_type >= num_entries) {
		LOG_ERR("Invalid numtables access [msg_type=%u]\n",
			bmsg->msg_type);
		return -1;
	}

	copylen = numtables[bmsg->msg_type].copylen;
	memcpy(msgbuf, serbuf, copylen);

	desptr = serbuf + copylen;
	name = (struct name *)(msgbuf + copylen);
	for (i = 0; i < numtables[bmsg->msg_type].names; i++, name++) {
		ret = deserialize_rina_name(&desptr, name);
		if (ret) {
			return ret;
		}
	}

	str = (string_t *)name;
	for (i = 0; i < numtables[bmsg->msg_type].strings; i++, str++) {
		ret = deserialize_string(&desptr, str);
		if (ret) {
			return ret;
		}
	}

	fspec = (struct flow_spec *)str;
	for (i = 0; i < numtables[bmsg->msg_type].flow_specs; i++, fspec++) {
		ret = deserialize_flow_spec(&desptr, fspec);
	}

	dif_config = (struct dif_config *)fspec;
	for (i = 0; i < numtables[bmsg->msg_type].dif_configs; i++, dif_config++) {
		ret = deserialize_dif_config(&desptr, dif_config);
	}

	dtp_config = (struct dtp_config *)dif_config;
	for (i = 0; i < numtables[bmsg->msg_type].dtp_configs; i++, dtp_config++) {
		ret = deserialize_dtp_config(&desptr, dtp_config);
	}

	dtcp_config = (struct dtcp_config *)dtp_config;
	for (i = 0; i < numtables[bmsg->msg_type].dtcp_configs; i++, dtcp_config++) {
		ret = deserialize_dtcp_config(&desptr, dtcp_config);
	}

	bf = (struct buffer *)dtcp_config;
	for (i = 0; i < numtables[bmsg->msg_type].buffers; i++, bf++) {
		ret = deserialize_buffer(&desptr, bf);
	}

	if ((desptr - serbuf) != serbuf_len) {
		return -1;
	}

	return 0;
}
COMMON_EXPORT(deserialize_irati_msg);

unsigned int irati_msg_serlen(struct irati_msg_layout *numtables,
			      size_t num_entries,
			      const struct irati_msg_base *msg)
{
	unsigned int ret;
	struct name *name;
	string_t *str;
	struct flow_spec *fspec;
	struct dif_config *dif_config;
	struct dtp_config *dtp_config;
	struct dtcp_config *dtcp_config;
	const struct buffer *bf;
	int i;

	if (msg->msg_type >= num_entries) {
		LOG_ERR("Invalid numtables access [msg_type=%u]\n", msg->msg_type);
		return -1;
	}

	ret = numtables[msg->msg_type].copylen;

	name = (struct name *)(((void *)msg) + ret);
	for (i = 0; i < numtables[msg->msg_type].names; i++, name++) {
		ret += rina_name_serlen(name);
	}

	str = (string_t *)name;
	for (i = 0; i < numtables[msg->msg_type].strings; i++, str++) {
		ret += sizeof(uint16_t) + string_prlen(*str);
	}

	fspec = (struct flow_spec *)str;
	for (i = 0; i < numtables[msg->msg_type].flow_specs; i++, fspec++) {
		ret += flow_spec_serlen(fspec);
	}

	dif_config = (struct dif_config *)fspec;
	for (i = 0; i < numtables[msg->msg_type].dif_configs; i++, dif_config++) {
		ret += dif_config_serlen(dif_config);
	}

	dtp_config = (struct dtp_config *)dif_config;
	for (i = 0; i < numtables[msg->msg_type].dtp_configs; i++, dtp_config++) {
		ret += dtp_config_serlen(dtp_config);
	}

	dtcp_config = (struct dtcp_config *)dtp_config;
	for (i = 0; i < numtables[msg->msg_type].dtcp_configs; i++, dtcp_config++) {
		ret += dtcp_config_serlen(dtcp_config);
	}

	bf = (const struct buffer *)dtcp_config;
	for (i = 0; i < numtables[msg->msg_type].buffers; i++, bf++) {
		ret += sizeof(bf->size) + bf->size;
	}

	return ret;
}
COMMON_EXPORT(irati_msg_serlen);

void irati_msg_free(struct irati_msg_layout *numtables, size_t num_entries,
                    struct irati_msg_base *msg)
{
	unsigned int copylen = numtables[msg->msg_type].copylen;
	struct name *name;
	string_t *str;
	struct flow_spec *fspec;
	struct dif_config * dif_config;
	struct dtp_config * dtp_config;
	struct dtcp_config * dtcp_config;
	int i;

	if (msg->msg_type >= num_entries) {
		LOG_ERR("Invalid numtables access [msg_type=%u]\n",
			msg->msg_type);
		return;
	}

	/* Skip the copiable part and scan all the names contained in
	 * the message. */
	name = (struct name *)(((void *)msg) + copylen);
	for (i = 0; i < numtables[msg->msg_type].names; i++, name++) {
		rina_name_free(name);
	}

	str = (string_t *)(name);
	for (i = 0; i < numtables[msg->msg_type].strings; i++, str++) {
		if (*str) {
			COMMON_FREE(*str);
		}
	}

	fspec = (struct flow_spec *)(str);
	for (i = 0; i < numtables[msg->msg_type].flow_specs; i++, fspec++) {
		flow_spec_free(fspec);
	}

	/* TODO check for dif_config */

	dtp_config = (struct dtp_config *)(dif_config);
	for (i = 0; i < numtables[msg->msg_type].dtp_configs; i++, dtp_config++) {
		dtp_config_free(dtp_config);
	}

	dtcp_config = (struct dtcp_config *)(dtp_config);
	for (i = 0; i < numtables[msg->msg_type].dtcp_configs; i++, dtcp_config++) {
		dtcp_config_free(dtcp_config);
	}
}
COMMON_EXPORT(irati_msg_free);

unsigned int irati_numtables_max_size(struct irati_msg_layout *numtables,
				      unsigned int n)
{
	unsigned int max = 0;
	int i = 0;

	for (i = 0; i < n; i++) {
		unsigned int cur = numtables[i].copylen +
				numtables[i].names * sizeof(struct name) +
				numtables[i].strings * sizeof(char *) +
				numtables[i].flow_specs * sizeof(struct flow_spec) +
				numtables[i].dif_configs * sizeof(struct dif_config) +
				numtables[i].dtp_configs * sizeof(struct dtp_config) +
				numtables[i].dtcp_configs * sizeof(struct dtcp_config);

		if (cur > max) {
			max = cur;
		}
	}

	return max;
}
