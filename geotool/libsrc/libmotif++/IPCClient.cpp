
extern "C" {
void receiveIPCMessageC();

#include "libstring.h" 
}

#include "motif++/Application.h"
#include "motif++/IPCClient.h"
#include <list>

#define COMMAND_LENGTH 1024

using namespace std;

/* initialize static variables */
bool IPCClient::instanceFlag = false;
IPCClient* IPCClient::instance = NULL;

#ifdef HAVE_INTERACTIVE_IPC
ipcConn* IPCClient::ipc_conn = NULL;
#endif

list<IPCMessageReceiver*> IPCClient::receivers;


/** Singleton's instance operation 
 *  @return The singleton's instance 
 */ 
IPCClient* IPCClient::getInstance() {
    if(!instanceFlag) {
	instance = new IPCClient();
	instanceFlag = True;
    }
    return instance;
}

/** Function to connect to the IPC server (nontux or tuxedo).  
 *  @param argcp pointer to the number of command line args
 *  @param argv  pointer to the list of command line args 
 *  TODO: raise an error if connection fails. 
 */ 
void IPCClient::connect(int* argcp, const char **argv) {
#ifdef HAVE_INTERACTIVE_IPC
    char*   agent;
    char*   name;
    char*   passwd;
 
    app_context = Application::getAppContext();

    /*
    strcpy(group, "");
    strcpy(name, "1/geotool");
    strcpy(passwd, "");
    */    

    /* setpar is called from main() 
    getpar("group", "s", group);
    getpar("name", "s", name);
    getpar("password", "s", passwd);
    */

    agent = stringGetArgv(*argcp, argv, "agent");
    name = stringGetArgv(*argcp, argv, "name");
    passwd = stringGetArgv(*argcp, argv, "password");

    

    /* don't check for group or password, since no applications do this */
    if (agent==NULL || name==NULL)
	{
	    fprintf(stderr, "cannot connect to ipc: name or agent not specified\n");
	    return;
	}
    if (passwd==NULL) {
	passwd = (char *)malloc(sizeof(char));
	passwd[0] = '\0';
    }
    /* assuming group, name, and passwd have values */
    if ((ipc_conn=ipc_attach(agent, name, passwd, TUXEDO)) != NULL &&
	ipc_add_xcallback(ipc_conn, &app_context, receiveIPCMessageC, 500)
	== IPCSUCCESS )
        {
	    /* success with IPC setup */
	    /* fprintf(stderr, "connected to ipc: group=%s name=%s \n", 
	       agent, name); */
        }
    else
        {
	    /* failure with IPC setup */
	    /* fprintf(stderr, "cannot connect to ipc: group=%s name=%s \n", 
	       agent, name); */
        }
    
    Free(agent); Free(name); Free(passwd);    

#endif
}

/** Function to register a receiver derived from abstract class IPCMessageReceiver.
  * IPCClient will dispatch all incoming messages to all regsitered receivers by 
  * calling the receivers' parseIPCMessage function. 
  * A receiver must decide if it can parse the message or not. 
  * @param recv pointer to the receiver object to register 
  */
void IPCClient::registerReceiver(IPCMessageReceiver* recv) {
    if (recv != NULL) {
	IPCClient::receivers.push_back(recv);
    }
}

/** Function to de-register a registered receiver. 
 *  The function will attempt to remove the receiver from the list of 
 *  registered receivers that IPCClient maintains. 
 *  @param recv receiver to be de-registered. 
 */ 
void IPCClient::deRegisterReceiver(IPCMessageReceiver* recv) {
    if(!IPCClient::receivers.empty()) {
	list<IPCMessageReceiver*>::iterator rec_start = IPCClient::receivers.begin();
	list<IPCMessageReceiver*>::iterator rec_end = IPCClient::receivers.end();
	while(rec_start != rec_end) {
	    if(*rec_start == recv) {
		IPCClient::receivers.erase(rec_start);
	    }
	    else {
		++rec_start;
	    }
	}
    }
}


/** Function to send an IPC message. 
 *  @param dest destination of the message (application registered with tuxedo or nontux)
 *  @param msg  the message itself 
 *  @param cl   message class used by some applications to categorize messages 
 *  @param id   message id  used by some applications to categorize messages 
 *  The function throws a runtime error if an error occurs while sending the message. 
 */ 
