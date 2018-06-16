// EventService.cpp : Defines the entry point for the console application.
//
#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "MSGHandleCenter.h"
#include "MyLog.h"
#include "Cmd_Acceptor.h"

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor *)arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

int main(int argc, char* argv[])
{

    /* Ignore signals generated when a connection is broken unexpectedly. */
    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);


    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);


    //get config details
    CSimpleConfig cg;
    cg.get_config();

    CMyLog::Init();
    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);

    /*  start event and control thread   */
    ACE_Thread_Manager::instance()->spawn(event_loop, reactor);
    ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up controlcenter daemon\n"));
    ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));

    MSG_HANDLE_CENTER::instance()->open();
    /*
     * read config from config file,and open port ,listen
     */

    ACE_Reactor* reactorptr = ACE_Reactor::instance();

    CCmd_Acceptor::InitHandler();
    CCmd_Acceptor peer_acceptor;
    if(peer_acceptor.open(ACE_INET_Addr(CSimpleConfig::EVENT_LISTEN_PORT), reactorptr, 1) == -1){
        ACE_ERROR_RETURN((LM_ERROR, "%p\n", "open listen port fail..."), -1);
    }
    else {
        ACE_DEBUG((LM_DEBUG, "(%P|%t) open listen port %d ok...\n", CSimpleConfig::EVENT_LISTEN_PORT));
    }


    MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();

    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));

    return 0;
}

