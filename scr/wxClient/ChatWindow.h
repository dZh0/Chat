#pragma once
#include "ConversationList.h"
#include "MessageBoard.h"
#include <wx/wx.h>


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
	void OnNewConversation(const wxThreadEvent& event);
	MessageBoard* CreateConversation(const wxWindowID id, const wxString& name);
	void OnNetworkEvent(wxThreadEvent& event);

private:
	void OnConnect(wxCommandEvent&);
	void OnClose(wxCommandEvent&);
	void OnAbout(wxCommandEvent&);
	void OnSendMessage(wxCommandEvent& event);
	void OnSelectMessageBoard(const wxCommandEvent& event);

	std::vector<MessageBoard*> conversations;
	wxPanel* messagePannel = nullptr;
	ConversationList* convList = nullptr;

	MessageBoard* activeMessageBoard = nullptr;

	MessageBoard* messageBoard = nullptr;
	wxTextCtrl* inputField = nullptr;
};

wxDECLARE_EVENT(EVT_CONVERSATION_NEW, wxCommandEvent);
wxDECLARE_EVENT(EVT_MESSAGE, wxCommandEvent);
enum { ID_SEND = 1, ID_RECEIVE = 2 };
const wxWindowID ID_CONNECT = wxWindow::NewControlId();
const wxWindowID ID_DISCONNECT = wxWindow::NewControlId();