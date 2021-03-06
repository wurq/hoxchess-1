/////////////////////////////////////////////////////////////////////////////
// Name:        hoxWelcomeUI.cpp
// Purpose:     
// Author:      Huy Phan
// Modified by: 
// Created:     03/05/2009 07:16:53
// RCS-ID:      
// Copyright:   Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>
// Licence:     GNU General Public License v3
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "hoxWelcomeUI.h"

////@begin XPM images
////@end XPM images


/*!
 * hoxWelcomeUI type definition
 */

IMPLEMENT_DYNAMIC_CLASS( hoxWelcomeUI, wxDialog )


/*!
 * hoxWelcomeUI event table definition
 */

BEGIN_EVENT_TABLE( hoxWelcomeUI, wxDialog )

////@begin hoxWelcomeUI event table entries
    EVT_BUTTON( ID_BTN_PLAY, hoxWelcomeUI::OnBtnPlayClick )

    EVT_BUTTON( ID_BTN_LOGIN, hoxWelcomeUI::OnBtnLoginClick )

    EVT_BUTTON( ID_BTN_OPTIONS, hoxWelcomeUI::OnBtnOptionsClick )

////@end hoxWelcomeUI event table entries

    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW, hoxWelcomeUI::OnHtmlLinkClicked )
END_EVENT_TABLE()


/*!
 * hoxWelcomeUI constructors
 */

hoxWelcomeUI::hoxWelcomeUI()
{
    Init();
}

hoxWelcomeUI::hoxWelcomeUI( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * hoxWelcomeUI creator
 */

bool hoxWelcomeUI::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin hoxWelcomeUI creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end hoxWelcomeUI creation
    return true;
}


/*!
 * hoxWelcomeUI destructor
 */

hoxWelcomeUI::~hoxWelcomeUI()
{
////@begin hoxWelcomeUI destruction
////@end hoxWelcomeUI destruction
}


/*!
 * Member initialisation
 */

void hoxWelcomeUI::Init()
{
////@begin hoxWelcomeUI member initialisation
    m_showNextTimeCheck = NULL;
////@end hoxWelcomeUI member initialisation
}


/*!
 * Control creation for hoxWelcomeUI
 */

void hoxWelcomeUI::CreateControls()
{    
////@begin hoxWelcomeUI content construction
    hoxWelcomeUI* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHtmlWindow* itemHtmlWindow4 = new wxHtmlWindow( itemDialog1, ID_HTMLWINDOW, wxDefaultPosition, wxSize(400, 200), wxHW_SCROLLBAR_AUTO|wxRAISED_BORDER );
    itemHtmlWindow4->SetPage(_("<b>HOXChess</b> - <i>An open source Xiangqi program</i>\n\n<p>\nTo get started, please select one of the actions on the right.\n</p>\n<p>\nVisit <a href=\"http://hoxchess.googlecode.com\">HOXChess on GoogleCode</a> for the latest news and support.\n</p>\n<p>\nCopyright 2007-2009 Huy Phan, <a href=\"http://www.playxiangqi.com\">PlayXiangqi.com</a>\n<br />\nContact: <a href=\"mailto:huyphan@playxiangqi.com\">huyphan@playxiangqi.com</a>\n</p>"));
    itemBoxSizer3->Add(itemHtmlWindow4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_showNextTimeCheck = new wxCheckBox( itemDialog1, ID_CHK_SHOW_WELCOME, _("Show this dialog next time"), wxDefaultPosition, wxDefaultSize, 0 );
    m_showNextTimeCheck->SetValue(true);
    itemBoxSizer3->Add(m_showNextTimeCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_BTN_PLAY, _("&Play With Computer"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxGROW|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_BTN_LOGIN, _("&Login Remote Server"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxGROW|wxALL, 5);

    itemBoxSizer6->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_BTN_OPTIONS, _("Change &Options..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton10, 0, wxGROW|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CANCEL, _("&Close Dialog"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetDefault();
    itemBoxSizer6->Add(itemButton11, 0, wxGROW|wxALL, 5);

////@end hoxWelcomeUI content construction

    // Create custom windows not generated automatically here.
////@begin hoxWelcomeUI content initialisation
////@end hoxWelcomeUI content initialisation
}


/*!
 * Should we show tooltips?
 */

bool hoxWelcomeUI::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap hoxWelcomeUI::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin hoxWelcomeUI bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end hoxWelcomeUI bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon hoxWelcomeUI::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin hoxWelcomeUI icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end hoxWelcomeUI icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_PLAY
 */

void hoxWelcomeUI::OnBtnPlayClick( wxCommandEvent& event )
{
    EndDialog( COMMAND_ID_PRACTICE );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_LOGIN
 */

void hoxWelcomeUI::OnBtnLoginClick( wxCommandEvent& event )
{
    EndDialog( COMMAND_ID_REMOTE );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_OPTIONS
 */

void hoxWelcomeUI::OnBtnOptionsClick( wxCommandEvent& event )
{
    EndDialog( COMMAND_ID_OPTIONS );
}


void
hoxWelcomeUI::OnHtmlLinkClicked( wxHtmlLinkEvent& event )
{
    /* Reference:
     *  http://wiki.wxwidgets.org/Calling_The_Default_Browser_In_WxHtmlWindow
     */

    const wxString sHref = event.GetLinkInfo().GetHref();
    if ( sHref.StartsWith( "http://" ) )
    {
		wxLaunchDefaultBrowser( sHref );
    }
    else if ( sHref.StartsWith( "mailto:" ) )
    {
        wxLogDebug("%s: Skip email links: [%s].", __FUNCTION__, sHref.c_str());
    }
    else
    {
        // Skip this event: the default behavior (load the clicked URL)
        // will happen...
        event.Skip();
    }
}




/*!
 * Check if this Dialog should be shown in the next startup.
 */

bool hoxWelcomeUI::ShowNextStartup() const
{
    return m_showNextTimeCheck->IsChecked();
}
