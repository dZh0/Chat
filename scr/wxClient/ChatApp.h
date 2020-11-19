#pragma once
#include <wx/app.h>
#include <thread>
#include "Client.h"

class ChatApp : public wxApp, public ChatClient
{
public:
	//from wxApp
	virtual bool OnInit() override;
	virtual int OnExit() override;
	//from ChatClient
	virtual void OnError(const std::string& errorMsg) const override;
	//virtual void OnMessageReceived() override;

	void ThreadTest();

	//TODO: These should be at some point saved and loaded
	wxString serverIP = "localhost";
	wxString serverPort = "1234";
	wxString userName = "Bob";
protected:
	std::thread* updateThread;
};
wxDECLARE_APP(ChatApp);

wxDECLARE_EVENT(EVT_NETWORK, wxThreadEvent);