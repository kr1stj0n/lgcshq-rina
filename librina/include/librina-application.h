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

#ifndef LIBRINA_APPLICATION_H
#define LIBRINA_APPLICATION_H

#include "librina-common.h"
#include <map>

/**
 * The librina-application library provides the native RINA API,
 * allowing applications to i) express their availability to be
 * accessed through one or more DIFS (application registration);
 * ii) allocate and deallocate flows to destination applications
 * (flow allocation and deallocation); iii) read and write data
 * from/to allocated flows (in the form of Service Data Units or
 * SDUs) and iv) query the DIFs available in the system and
 * their properties.
 *
 * For the "slow-path" operations, librina-application interacts
 * with the RINA daemons in by exchanging messages over Netlink
 * sockets. In the case of the "fast-path" operations - i.e. those
 * that need to be invoked for every single SDU: read and write -
 * librina-application communicates directly with the services
 * provided by the kernel through the use of system calls.
 *
 * The librina-application API is event-based; that is: the API
 * provides a different method for each action that can be invoked
 * (allocate_flow, register_application and so on), but only two
 * methods - one blocking, the other non-blocking - to get the
 * results of the operations and SDUs available to be read
 * (event_wait and event_poll).
 */

enum FlowState {FLOW_ALLOCATED, FLOW_DEALLOCATED};

/**
 * Represents a flow between two application processes, and encapsulates
 * the services that the flow provides.
 */
class Flow{
	/** The port-id that locally identifies the flow */
	int portId;

	/** The name of the DIF that is providing this flow */
	ApplicationProcessNamingInformation DIFName;

	/** The application that requested the flow */
	ApplicationProcessNamingInformation sourceApplicationName;

	/** The application targeted by the flow */
	ApplicationProcessNamingInformation destinationApplicationName;

	/** The characteristics of the flow */
	FlowSpecification flowSpecification;

	/** The state of the flow */
	FlowState flowState;

	Flow(const ApplicationProcessNamingInformation& sourceApplicationName,
		 const ApplicationProcessNamingInformation& destinationApplicationName,
		 const FlowSpecification& flowSpecification, FlowState flowState,
		 const ApplicationProcessNamingInformation& DIFName, int portId);
public:
	static const std::string flow_not_allocated_error;
	const FlowState& getState() const;
	int getPortId() const;
	const ApplicationProcessNamingInformation& getDIFName() const;
	const ApplicationProcessNamingInformation& getSourceApplicationName() const;
	const ApplicationProcessNamingInformation& getDestinationApplcationName() const;
	const FlowSpecification getFlowSpecification() const;

	/**
	 * Reads an SDU from the flow. This function will block until there is an
	 * SDU available.
	 *
	 * @param sdu A buffer to store the SDU data
	 * @return int The number of bytes read
	 * @throws IPCException if the flow is not in the ALLOCATED state
	 */
	int readSDU(unsigned char * sdu) throw(IPCException);

	/**
	 * Writes an SDU to the flow
	 *
	 * @param sdu A buffer that contains the SDU data
	 * @param size The size of the SDU data, in bytes
	 * @throws IPCException if the flow is not in the ALLOCATED state or
	 * there are problems writing to the flow
	 */
	void writeSDU(unsigned char * sdu, int size) throw(IPCException);

	friend class IPCManager;
};

/**
 * Contains the information about a registered application: its
 * name and the DIFs where it is registered
 */
class ApplicationRegistration{
	/** The registered application name */
	ApplicationProcessNamingInformation applicationName;

	/** The list of one or more DIFs in which the application is registered */
	std::list<ApplicationProcessNamingInformation> DIFNames;

public:
	ApplicationRegistration(
			const ApplicationProcessNamingInformation& applicationName);
	const ApplicationProcessNamingInformation& getApplicationName() const;
	const std::list<ApplicationProcessNamingInformation>& getDIFNames() const;
	void addDIFName(const ApplicationProcessNamingInformation& DIFName);
	void removeDIFName(const ApplicationProcessNamingInformation& DIFName);
};

/**
 * Point of entry to the IPC functionality available in the system. This class
 * is a singleton.
 */
class IPCManager : public IPCEventStore{
	static bool instanceFlag;
	static IPCManager * instance;

	/** The flows that are currently allocated */
	std::map<int, Flow> allocatedFlows;

	/** The applications that are currently registered in one or more DIFs */
	std::map<ApplicationProcessNamingInformation, ApplicationRegistration > applicationRegistrations;

	IPCManager();

public:
	static IPCManager * getInstance();
	~IPCManager(){
		instanceFlag = false;
	}

	const std::string application_registered_error;
	const std::string application_not_registered_error;
	const std::string unknown_flow_error;

	/**
	 * Retrieves the names and characteristics of a single DIF or of all the
	 * DIFs available to the application.
	 *
	 * @param DIFName If provided, the function will return the information of
	 * the requested DIF, otherwise it will return the properties of all the
	 * DIFs available to the application.
	 * @return The properties of one or more DIFs
	 */
	std::vector<DIFProperties> getDIFProperties(
			const ApplicationProcessNamingInformation& DIFName);

