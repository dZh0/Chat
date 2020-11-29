#pragma once
#include <wx/app.h>
#include <thread>
#include <future>
#include "Client.h"

class ChatApp : public wxApp, public ChatClient
{
public:
	// from wxApp:
	virtual bool OnInit() override;
	virtual int OnExit() override;
	// from ChatClient:
	virtual void OnError(const std::string& errorMsg) override;
	virtual void OnDisconnect() override;
	virtual void OnPingMessageRecieved() override;

	void Connect();
	void Update();

	void OnMessageRecieved(wxThreadEvent& event);
	void HandleErrorEvent(wxThreadEvent& event);

	//TODO: These should be at some point saved and loaded
	wxString serverIP = "localhost";
	wxString serverPort = "1234";
	wxString userName = "Bob";
protected:
	std::thread* updateThread;
};
wxDECLARE_APP(ChatApp);

wxDECLARE_EVENT(EVT_NETWORK, wxThreadEvent);
wxDECLARE_EVENT(EVT_ERROR, wxThreadEvent); // OnError() can potentially be called from within a thread so it needs to be wxThreadEvent.