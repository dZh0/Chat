#pragma once
#include "ConversationList.h"
#include "MessageBoard.h"
#include <map>
#include <wx/wx.h>

//@METO:	Is it ok for these to be global? They are used by ChatApp class to identify events.
//			Another brilliant solution from wxWidgets: Neither wxNewId() nor NewControlId allow for the IDs to be passed through the events.
const wxWindowID ID_CONNECT = 1;
const wxWindowID ID_DISCONNECT = 2;
const wxWindowID ID_SEND_MESSAGE = 3;

class ChatWindow : public wxFrame
{
public:
	ChatWindow
	(
		wxWindow* parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(800, 600),
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxFrameNameStr
	);
	void OnNewConversation(wxThreadEvent& event);
	void OnMessageReceived(wxThreadEvent& event);

	MessageBoard* GetConversation(const wxWindowID id) const;
	MessageBoard* CreateConversation(const wxWindowID id, const wxString& name);


private:
	void OnConnect(wxCommandEvent&);
	void OnClose(wxCommandEvent&);
	void OnAbout(wxCommandEvent&);
	void OnSendMessage(wxCommandEvent& event);
	void OnSelectMessageBoard(wxCommandEvent& event);
	void ActivateMessageBoard(MessageBoard* msgBoard);

	wxPanel* messagePannel = nullptr;
	ConversationList* convList = nullptr;
	std::map<const int, MessageBoard*> targets;
	MessageBoard* activeMessageBoard = nullptr;
	wxTextCtrl* inputField = nullptr;
};