	/**
	 * Registers an application to a DIF.
	 *
	 * @param applicationName The name of the application to be registered
	 * @param DIFName Then name of the DIF where the application will register
	 * @throws IPCException if the DIF doesn't exist or the application doesn't
	 * have enough rights to use it.
	 */
	void registerApplication(const ApplicationProcessNamingInformation& applicationName,
			const ApplicationProcessNamingInformation& DIFName) throw(IPCException);

	/**
	 * Unregisters an application from a DIF.
	 *
     * @param applicationName The name of the application to be unregistered
	 * @param DIFName Then name of the DIF where the application has to be
	 * unregistered from
	 * @throws IPCException if the DIF doesn't exist or the application was not
	 * registered there
	 */
	void unregisterApplication(ApplicationProcessNamingInformation applicationName,
			ApplicationProcessNamingInformation DIFName) throw(IPCException);

	/**
	 * Requests the allocation of a Flow
	 *
	 * @param sourceAppName The naming information of the application requesting
	 * the flow
	 * @param destAppName The naming information of the application that is the target
	 * of the flow
	 * @param flowSpecifiction The characteristics required for the flow
	 * @return A Flow object encapsulating the flow service
	 * @throws IPCException if there are problems during the flow allocation
	 */
	const Flow& allocateFlowRequest(const ApplicationProcessNamingInformation& sourceAppName,
			const ApplicationProcessNamingInformation& destAppName,
			const FlowSpecification& flow) const throw(IPCException);

	/**
	 * Confirms or denies the request for a flow to this application.
	 *
	 * @param portId The Id of the flow to confirm/deny
	 * @param accept true if the flow is accepted, false otherwise
	 * @param reason IF the flow was denied, contains a short explanation
	 * providing some motivation
	 * @return Flow If the flow is accepted, returns the flow object
	 * @throws IPCException If there are problems confirming/denying the flow
	 */
	const Flow& allocateFlowResponse(int portId, bool accept,
			const std::string& reason) const throw(IPCException);

	/**
	 * Causes the flow to be deallocated, and the object deleted.
	 *
	 * @throws IPCException if the flow is not in the ALLOCATED state or
	 * there are problems deallocating the flow
	 */
	void deallocateFlow(const Flow& flow) throw(IPCException);

	/**
	 * Returns the flows that are currently allocated
	 *
	 * @return the flows allocated
	 */
	std::vector<Flow> getAllocatedFlows();

	/**
	 * Returns the applications that are currently registered in one or more
	 * DIFs.
	 *
	 * @return the registered applications
	 */
	std::vector<ApplicationRegistration> getRegisteredApplications();

	IPCEvent eventPoll();
	IPCEvent eventWait();
	IPCEvent eventWait(long maxTimeInMilis);
};

/**
 * Event informing that a flow has been deallocated by an IPC Process, without
 * the application having requested it
 */
class FlowDeallocatedEvent: public IPCEvent{
	Flow flow;
public:
	FlowDeallocatedEvent(const Flow& flow);
	const Flow& getFlow();
};

/**
 * Event informing that an application has been unregistered from a DIF,
 * without the application having requested it
 */
class ApplicationUnregisteredEvent: public IPCEvent{
	/** The application that has been unregistered */
	ApplicationProcessNamingInformation applicationName;

	/** The DIF from which the application has been unregistered */
	ApplicationProcessNamingInformation DIFName;

public:
	ApplicationUnregisteredEvent(
			const ApplicationProcessNamingInformation& appName,
			const ApplicationProcessNamingInformation& DIFName);
	const ApplicationProcessNamingInformation& getApplicationName() const;
	const ApplicationProcessNamingInformation& getDIFName() const;
};

/**
 * Event informing about an incoming flow request from another application
 */
class IncomingFlowRequestEvent: public IPCEvent{
	/** The port-id that locally identifies the flow */
	int portId;

	/** The name of the DIF that is providing this flow */
	ApplicationProcessNamingInformation DIFName;

	/** The application that requested the flow */
	ApplicationProcessNamingInformation sourceApplicationName;

	/** The application targeted by the flow */
	ApplicationProcessNamingInformation destinationApplicationName;

	/** The characteristics of the flow */
	FlowSpecification flowSpecification;

public:
	IncomingFlowRequestEvent(int portId,
			const FlowSpecification& flowSpecification,
			const ApplicationProcessNamingInformation& sourceApplicationName,
			const ApplicationProcessNamingInformation& destApplicationName,
			const ApplicationProcessNamingInformation& DIFName);
	int getPortId() const;
	const FlowSpecification& getFlowSpecification() const;
	const ApplicationProcessNamingInformation& getDIFName() const;
	const ApplicationProcessNamingInformation& getSourceApplicationName() const;
	const ApplicationProcessNamingInformation& getDestApplicationName() const;
};


#endif
