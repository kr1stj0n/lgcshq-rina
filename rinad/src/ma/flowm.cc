#include "flowm.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "agent.h"
#define RINA_PREFIX "mad.flowm"
#include <librina/logs.h>

namespace rinad{
namespace mad{

/*
* Flow worker
*/
void* FlowWorker::run(void* param){

	(void)param;
	//Recover the port_id of the flow
	keep_on = true;

	while(keep_on == true){
		//TODO: block for incoming messages
		sleep(1);
	}

	return NULL;
}

/*
* FlowManager
*/

//Singleton instance
Singleton<FlowManager_> FlowManager;

//Constructors destructors(singleton)
FlowManager_::FlowManager_(){

	//TODO: register to flow events in librina and spawn workers
	pthread_mutex_init(&mutex, NULL);
}

FlowManager_::~FlowManager_(){
	pthread_mutex_destroy(&mutex);
}

//Initialization and destruction routines
void FlowManager_::init(){
	//TODO
	LOG_DBG("Initialized");
}
void FlowManager_::destroy(){
	//TODO
}

//
// FIXME: spawning a thread per flow is a waste of resources in general
// but currently there is no functionality like select/poll/epoll to wait for
// read events over a bunch of flows, so this is currently
// the only way to go.
//

//Workers
void FlowManager_::spawnWorker(int port_id){

	FlowWorker* w = NULL;
	std::stringstream msg;

	//Create worker object
	try{
		w = new FlowWorker();
	}catch(std::bad_alloc& e){
		msg <<std::string("ERROR: Unable to create worker context; out of memory? Dropping flow with port_id:")<<port_id<<std::endl;
		goto SPAWN_ERROR;
	}

	pthread_mutex_lock(&mutex);

	//Double check that there is not an existing worker for that port_id
	//This should never happen
	//TODO: evaluate whether to surround this with an ifdef DEBUG
	if(workers.find(port_id) != workers.end()){
		msg <<std::string("ERROR: Corrupted FlowManager internal state or double call to spawnWorker(). Dropping flow with port_id: ")<<port_id<<std::endl;
		goto SPAWN_ERROR;
	}

	//Add state to the workers map
	try{
		workers[port_id] = w;
	}catch(...){
		msg <<std::string("ERROR: Could not add the worker state to 'workers' FlowManager internal out of memory? Dropping flow with port_id: ")<<port_id<<std::endl;
		goto SPAWN_ERROR;
	}

	//Spawn pthread
	if(pthread_create(w->getPthreadContext(), NULL,
					w->run_trampoline, (void*)w) != 0){
		msg <<std::string("ERROR: Could spawn pthread for flow with port_id: ")<<port_id<<std::endl;
		goto SPAWN_ERROR;
	}

	pthread_mutex_unlock(&mutex);

	return;

SPAWN_ERROR:
	if(w){
		if(workers.find(port_id) != workers.end())
			workers.erase (port_id);
		delete w;
	}
	assert(0);
	pthread_mutex_unlock(&mutex);
	throw Exception(msg.str().c_str());
}

//Join
void FlowManager_::joinWorker(int port_id){

	std::stringstream msg;
	FlowWorker* w = NULL;

	pthread_mutex_lock(&mutex);

	if(workers.find(port_id) == workers.end()){
		msg <<std::string("ERROR: Could not find the context offlow with port_id: ")<<port_id<<std::endl;
		goto JOIN_ERROR;
	}

	//Recover FlowWorker object
	w = workers[port_id];

	//Signal
	w->stop();

	//Join
	if(pthread_join(*w->getPthreadContext(), NULL) !=0){
		msg <<std::string("ERROR: Could not join worker for flow id: ")<<port_id<<". Error: "<<strerror(errno)<<std::endl;
		goto JOIN_ERROR;
	}

	//Clean-up
	workers.erase(port_id);
	delete w;

	pthread_mutex_unlock(&mutex);

	return;

JOIN_ERROR:
	assert(0);
	pthread_mutex_unlock(&mutex);
	throw Exception(msg.str().c_str());

}

}; //namespace mad
}; //namespace rinad


