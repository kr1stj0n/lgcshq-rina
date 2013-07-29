/*
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

/*
 * librina-netlink-parsers.h
 *
 *  Created on: 14/06/2013
 *      Author: eduardgrasa
 */

#ifndef LIBRINA_NETLINK_PARSERS_H_
#define LIBRINA_NETLINK_PARSERS_H_

#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netlink/genl/genl.h>

#include "netlink-messages.h"

namespace rina {

int putBaseNetlinkMessage(nl_msg* netlinkMessage, BaseNetlinkMessage * message);

BaseNetlinkMessage * parseBaseNetlinkMessage(nlmsghdr* netlinkMesasgeHeader);

/* APPLICATION PROCESS NAMING INFORMATION CLASS */
enum ApplicationProcessNamingInformationAttributes {
	APNI_ATTR_PROCESS_NAME = 1,
	APNI_ATTR_PROCESS_INSTANCE,
	APNI_ATTR_ENTITY_NAME,
	APNI_ATTR_ENTITY_INSTANCE,
	__APNI_ATTR_MAX,
};

#define APNI_ATTR_MAX (__APNI_ATTR_MAX -1)

int putApplicationProcessNamingInformationObject(nl_msg* netlinkMessage,
		const ApplicationProcessNamingInformation& object);

ApplicationProcessNamingInformation *
parseApplicationProcessNamingInformationObject(nlattr *nested);

/* AppAllocateFlowRequestMessage CLASS*/
enum AppAllocateFlowRequestAttributes {
	AAFR_ATTR_SOURCE_APP_NAME = 1,
	AAFR_ATTR_DEST_APP_NAME,
	AAFR_ATTR_FLOW_SPEC,
	__AAFR_ATTR_MAX,
};

#define AAFR_ATTR_MAX (__AAFR_ATTR_MAX -1)

int putAppAllocateFlowRequestMessageObject(nl_msg* netlinkMessage,
		const AppAllocateFlowRequestMessage& object);

AppAllocateFlowRequestMessage * parseAppAllocateFlowRequestMessage(
		nlmsghdr *hdr);

/* FLOW SPECIFICATION CLASS */
enum FlowSpecificationAttributes {
	FSPEC_ATTR_AVG_BWITH = 1,
	FSPEC_ATTR_AVG_SDU_BWITH,
	FSPEC_ATTR_DELAY,
	FSPEC_ATTR_JITTER,
	FSPEC_ATTR_MAX_GAP,
	FSPEC_ATTR_MAX_SDU_SIZE,
	FSPEC_ATTR_IN_ORD_DELIVERY,
	FSPEC_ATTR_PART_DELIVERY,
	FSPEC_ATTR_PEAK_BWITH_DURATION,
	FSPEC_ATTR_PEAK_SDU_BWITH_DURATION,
	FSPEC_ATTR_UNDETECTED_BER,
	__FSPEC_ATTR_MAX,
};

#define FSPEC_ATTR_MAX (__FSPEC_ATTR_MAX -1)

int putFlowSpecificationObject(nl_msg* netlinkMessage,
		const FlowSpecification& object);

FlowSpecification * parseFlowSpecificationObject(nlattr *nested);

/* AppAllocateFlowRequestResultMessage CLASS*/
enum AppAllocateFlowRequestResultAttributes {
	AAFRR_ATTR_SOURCE_APP_NAME = 1,
	AAFRR_ATTR_PORT_ID,
	AAFRR_ATTR_ERROR_DESCRIPTION,
	AAFRR_ATTR_DIF_NAME,
	AAFRR_ATTR_IPC_PROCESS_PORT_ID,
	AAFRR_ATTR_IPC_PROCESS_ID,
	__AAFRR_ATTR_MAX,
};

#define AAFRR_ATTR_MAX (__AAFRR_ATTR_MAX -1)

int putAppAllocateFlowRequestResultMessageObject(nl_msg* netlinkMessage,
		const AppAllocateFlowRequestResultMessage& object);

AppAllocateFlowRequestResultMessage * parseAppAllocateFlowRequestResultMessage(
		nlmsghdr *hdr);

/* AppAllocateFlowRequestMessage CLASS*/
enum AppAllocateFlowRequestArrivedAttributes {
	AAFRA_ATTR_SOURCE_APP_NAME = 1,
	AAFRA_ATTR_DEST_APP_NAME,
	AAFRA_ATTR_FLOW_SPEC,
	AAFRA_ATTR_PORT_ID,
	AAFRA_ATTR_DIF_NAME,
	__AAFRA_ATTR_MAX,
};

#define AAFRA_ATTR_MAX (__AAFRA_ATTR_MAX -1)

int putAppAllocateFlowRequestArrivedMessageObject(nl_msg* netlinkMessage,
		const AppAllocateFlowRequestArrivedMessage& object);

AppAllocateFlowRequestArrivedMessage * parseAppAllocateFlowRequestArrivedMessage(
		nlmsghdr *hdr);

/* AppAllocateFlowResponseMessage CLASS*/
enum AppAllocateFlowResponseAttributes {
	AAFRE_ATTR_DIF_NAME = 1,
	AAFRE_ATTR_ACCEPT,
	AAFRE_ATTR_DENY_REASON,
	AAFRE_ATTR_NOTIFY_SOURCE,
	__AAFRE_ATTR_MAX,
};

#define AAFRE_ATTR_MAX (__AAFRE_ATTR_MAX -1)

int putAppAllocateFlowResponseMessageObject(nl_msg* netlinkMessage,
		const AppAllocateFlowResponseMessage& object);

AppAllocateFlowResponseMessage * parseAppAllocateFlowResponseMessage(
		nlmsghdr *hdr);

/* AppDeallocateFlowRequestMessage CLASS*/
enum AppDeallocateFlowRequestMessageAttributes {
	ADFRT_ATTR_PORT_ID = 1,
	ADFRT_ATTR_DIF_NAME,
	ADFRT_ATTR_APP_NAME,
	__ADFRT_ATTR_MAX,
};

#define ADFRT_ATTR_MAX (__ADFRT_ATTR_MAX -1)

int putAppDeallocateFlowRequestMessageObject(nl_msg* netlinkMessage,
		const AppDeallocateFlowRequestMessage& object);

AppDeallocateFlowRequestMessage * parseAppDeallocateFlowRequestMessage(
		nlmsghdr *hdr);

/* AppDeallocateFlowResponseMessage CLASS*/
enum AppDeallocateFlowResponseMessageAttributes {
	ADFRE_ATTR_RESULT = 1,
	ADFRE_ATTR_ERROR_DESCRIPTION,
	ADFRE_ATTR_APP_NAME,
	__ADFRE_ATTR_MAX,
};

#define ADFRE_ATTR_MAX (__ADFRE_ATTR_MAX -1)

int putAppDeallocateFlowResponseMessageObject(nl_msg* netlinkMessage,
		const AppDeallocateFlowResponseMessage& object);

AppDeallocateFlowResponseMessage * parseAppDeallocateFlowResponseMessage(
		nlmsghdr *hdr);

/* AppFlowDeallocatedNotificationMessage CLASS*/
enum AppFlowDeallocatedNotificationMessageAttributes {
	AFDN_ATTR_PORT_ID = 1,
	AFDN_ATTR_CODE,
	AFDN_ATTR_REASON,
	AFDN_ATTR_APP_NAME,
	AFDN_ATTR_DIF_NAME,
	__AFDN_ATTR_MAX,
};

#define AFDN_ATTR_MAX (__AFDN_ATTR_MAX -1)

int putAppFlowDeallocatedNotificationMessageObject(nl_msg* netlinkMessage,
		const AppFlowDeallocatedNotificationMessage& object);

AppFlowDeallocatedNotificationMessage * parseAppFlowDeallocatedNotificationMessage(
		nlmsghdr *hdr);

/* ApplicationRegistrationInformation CLASS*/
enum ApplicationRegistrationInformationAttributes {
	ARIA_ATTR_APP_REG_TYPE = 1,
	ARIA_ATTR_APP_DIF_NAME,
	__ARIA_ATTR_MAX,
};

#define ARIA_ATTR_MAX (__ARIA_ATTR_MAX -1)

int putApplicationRegistrationInformationObject(nl_msg* netlinkMessage,
		const ApplicationRegistrationInformation& object);

ApplicationRegistrationInformation * parseApplicationRegistrationInformation(
		nlattr *nested);

/* AppRegisterApplicationRequestMessage CLASS*/
enum AppRegisterApplicationRequestMessageAttributes {
	ARAR_ATTR_APP_NAME = 1,
	ARAR_ATTR_APP_REG_INFO,
	__ARAR_ATTR_MAX,
};

#define ARAR_ATTR_MAX (__ARAR_ATTR_MAX -1)

int putAppRegisterApplicationRequestMessageObject(nl_msg* netlinkMessage,
		const AppRegisterApplicationRequestMessage& object);

AppRegisterApplicationRequestMessage * parseAppRegisterApplicationRequestMessage(
		nlmsghdr *hdr);

/* AppRegisterApplicationResponseMessage CLASS*/
enum AppRegisterApplicationResponseMessageAttributes {
	ARARE_ATTR_APP_NAME = 1,
	ARARE_ATTR_RESULT,
	ARARE_ATTR_ERROR_DESCRIPTION,
	ARARE_ATTR_DIF_NAME,
	ARARE_ATTR_PROCESS_PORT_ID,
	ARARE_ATTR_PROCESS_IPC_PROCESS_ID,
	__ARARE_ATTR_MAX,
};

#define ARARE_ATTR_MAX (__ARARE_ATTR_MAX -1)

int putAppRegisterApplicationResponseMessageObject(nl_msg* netlinkMessage,
		const AppRegisterApplicationResponseMessage& object);

AppRegisterApplicationResponseMessage *
	parseAppRegisterApplicationResponseMessage(nlmsghdr *hdr);


/* AppUnregisterApplicationRequestMessage CLASS*/
enum AppUnregisterApplicationRequestMessageAttributes {
	AUAR_ATTR_APP_NAME = 1,
	AUAR_ATTR_DIF_NAME,
	__AUAR_ATTR_MAX,
};

#define AUAR_ATTR_MAX (__AUAR_ATTR_MAX -1)

int putAppUnregisterApplicationRequestMessageObject(nl_msg* netlinkMessage,
		const AppUnregisterApplicationRequestMessage& object);

AppUnregisterApplicationRequestMessage * parseAppUnregisterApplicationRequestMessage(
		nlmsghdr *hdr);


/* AppUnregisterApplicationResponseMessage CLASS*/
enum AppUnregisterApplicationResponseMessageAttributes {
	AUARE_ATTR_RESULT = 1,
	AUARE_ATTR_ERROR_DESCRIPTION,
	AUARE_ATTR_APP_NAME,
	__AUARE_ATTR_MAX,
};

#define AUARE_ATTR_MAX (__AUARE_ATTR_MAX -1)

int putAppUnregisterApplicationResponseMessageObject(nl_msg* netlinkMessage,
		const AppUnregisterApplicationResponseMessage& object);

AppUnregisterApplicationResponseMessage * parseAppUnregisterApplicationResponseMessage(
		nlmsghdr *hdr);


/* AppRegistrationCanceledNotificationMessage CLASS*/
enum AppRegistrationCanceledNotificationMessageAttributes {
	ARCN_ATTR_CODE = 1,
	ARCN_ATTR_REASON,
	ARCN_ATTR_APP_NAME,
	ARCN_ATTR_DIF_NAME,
	__ARCN_ATTR_MAX,
};

#define ARCN_ATTR_MAX (__ARCN_ATTR_MAX -1)

int putAppRegistrationCanceledNotificationMessageObject(nl_msg* netlinkMessage,
		const AppRegistrationCanceledNotificationMessage& object);

AppRegistrationCanceledNotificationMessage * parseAppRegistrationCanceledNotificationMessage(
		nlmsghdr *hdr);

/* AppGetDIFPropertiesRequestMessage CLASS*/
enum AppGetDIFPropertiesRequestMessageAttributes {
	AGDP_ATTR_APP_NAME = 1,
	AGDP_ATTR_DIF_NAME,
	__AGDP_ATTR_MAX,
};

#define AGDP_ATTR_MAX (__AGDP_ATTR_MAX -1)

int putAppGetDIFPropertiesRequestMessageObject(nl_msg* netlinkMessage,
		const AppGetDIFPropertiesRequestMessage& object);

AppGetDIFPropertiesRequestMessage * parseAppGetDIFPropertiesRequestMessage(
		nlmsghdr *hdr);

/* QOSCUBE CLASS*/
enum QoSCubesAttributes {
	QOS_CUBE_ATTR_NAME = 1,
	QOS_CUBE_ATTR_ID,
	QOS_CUBE_ATTR_AVG_BAND,
	QOS_CUBE_ATTR_AVG_SDU_BAND,
	QOS_CUBE_ATTR_PEAK_BAND_DUR,
	QOS_CUBE_ATTR_PEAK_SDU_BAND_DUR,
	QOS_CUBE_ATTR_UND_BER,
	QOS_CUBE_ATTR_PART_DEL,
	QOS_CUBE_ATTR_ORD_DEL,
	QOS_CUBE_ATTR_MAX_GAP,
	QOS_CUBE_ATTR_DELAY,
	QOS_CUBE_ATTR_JITTER,
	__QOS_CUBE_ATTR_MAX,
};

#define QOS_CUBE_ATTR_MAX (__QOS_CUBE_ATTR_MAX -1)

int putQoSCubeObject(nl_msg* netlinkMessage,
		const QoSCube& object);

QoSCube * parseQoSCubeObject(nlmsghdr *hdr);

/* DIFPROPERTIES CLASS*/
enum DIFPropertiesAttributes {
	DIF_PROP_ATTR_DIF_NAME = 1,
	DIF_PROP_ATTR_MAX_SDU_SIZE,
	DIF_PROP_ATTR_QOS_CUBES,
	__DIF_PROP_ATTR_MAX,
};

#define DIF_PROP_ATTR_MAX (__DIF_PROP_ATTR_MAX -1)

int putDIFPropertiesObject(nl_msg* netlinkMessage,
		const DIFProperties& object);

DIFProperties * parseDIFPropertiesObject(nlmsghdr *hdr);

/* AppGetDIFPropertiesResponseMessage CLASS*/
enum AppGetDIFPropertiesResponseMessageAttributes {
	AGDPR_ATTR_RESULT = 1,
	AGDPR_ATTR_ERROR_DESC,
	AGDPR_ATTR_APP_NAME,
	AGDPR_ATTR_DIF_PROPERTIES,
	__AGDPR_ATTR_MAX,
};

#define AGDPR_ATTR_MAX (__AGDPR_ATTR_MAX -1)

int putAppGetDIFPropertiesResponseMessageObject(nl_msg* netlinkMessage,
		const AppGetDIFPropertiesResponseMessage& object);

AppGetDIFPropertiesResponseMessage * parseAppGetDIFPropertiesResponseMessage(
		nlmsghdr *hdr);

/* IpcmRegisterApplicationRequestMessage CLASS*/
enum IpcmRegisterApplicationRequestMessageAttributes {
	IRAR_ATTR_APP_NAME = 1,
	IRAR_ATTR_DIF_NAME,
	IRAR_ATTR_APP_PORT_ID,
	__IRAR_ATTR_MAX,
};

#define IRAR_ATTR_MAX (__IRAR_ATTR_MAX -1)

int putIpcmRegisterApplicationRequestMessageObject(nl_msg* netlinkMessage,
		const IpcmRegisterApplicationRequestMessage& object);

IpcmRegisterApplicationRequestMessage *
	parseIpcmRegisterApplicationRequestMessage(nlmsghdr *hdr);

/* IpcmRegisterApplicationResponseMessage CLASS*/
enum IpcmRegisterApplicationResponseMessageAttributes {
	IRARE_ATTR_APP_NAME = 1,
	IRARE_ATTR_RESULT,
	IRARE_ATTR_ERROR_DESCRIPTION,
	IRARE_ATTR_DIF_NAME,
	__IRARE_ATTR_MAX,
};

#define IRARE_ATTR_MAX (__IRARE_ATTR_MAX -1)

int putIpcmRegisterApplicationResponseMessageObject(nl_msg* netlinkMessage,
		const IpcmRegisterApplicationResponseMessage& object);

IpcmRegisterApplicationResponseMessage *
	parseIpcmRegisterApplicationResponseMessage(nlmsghdr *hdr);

/* IpcmUnregisterApplicationRequestMessage CLASS*/
enum IpcmUnregisterApplicationRequestMessageAttributes {
	IUAR_ATTR_APP_NAME = 1,
	IUAR_ATTR_DIF_NAME,
	__IUAR_ATTR_MAX,
};

#define IUAR_ATTR_MAX (__IUAR_ATTR_MAX -1)

int putIpcmUnregisterApplicationRequestMessageObject(nl_msg* netlinkMessage,
		const IpcmUnregisterApplicationRequestMessage& object);

IpcmUnregisterApplicationRequestMessage *
	parseIpcmUnregisterApplicationRequestMessage(nlmsghdr *hdr);

/* IpcmUnregisterApplicationResponseMessage CLASS*/
enum IpcmUnregisterApplicationResponseMessageAttributes {
	IUARE_ATTR_RESULT = 1,
	IUARE_ATTR_ERROR_DESCRIPTION,
	__IUARE_ATTR_MAX,
};

#define IUARE_ATTR_MAX (__IUARE_ATTR_MAX -1)

int putIpcmUnregisterApplicationResponseMessageObject(nl_msg* netlinkMessage,
		const IpcmUnregisterApplicationResponseMessage& object);

IpcmUnregisterApplicationResponseMessage *
	parseIpcmUnregisterApplicationResponseMessage(nlmsghdr *hdr);

/* DIF Configuration CLASS */
enum DIFConfigurationAttributes {
	DCONF_ATTR_DIF_TYPE = 1,
	DCONF_ATTR_DIF_NAME,
	__DCONF_ATTR_MAX,
};

#define DCONF_ATTR_MAX (__DCONF_ATTR_MAX -1)

int putDIFConfigurationObject(nl_msg* netlinkMessage,
		const DIFConfiguration& object);

DIFConfiguration * parseDIFConfigurationObject(nlattr *nested);

/* IpcmAssignToDIFRequestMessage CLASS*/
enum IpcmAssignToDIFRequestMessageAttributes {
	IATDR_ATTR_DIF_CONFIGURATION = 1,
	__IATDR_ATTR_MAX,
};

#define IATDR_ATTR_MAX (__IATDR_ATTR_MAX -1)

int putIpcmAssignToDIFRequestMessageObject(nl_msg* netlinkMessage,
		const IpcmAssignToDIFRequestMessage& object);

IpcmAssignToDIFRequestMessage *
	parseIpcmAssignToDIFRequestMessage(nlmsghdr *hdr);

/* IpcmAssignToDIFResponseMessage CLASS*/
enum IpcmAssignToDIFResponseMessageAttributes {
	IATDRE_ATTR_RESULT = 1,
	IATDRE_ATTR_ERROR_DESCRIPTION,
	__IATDRE_ATTR_MAX,
};

#define IATDRE_ATTR_MAX (__IATDRE_ATTR_MAX -1)

int putIpcmAssignToDIFResponseMessageObject(nl_msg* netlinkMessage,
		const IpcmAssignToDIFResponseMessage& object);

IpcmAssignToDIFResponseMessage *
	parseIpcmAssignToDIFResponseMessage(nlmsghdr *hdr);

/* IpcmAllocateFlowRequestMessage CLASS*/
enum IpcmAllocateFlowRequestMessageAttributes {
	IAFRM_ATTR_SOURCE_APP = 1,
	IAFRM_ATTR_DEST_APP,
	IAFRM_ATTR_FLOW_SPEC,
	IAFRM_ATTR_DIF_NAME,
	IAFRM_ATTR_PORT_ID,
	IAFRM_ATTR_APP_PORT,
	__IAFRM_ATTR_MAX,
};

#define IAFRM_ATTR_MAX (__IAFRM_ATTR_MAX -1)

int putIpcmAllocateFlowRequestMessageObject(nl_msg* netlinkMessage,
		const IpcmAllocateFlowRequestMessage& object);

IpcmAllocateFlowRequestMessage *
	parseIpcmAllocateFlowRequestMessage(nlmsghdr *hdr);

/* IpcmAllocateFlowResponseMessage CLASS*/
enum IpcmAllocateFlowResponseMessageAttributes {
	IAFREM_ATTR_RESULT = 1,
	IAFREM_ATTR_ERROR_DESCRIPTION,
	__IAFREM_ATTR_MAX,
};

#define IAFREM_ATTR_MAX (__IAFREM_ATTR_MAX -1)

int putIpcmAllocateFlowResponseMessageObject(nl_msg* netlinkMessage,
		const IpcmAllocateFlowResponseMessage& object);

IpcmAllocateFlowResponseMessage *
	parseIpcmAllocateFlowResponseMessage(nlmsghdr *hdr);


/* IpcmDIFRegistrationNotification CLASS*/
enum IpcmDIFRegistrationNotificationAttributes {
	IDRN_ATTR_IPC_PROCESS_NAME = 1,
	IDRN_ATTR_DIF_NAME,
	IDRN_ATTR_REGISTRATION,
	__IDRN_ATTR_MAX,
};

#define IDRN_ATTR_MAX (__IDRN_ATTR_MAX -1)

int putIpcmDIFRegistrationNotificationObject(nl_msg* netlinkMessage,
		const IpcmDIFRegistrationNotification& object);

IpcmDIFRegistrationNotification *
	parseIpcmDIFRegistrationNotification(nlmsghdr *hdr);

/* IpcmDIFQueryRIBRequestMessage CLASS*/
enum IpcmDIFQueryRIBRequestMessageAttributes {
	IDQR_ATTR_OBJECT_CLASS = 1,
	IDQR_ATTR_OBJECT_NAME,
	IDQR_ATTR_OBJECT_INSTANCE,
	IDQR_ATTR_SCOPE,
	IDQR_ATTR_FILTER,
	__IDQR_ATTR_MAX,
};

#define IDQR_ATTR_MAX (__IDQR_ATTR_MAX -1)

int putIpcmDIFQueryRIBRequestMessageObject(nl_msg* netlinkMessage,
		const IpcmDIFQueryRIBRequestMessage& object);

IpcmDIFQueryRIBRequestMessage *
	parseIpcmDIFQueryRIBRequestMessage(nlmsghdr *hdr);

/* RIBObject CLASS*/
enum RIBObjectAttributes {
	RIBO_ATTR_OBJECT_CLASS = 1,
	RIBO_ATTR_OBJECT_NAME,
	RIBO_ATTR_OBJECT_INSTANCE,
	__RIBO_ATTR_MAX,
};

#define RIBO_ATTR_MAX (__RIBO_ATTR_MAX -1)

int putRIBObject(nl_msg* netlinkMessage, const RIBObject& object);

RIBObject * parseRIBObject(nlattr *nested);

/* IpcmDIFQueryRIBResponseMessage CLASS*/
enum IpcmDIFQueryRIBResponseMessageAttributes {
	IDQRE_ATTR_RESULT = 1,
	IDQRE_ATTR_ERROR_DESCRIPTION,
	IDQRE_ATTR_RIB_OBJECTS,
	__IDQRE_ATTR_MAX,
};

#define IDQRE_ATTR_MAX (__IDQRE_ATTR_MAX -1)

int putIpcmDIFQueryRIBResponseMessageObject(nl_msg* netlinkMessage,
		const IpcmDIFQueryRIBResponseMessage& object);

IpcmDIFQueryRIBResponseMessage *
	parseIpcmDIFQueryRIBResponseMessage(nlmsghdr *hdr);

}

#endif /* LIBRINA_NETLINK_PARSERS_H_ */