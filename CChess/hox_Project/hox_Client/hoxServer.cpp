/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxServer.cpp
// Created:         10/24/2007
//
// Description:     The Server Thread to help this server dealing with
//                  network connections.
/////////////////////////////////////////////////////////////////////////////

#include "hoxServer.h"
#include "hoxSocketServer.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"
#include "hoxRemoteConnection.h"

DEFINE_EVENT_TYPE( hoxEVT_SERVER_RESPONSE )

//-----------------------------------------------------------------------------
// hoxServer
//-----------------------------------------------------------------------------

hoxServer::hoxServer( hoxSite* site )
        : wxThreadHelper()
		, m_socketServer( NULL )
        , m_shutdownRequested( false )
        , m_site( site )
{
    const char* FNAME = "hoxServer::hoxServer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxServer::~hoxServer()
{
    const char* FNAME = "hoxServer::~hoxServer";
    wxLogDebug("%s: ENTER.", FNAME);

    _DestroyAllActiveSockets();
}

hoxResult
hoxServer::StartServer( int nPort )
{
    const char* FNAME = "hoxServer::StartServer";

	/* Start the main server thread */

    if ( this->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create Server thread.", FNAME);
        return hoxRESULT_ERR;
    }
    wxASSERT_MSG( !this->GetThread()->IsDetached(), "The Server thread must be joinable.");

    this->GetThread()->Run();

	/* Start the socket-server thread */

    wxCHECK_MSG( m_socketServer == NULL, hoxRESULT_ERR, "The socket-server should not have been created.");

    m_socketServer = new hoxSocketServer( nPort,
                                          this,
                                          m_site );

    if ( m_socketServer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create socker-server thread.", FNAME);
        return hoxRESULT_ERR;
    }
    wxASSERT_MSG( !m_socketServer->GetThread()->IsDetached(), "The socket-server thread must be joinable.");

    m_socketServer->GetThread()->Run();

	return hoxRESULT_OK;
}

void      
hoxServer::CloseServer()
{
	const char* FNAME = "hoxServer::CloseServer";

	/* Close the socket-server thread */

	if ( m_socketServer != NULL )
	{
		wxLogDebug("%s: Request the socket-server thread to be shutdowned...", FNAME);
		m_socketServer->RequestShutdown();
		wxThread::ExitCode exitCode = m_socketServer->GetThread()->Wait();
		wxLogDebug("%s: The socket-server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
		delete m_socketServer;
		m_socketServer = NULL;
	}

	/* Close the main thread. */

	if ( this->GetThread()->IsRunning() )
	{
		wxLogDebug("%s: Request the Server thread to be shutdowned...", FNAME);
		hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
		this->AddRequest( request );
		wxThread::ExitCode exitCode = this->GetThread()->Wait();
		wxLogDebug("%s: The Server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
	}
}

void
hoxServer::_DestroyAllActiveSockets()
{
    const char* FNAME = "hoxServer::_DestroyAllActiveSockets";
    wxLogDebug("%s: ENTER.", FNAME);

    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
    {
        it->socket->Destroy();
    }
}

void
hoxServer::_DestroyActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DestroyActiveSocket";
    wxLogDebug("%s: ENTER.", FNAME);

    if ( _DetachActiveSocket( sock ) )
    {
        sock->Destroy();
    }
}

bool
hoxServer::_DetachActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DetachActiveSocket";
    wxLogDebug("%s: ENTER.", FNAME);

    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
    {
        if ( it->socket == sock )
        {
			hoxPlayer* player = m_site->FindPlayer( it->playerId );
			if ( player != NULL )
			{
				hoxConnection* connection = player->GetConnection();
				hoxRemoteConnection* remoteConnection = wxDynamicCast(connection, hoxRemoteConnection );
				if ( remoteConnection )
				{
					wxLogDebug("%s: Clear call-backk socket for player [%s].", 
						FNAME, player->GetName().c_str());
					remoteConnection->SetCBSocket( NULL );
				}
			}
            m_activeSockets.erase( it );
            return true;
        }
    }
    wxLogDebug("%s: Could NOT find the specified socket to detach.", FNAME);
    return false;
}

bool
hoxServer::_FindSocketInfo( const wxString& playerId,
                            SocketInfo&     socketInfo )
{
    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
    {
        if ( it->playerId == playerId )
        {
            socketInfo = (*it);
            return true;
        }
    }

    return false;
}

void*
hoxServer::Entry()
{
    const char* FNAME = "hoxServer::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( !m_shutdownRequested && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        request = _GetRequest();
        if ( request == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request [%s]...", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());

         _HandleRequest( request );
        delete request;
    }

    return NULL;
}

bool 
hoxServer::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::AddRequest";
    //wxLogDebug("%s: ENTER. Trying to obtain the lock...", FNAME);
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: *** WARN *** Deny request [%s]. The thread is shutdowning.", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
        delete request;
        return false;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
    //wxLogDebug("%s: END.", FNAME);
	return true;
}

