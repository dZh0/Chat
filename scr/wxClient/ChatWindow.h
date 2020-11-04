#pragma once
#include <wx/wx.h>

class MessageBoard;

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

	MessageBoard* messageBoard = nullptr;
	wxTextCtrl* inputField = nullptr;
};

class MessageBoard : public wxScrolledWindow
{
public:
	MessageBoard(wxWindow* parent, wxWindowID id = wxID_ANY);
	virtual void AddMessage(wxString senderName, wxString messageContent);
private:
	wxBoxSizer* sizer = nullptr;
};

class TextMessage : public wxPanel
{
public:
	TextMessage(wxWindow* parent, wxWindowID id, wxString senderName, wxString content);
};
