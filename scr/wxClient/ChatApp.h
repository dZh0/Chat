#pragma once
#include <wx/app.h>
#include "Client.h"

class ChatApp : public wxApp, public ChatClient
{
public:
	virtual bool OnInit() override;
	virtual int OnExit() override;
	virtual void OnError(const std::string& errorMsg) const override;

	//TODO: These should be at some point saved and loaded
	wxString serverIP = "localhost";
	wxString serverPort = "1234";
	wxString userName = "Bob";
};
wxDECLARE_APP(ChatApp);

wxDECLARE_EVENT(EVT_NETWORK, wxCommandEvent);

constexpr int MESSAGE_RECIEVED_ID = 10000;
class UpdateThread : public wxThread
{
public:
	UpdateThread(wxFrame* parent);
	virtual ExitCode Entry();
private:
	wxFrame* m_parent;
};