void 
hoxServer::_HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::_HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type) );

    wxLogDebug("%s: ENTER.", FNAME);

    /* 
     * SPECIAL CASE: 
     *     Handle the "special" request: Socket-Lost event,
     *     which is applicable to any request.
     */
    if (    request->type == hoxREQUEST_TYPE_PLAYER_DATA
       )
    {
        result = _CheckAndHandleSocketLostEvent( request, response->content );
        if ( result == hoxRESULT_HANDLED )
        {
            response->flags |= hoxRESPONSE_FLAG_CONNECTION_LOST;
            result = hoxRESULT_OK;  // Consider "success".
            goto exit_label;
        }
    }

    /*
     * NORMAL CASE: 
     *    Handle "normal" request.
     */
    switch( request->type )
    {
        case hoxREQUEST_TYPE_ACCEPT:
            result = _HandleRequest_Accept( request );
            break;

        case hoxREQUEST_TYPE_MOVE: /* fall through */
        case hoxREQUEST_TYPE_NEW_JOIN: /* fall through */
        case hoxREQUEST_TYPE_LEAVE: /* fall through */
        case hoxREQUEST_TYPE_WALL_MSG:
            result = hoxNetworkAPI::SendRequest( request->socket, 
                                                 request->content,
                                                 response->content );
            break;

        case hoxREQUEST_TYPE_PLAYER_DATA: // Incoming data from remote player.
        {
            wxSocketBase* sock = request->socket;
            // We disable input events until we are done processing the current command.
            hoxNetworkAPI::SocketInputLock socketLock( sock );
            result = hoxNetworkAPI::ReadLine( sock, response->content );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to read incoming command.", FNAME);
                goto exit_label;
            }
            break;
        }

        case hoxREQUEST_TYPE_OUT_DATA:
            result = hoxNetworkAPI::SendOutData( request->socket, 
                                                 request->content );
            break;

        default:
            wxLogError("%s: Unsupported Request-Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

exit_label:
    /* Log error */
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_SERVER_RESPONSE, request->type );
        response->code = result;
        event.SetEventObject( response.release() );
        wxPostEvent( request->sender, event );
    }
}

hoxResult 
hoxServer::_CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                           wxString&         response )
{
    const char* FNAME = "hoxServer::_CheckAndHandleSocketLostEvent";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    wxSocketBase* sock = request->socket;
    
    if ( request->socketEvent == wxSOCKET_LOST )
    {
        wxLogDebug("%s: Received socket-lost event. Deleting client socket.", FNAME);
        _DestroyActiveSocket( sock );
        result = hoxRESULT_HANDLED;
    }

    //wxLogDebug("%s: Not a socket-lost event. Fine - Do nothing. END.", FNAME);
    return result;
}

hoxResult 
hoxServer::_HandleRequest_Accept( hoxRequest* request ) 
{
    const char* FNAME = "hoxServer::_HandleRequest_Accept";  // function's name
    const wxString playerId = request->content;
    wxSocketBase* socket = request->socket;

    wxLogDebug("%s: Saving an active (socket) connection.", FNAME);
    wxCHECK_MSG(socket, hoxRESULT_ERR, "The socket cannot be NULL.");

    /* TODO: Check for existing player before create a new player/info. */

    /* Save the player + socket in our list */
    SocketInfo socketInfo(playerId, socket);
    m_activeSockets.push_back( socketInfo );

    return hoxRESULT_OK;
}

hoxRequest*
hoxServer::_GetRequest()
{
    const char* FNAME = "hoxServer::_GetRequest";
    wxMutexLocker lock( m_mutexRequests );

    hoxRequest* request = NULL;

    wxASSERT_MSG( !m_requests.empty(), "We must have at least one request.");
    request = m_requests.front();
    m_requests.pop_front();

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( request->type == hoxREQUEST_TYPE_SHUTDOWN )
    {
        wxLogDebug("%s: Shutting down this thread...", FNAME);
        m_shutdownRequested = true;
        delete request; // *** Signal "no more request" ...
        return NULL;    // ... to the caller!
    }

    return request;
}

/************************* END OF FILE ***************************************/
