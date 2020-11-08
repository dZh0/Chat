#pragma once
#include "ConversationList.h"
#include "MessageBoard.h"
#include <wx/wx.h>

class ChatWindow : public wxFrame
{
public:
	ChatWindow();
	void OnMessageRecieved(wxCommandEvent& event);
private:
	void OnConnect(wxCommandEvent&);
	void OnExit(wxCommandEvent&);
	void OnAbout(wxCommandEvent&);
	void OnSendMessage(wxCommandEvent& event);

	ConversationList* convList = nullptr;
	MessageBoard* messageBoard = nullptr;
	wxTextCtrl* inputField = nullptr;
};