
#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include "config.h"
extern "C" {
#include <X11/Intrinsic.h>
#include <Xm/XmAll.h>

#ifdef HAVE_INTERACTIVE_IPC_NONTUX
#include "libipcnt.h"
#endif

#ifdef HAVE_INTERACTIVE_IPC_TUXEDO
#include "libipc.h"
#endif
}

#include <list>
#include <string>
#include <stdexcept>

using namespace std;

/** This is the abstract base class from which message receivers for specific 
 *  types of messages can be derived. 
 *  Instances of derived classes can be registered with IPCClient for 
 *  receiving messages that are then parsed by the derived classes' method 
 *  receiveIPCMessage. 
 *  parseIPCMessage is expected to produce a list of geotool++ commands which 
 *  is placed into the commands list. IPCClient::receiveIPCMessage calls parseIPCMessage 
 *  for each of its registered receivers and then issues the commands in that 
 *  receivers commands list.
 *  IPCClient::receiveIPCMessage also calls the preProcessHook and postProcessHook before 
 *  and after it issues the commands produced by the respective receiver.  
 *
 *  @ingroup libmotif
*/
class IPCMessageReceiver {
 public: 
    IPCMessageReceiver() {};
    virtual ~IPCMessageReceiver() {};

    /** pure virtual function to parse the message received by a specific receiver */
    virtual void parseIPCMessage(const char* msg_id, const char* msg, const char* msg_class) throw(runtime_error) {};
    
    /** Contains processing to be done done before issuing commands generated through parsing */
    void preProcessHook() {};
    
    /** Contains processing to be done after issuing commands generated through parsing */
    void postProcessHook() {};
    
    /** Return a begin iterator for the list of commands */
    list<string>::const_iterator scriptStart() {
	return commands.begin();
    }

    /** Return an end iterator for the list of commands */
    list<string>::const_iterator scriptEnd() {
	return commands.end();
    }

    /** Helper function to get the value of a "name=value" property from a string */
    Boolean getProp(const char* msg, const char* name, string* value);

protected:
 
    //!< list of commands to be produced by parseIPCMessage 
    list<string> commands;
};

/** This class encapsulates the functionality needed to connect, receive and 
 * send messages via libipcnt (nontux) or libipc (tuxedo). 
 * 
 * The following lines of code illustrate the usage of the IPCClient in 
 * Application::init where the connection to the server (nontux or tuxedo) is
 * established 
 * \code

void Application::init(const char *name, int *argcp, const char **argv,
		const char *version_str, const char *installation_dir,
		const char *only_plugins)
{
  
    ....
  
    IPCClient* ipcc = IPCClient::getInstance();
    ipcc->connect(argcp, argv);
    
}\endcode

 * Specific message receiver classes derived from IPCMessageReceiver can register with 
 * IPCClient to receive messages IPCClient receives from the server. 
 * An example of this is shown for the class PMCCMessageReceiver which is part of the gpmcc 
 * plugin. 
 * \code
 
 PMCCMessageReceiver pmccReceiver;
 
 IPCClient* ipcc = IPCClient::getInstance();
 ipcc->registerReceiver(&pmccReceiver);

\endcode

 * The use of tuxedo, nontux or the disabling of ipc communication within the IPCClient 
 * code is based on the preprocessor variables HAVE_INTERACTIVE_IPC_NONTUX, 
 * HAVE_INTERACTIVE_IPC_TUXEDO and HAVE_INTERACTIVE_IPC which are defined in config.h 
 * based on the value of the configure flag --enable-interactive-ipc.  
 * Classes calling IPCClient do not need to use the above variables but can call IPCClient 
 * methods irrespective of whether the code is configured with IPC enabled or not. 
 * 
 * The IPCClient is implemented as a singleton. 
 * @ingroup libmotif
 */
class IPCClient {
 public: 
    //!< singleton method that gives access to the private instance of the class  
    static IPCClient* getInstance();
    void connect(int *argcp, const char **argv);
    /** Template method called when geotool++ receives an ipc message from a nontux or tuxedo server */
    static void receiveIPCMessage();
    /** Method called to send an IPC message */  
    static void sendIPCMessage(string dest, string msg, string cl, string id) throw(runtime_error);
    /** Register a specific receiver object for forwarding incoming messages */
    void registerReceiver(IPCMessageReceiver* recv);
    /** De-register a specific receiver object to stop forwarding of incoming messages to it */  
    void deRegisterReceiver(IPCMessageReceiver* recv);

 private:
    IPCClient() {};
    ~IPCClient() {};

    //!< private instance of the Singleton. 
    static IPCClient* instance;
    //!< boolean showing if the private instance was initialized or not. 
    static bool instanceFlag;
#ifdef HAVE_INTERACTIVE_IPC
    //!< ipc connection object defined in libipcnt or libipc. 
    static ipcConn* ipc_conn;
#endif
    //!< list holding registered receivers  
    static list<IPCMessageReceiver*> receivers;
    XtAppContext app_context;



};



#endif /* IPC_CLIENT_H */

