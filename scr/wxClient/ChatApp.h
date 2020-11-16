#pragma once
#include <wx/app.h>
#include "Client.h"

class ChatApp : public wxApp, public ChatClient
{
public:
	virtual bool OnInit() override;
	virtual int OnExit() override;
	void Test(wxCommandEvent& event);
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
