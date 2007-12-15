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
// Name:            hoxChesscapePlayer.cpp
// Created:         12/12/2007
//
// Description:     The Chesscape LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxChesscapePlayer.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"      // wxGetApp()
#include "MyFrame.h"
#include <wx/tokenzr.h>

IMPLEMENT_DYNAMIC_CLASS(hoxChesscapePlayer, hoxLocalPlayer)

BEGIN_EVENT_TABLE(hoxChesscapePlayer, hoxLocalPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxChesscapePlayer::OnIncomingNetworkData)
    EVT_COMMAND(hoxREQUEST_TYPE_PLAYER_DATA, hoxEVT_CONNECTION_RESPONSE, hoxChesscapePlayer::OnConnectionResponse_PlayerData)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxChesscapePlayer::OnConnectionResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxChesscapePlayer
//-----------------------------------------------------------------------------

hoxChesscapePlayer::hoxChesscapePlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxChesscapePlayer::hoxChesscapePlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxLocalPlayer( name, type, score )
{ 
    const char* FNAME = "hoxChesscapePlayer::hoxChesscapePlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxChesscapePlayer::~hoxChesscapePlayer() 
{
    const char* FNAME = "hoxChesscapePlayer::~hoxChesscapePlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxResult 
hoxChesscapePlayer::ConnectToNetworkServer( wxEvtHandler* sender )
{
    this->StartConnection();

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
	request->parameters["pid"] = this->GetName();
	request->parameters["password"] = this->GetPassword();
    //request->content = 
    //    wxString::Format("op=CONNECT&pid=%s\r\n", this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

hoxResult 
hoxChesscapePlayer::QueryForNetworkTables( wxEvtHandler* sender )
{
	const char* FNAME = "hoxChesscapePlayer::QueryForNetworkTables";
	wxLogDebug("%s: ENTER.", FNAME);

	/* Just return the "cache" list. */

	hoxRequestType requestType = hoxREQUEST_TYPE_LIST;
    hoxResponse_AutoPtr response( new hoxResponse(requestType, 
                                                  sender) );
	/* Clone the "cache" list and return the cloned */
	hoxNetworkTableInfoList* pTableList = new hoxNetworkTableInfoList;

	for ( hoxNetworkTableInfoList::const_iterator it = m_networkTables.begin();
		                                          it != m_networkTables.end(); 
												++it )
	{
		pTableList->push_back( (*it) );
	}
	
	response->eventObject = pTableList;

    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
    response->code = hoxRESULT_OK;
    event.SetEventObject( response.release() );  // Caller will de-allocate.
    wxPostEvent( sender, event );

	return hoxRESULT_OK;
}

hoxResult 
hoxChesscapePlayer::JoinNetworkTable( const wxString& tableId,
                                      wxEvtHandler*   sender )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_JOIN, sender );
	request->parameters["pid"] = this->GetName();
	request->parameters["tid"] = tableId;
    //request->content = 
    //    wxString::Format("op=JOIN&tid=%s&pid=%s\r\n", tableId.c_str(), this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

void
hoxChesscapePlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxChesscapePlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA, this );
    request->socket      = event.GetSocket();
    request->socketEvent = event.GetSocketEvent();
    this->AddRequestToConnection( request );
}

