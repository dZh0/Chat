#pragma once
#include <wx/wx.h>
//#include "Client.h"



class ChatApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void Connect();
private:
	bool isConnectionSet = false;
};
wxDECLARE_APP(ChatApp);

wxDECLARE_EVENT(EVT_NETWORK, wxCommandEvent);


class MessageBoard;

class ChatWindow : public wxFrame
{
public:
	ChatWindow();
	const int MESSAGE_PANNEL_ID = 500;
	void OnMessageRecieved(wxCommandEvent& event);
private:
	void OnConnect(wxCommandEvent&);
	void OnExit(wxCommandEvent&);
	void OnAbout(wxCommandEvent&);
	void OnSendMessage(wxCommandEvent& event);

	MessageBoard* messageBoard = nullptr;
	wxTextCtrl* inputField = nullptr;
};

class ConnectDialog : public wxDialog
{
public:
	ConnectDialog();

	bool Validate() override;
	bool TransferDataFromWindow() override;
};


constexpr int MESSAGE_RECIEVED_ID = 10000;
class UpdateThread : public wxThread
{
public:
	UpdateThread(wxFrame* parent);
	virtual ExitCode Entry();
private:
	wxFrame* m_parent;
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