void IPCClient::sendIPCMessage(string dest, string msg, string cl, string id) throw(runtime_error) {

#ifdef HAVE_INTERACTIVE_IPC

    char    msg_id[IPC_ID_LENGTH];
    char    msg_class[IPC_ID_LENGTH];
    char    msg_dest[IPC_ID_LENGTH];
    char    msg_data[IPC_DATA_LENGTH];

    strncpy(msg_id,    id.c_str(), IPC_ID_LENGTH);
    strncpy(msg_class, cl.c_str(), IPC_ID_LENGTH);
    strncpy(msg_dest,  dest.c_str(), IPC_ID_LENGTH);
    strncpy(msg_data,  msg.c_str(), IPC_DATA_LENGTH);

    if(IS_ATTACHED(ipc_conn)) {
	if(!dest.empty()) {
	    int ret = ipc_send(ipc_conn, msg_dest, msg_data, msg_class, msg_id);
	    if (ret != IPCSUCCESS) {
		throw runtime_error("Could not send IPC Message");
	    }
	}
    }

#endif

}

/** C wrapper around IPCClient::receiveIPCMessage().
  * This is needed because of the receiver functin must be registered 
  * through C calls to libipc(nt) functions to receive messages. 
  */   
void receiveIPCMessageC() {
    IPCClient::receiveIPCMessage();
}

/** Function called when an IPC message is received.  
 *  It is a template method which goes through the list of registered 
 *  receivers and calls the parseIPCMessage function of each receiver. 
 *  It then calls the receiver's preProcessHook, issues the commands 
 *  in the receiver's commands list and finally calls the receiver's 
 *  postProcessHook(). 
 */   
void IPCClient::receiveIPCMessage() {

#ifdef HAVE_INTERACTIVE_IPC

    	char    msg_id[IPC_ID_LENGTH];
	char    msg[IPC_DATA_LENGTH];
	char    lpoid[IPC_ADDRESS_LENGTH];
        char    comm[COMMAND_LENGTH];
 
	Application* app = Application::getApplication();
	/* read CommAgent messages */
	while(ipc_receive(ipc_conn,lpoid,msg,msg_id,NULL))
	{
		printf("processing message: %s, %s, %s\n", 
		       msg_id, msg, lpoid );
		if(!IPCClient::receivers.empty()) {
		    list<IPCMessageReceiver*>::iterator rec_start = IPCClient::receivers.begin();
		    list<IPCMessageReceiver*>::iterator rec_end = IPCClient::receivers.end();
		    while(rec_start != rec_end) {
			try {
			(*rec_start)->parseIPCMessage(msg_id, msg, lpoid);
			} catch (runtime_error) {
			    /* handle this somehow */
			    printf("Error parsing Message\n");
			    rec_start++;
			    continue;
			}
			list<string>::const_iterator startScr = (*rec_start)->scriptStart();
			list<string>::const_iterator endScr = (*rec_start)->scriptEnd();
			(*rec_start)->preProcessHook();
			while(startScr != endScr) {
			    cout << *startScr <<endl; 
			    strncpy(comm, startScr->c_str(), COMMAND_LENGTH);
			    app->parseLine(comm);
			    startScr++;
			}
			(*rec_start)->postProcessHook();
			rec_start++;
		    }
		}
	}
#endif

}

/** Helper function for the IPCMessageReceiver derived classes. 
 * Attempts to parse strings of the form name=value for a given name 
 * and returns the value. 
 * @param msg the string to be parsed 
 * @param name the name of the property we are trying to parse 
 * @param[out] value the value associated with "name" in msg.
 * @return true if a correct "name=value" pair was parsed in msg, false otherwise. 
 */ 
Boolean IPCMessageReceiver::getProp (const char *msg, const char *name, string* value) {

    string msgStr(msg);
    string propName(name);
    int nameStart, valStart, valEnd;

    value->clear();
    nameStart = msgStr.find(propName + "=", 0);
    if (nameStart >= 0) {
	try {
	    if(msgStr.at(nameStart + propName.length() + 1) == '"') {
		/* quote delimited */
		valStart = nameStart + propName.length() + 2;
		valEnd = msgStr.find(string("\""), valStart);
		if (valEnd >= 0) {
		    *value = msgStr.substr(valStart, valEnd - valStart);
		    return True;
		}
		else {
		    return False;
		}
	    }
	}
	catch(int e) {
	    return False;
	}
	
	/* no quotes, space delimited */
	valStart = nameStart + propName.length() + 1;
	valEnd = msgStr.find(string(" "), valStart);
	if (valEnd >= 0) {
	    *value = msgStr.substr(valStart, valEnd - valStart);
	    return True;
	}
	else if ((int)msgStr.length() > valStart) {
	    *value = msgStr.substr(valStart, msgStr.length() - valStart);
	    return True;
	}
	return False; 
    }
    return False;
}