void 
hoxChesscapePlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxChesscapePlayer::OnConnectionResponse_PlayerData";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    /* NOTE: Only handle the connection-lost event. */

    if ( (response->flags & hoxRESPONSE_FLAG_CONNECTION_LOST) !=  0 )
    {
        wxLogDebug("%s: Connection has been lost.", FNAME);
        /* Currently, we support one connection per player.
         * Since this ONLY connection is closed, the player must leave
         * all tables.
         */
        this->LeaveAllTables();
    }
    else
    {
		wxString contentStr = response->content;
		wxString command;
		wxString paramsStr;

		/* CHECK: The first character must be 0x10 */
		if ( contentStr.empty() || contentStr[0] != 0x10 )
		{
			wxLogDebug("%s: Invalid command = [%s].", FNAME, contentStr.c_str());
			goto exit_label;
		}

		/* Chop off the 1st character */
		contentStr = contentStr.Mid(1);

		/* Extract the command and its parameters-string */
		command = contentStr.BeforeFirst( '=' );
		wxLogDebug("%s: Received the command = [%s].", FNAME, command.c_str());
		
		paramsStr = contentStr.AfterFirst('=');

		if ( command == "login" )
		{
			wxLogDebug("%s: Processing LOGIN command...", FNAME);
			/* Parse for login-info */
			wxString name;
			wxString score;
			wxString role;

			wxString delims;
			delims += 0x10;
			// ... Do not return empty tokens
			wxStringTokenizer tkz( paramsStr, delims, wxTOKEN_STRTOK );
			int tokenPosition = 0;
			wxString token;
			while ( tkz.HasMoreTokens() )
			{
				token = tkz.GetNextToken();
				switch (tokenPosition)
				{
					case 0: name = token; break;
					case 1: score = token; break;
					case 2: role = token; break;
					default: /* Ignore the rest. */ break;
				}
				++tokenPosition;
			}		

			wxLogDebug("%s: .... name=[%s], score=[%s], role=[%s].", 
				FNAME, name.c_str(), score.c_str(), role.c_str());
		}
		else if ( command == "show" )
		{
			wxLogDebug("%s: Processing SHOW command...", FNAME);
			wxString delims;
			delims += 0x11;   // table-delimiter
			// ... Do not return empty tokens
			wxStringTokenizer tkz( paramsStr, delims, wxTOKEN_STRTOK );
			wxString tableStr;
			while ( tkz.HasMoreTokens() )
			{
				tableStr = tkz.GetNextToken();
				//wxLogDebug("%s: .... table-str=[%s].", FNAME, tableStr.c_str());
				_AddTableToList( tableStr );
			}
		}
		else if ( command == "tCmd" )
		{
			wxString tCmd = paramsStr.BeforeFirst(0x10);
			wxLogDebug("%s: Processing tCmd=[%s] command...", FNAME, tCmd.c_str());

			/* TODO: Only support 1 table for now. */
			const hoxRoleList roles = this->GetRoles();
			if ( roles.empty() )
			{
				wxLogDebug("%s: *** WARN *** This player [%s] has not joined any table yet.", 
					FNAME, this->GetName().c_str());
				goto exit_label;
			}
			wxString tableId = roles.front().tableId;

			// Find the table hosted on this system using the specified table-Id.
			hoxTable* table = this->GetSite()->FindTable( tableId );
			if ( table == NULL )
			{
				wxLogDebug("%s: *** WARN *** Table [%s] not found.", FNAME, tableId.c_str());
				goto exit_label;
			}

			if ( tCmd == "MvPts" )
			{

				wxString delims;
				delims += 0x10;   // move-delimiter
				// ... Do not return empty tokens
				wxStringTokenizer tkz( paramsStr.AfterFirst(0x10), delims, wxTOKEN_STRTOK );
				wxString moveStr;
				while ( tkz.HasMoreTokens() )
				{
					moveStr = tkz.GetNextToken();
					wxLogDebug("%s: .... move-str=[%s].", FNAME, moveStr.c_str());
					// Inform our table...
					table->OnMove_FromNetwork( this, moveStr );
				}
			}
			else if ( tCmd == "Move" )
			{
				wxString delims;
				delims += 0x10;   // move-delimiter
				// ... Do not return empty tokens
				wxStringTokenizer tkz( paramsStr.AfterFirst(0x10), delims, wxTOKEN_STRTOK );
				wxString moveStr;
				while ( tkz.HasMoreTokens() )
				{
					// Just get the 1st token (containing the Move) then get out.
					moveStr = tkz.GetNextToken();
					wxLogDebug("%s: .... [Update] Move =[%s].", FNAME, moveStr.c_str());
					// Inform our table...
					table->OnMove_FromNetwork( this, moveStr );
					break;
				}
			}
		}
		else
		{
			wxLogDebug("%s: Ignore other command = [%s] for now.", FNAME, command.c_str());
		}

        //result = this->HandleIncomingData( response->content );
        //if ( result != hoxRESULT_OK )
        //{
        //    wxLogError("%s: Error occurred while handling incoming data.", FNAME);
        //    return;
        //}
    }

exit_label:
    wxLogDebug("%s: END.", FNAME);
}

bool 
hoxChesscapePlayer::_AddTableToList( const wxString& tableStr ) const
{
	const char* FNAME = "hoxChesscapePlayer::_AddTableToList";
	wxString delims;
	delims += 0x10;
	// ... Do not return empty tokens
	wxStringTokenizer tkz( tableStr, delims, wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;
	hoxNetworkTableInfo tableInfo;
	wxString debugStr;  // For Debug purpose only!

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		debugStr << '[' << token << "]";
		switch (tokenPosition)
		{
			case 0: /* Table-Id */
				tableInfo.id = token; 
				break;

			case 1: /* Group: Public / Private */
				tableInfo.status = (token == "0" ? 0 : 1); 
				break;

			case 2: /* Timer symbol 'T' */
				if ( token != "T" )
					wxLogDebug("%s: *** WARN *** This token [%s] should be 'T].", FNAME, token.c_str());
				break;

			case 3: /* Timer: Game-time (in minutes) */
				break;

			case 4: /* Timer: Increment-time (in seconds) */
				break;

			case 5: /* Table-type: Rated / Nonrated / Solo */
				break;

			case 6: /* Players-info */
			{
				wxString playersInfo = token;
				wxString delims;
				delims += 0x20;
				// ... Do not return empty tokens
				wxStringTokenizer tkz( playersInfo, delims, wxTOKEN_STRTOK );
				int pPosition = 0;
				wxString ptoken;
				while ( tkz.HasMoreTokens() )
				{
					token = tkz.GetNextToken();
					switch (pPosition)
					{
						case 0:	tableInfo.redId = token;   break;
						case 1:	tableInfo.redScore = token; break;
						case 2:	tableInfo.blackId = token; break;
						case 3:	tableInfo.blackScore = token; break;
						default:                           break;
					}
					++pPosition;
				}
				break;
			}

			default: /* Ignore the rest. */ break;
		}
		++tokenPosition;
	}		
	wxLogDebug("%s: ... %s", FNAME, debugStr.c_str());

	/* Insert into our list. */
	m_networkTables.push_back( tableInfo );

	return true;
}

void 
hoxChesscapePlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxChesscapePlayer::OnConnectionResponse";
    hoxResult     result;
    int           returnCode = -1;
    wxString      returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

	/* For JOIN, lookup the tableInfo and return it. */
	if ( response->type == hoxREQUEST_TYPE_JOIN )
	{
		const wxString tableId = response->content;
		for ( hoxNetworkTableInfoList::const_iterator it = m_networkTables.begin();
													  it != m_networkTables.end(); 
													++it )
		{
			if ( it->id == tableId )
			{
				response->eventObject = new hoxNetworkTableInfo( *it );
				wxEvtHandler* sender = response->sender;
				response.release();
				wxPostEvent( sender, event );
				break;
			}
		}
		return;
	}

    if ( response->type == hoxREQUEST_TYPE_LEAVE )
    {
        wxLogDebug("%s: LEAVE (table) 's response received. END.", FNAME);
        return;
    }

    if ( response->sender && response->sender != this )
    {
        wxEvtHandler* sender = response->sender;
        response.release();
        wxPostEvent( sender, event );
        return;
    }

    if ( response->type == hoxREQUEST_TYPE_OUT_DATA )
    {
        wxLogDebug("%s: OUT_DATA 's response received. END.", FNAME);
        return;
    }

    /* Parse the response */
    result = hoxNetworkAPI::ParseSimpleResponse( response->content,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK || returnCode != 0 )
    {
        wxLogDebug("%s: *** WARN *** Failed to parse the response. [%d] [%s]", 
            FNAME,  returnCode, returnMsg.c_str());
        return;
    }

    wxLogDebug("%s: The response is OK.", FNAME);
}

/************************* END OF FILE ***************************************/