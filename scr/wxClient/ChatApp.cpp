#include <wx/busyinfo.h>
#include <wx/log.h>
#include "ConnectDialog.h"
#include "ChatWindow.h"
#include "ChatApp.h"

wxIMPLEMENT_APP(ChatApp);
wxDEFINE_EVENT(EVT_CONVERSATION, wxThreadEvent);
wxDEFINE_EVENT(EVT_MESSAGE, wxThreadEvent);
wxDEFINE_EVENT(EVT_ERROR, wxThreadEvent);

bool ChatApp::OnInit()
{
	Bind(EVT_ERROR, &ChatApp::HandleErrorEvent, this);
	if (!ChatClient::InitNetwork()) 
	{
		wxMessageBox("Network inicialization failed", "Error", wxICON_ERROR | wxSTAY_ON_TOP);
		exit(EXIT_FAILURE);
	}
	mainWindow = new ChatWindow(nullptr, wxID_ANY, "Chat");
	mainWindow->Show(true);
	
	mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnConnect, this, ID_CONNECT);
	//mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnDisconnect, this, ID_DISCONNECT); //TODO: Add later
	mainWindow->Bind(wxEVT_TEXT_ENTER, &ChatApp::OnSendMessage, this); //TODO: Maybe bind after a connection is established?
	Connect();
	return true;
}

int ChatApp::OnExit()
{
	ChatClient::Disconnect();
	return 0;
}

void ChatApp::OnError(const std::string& errorMsg)
{
	wxThreadEvent event(EVT_ERROR);
	event.SetString(errorMsg);
	QueueEvent(event.Clone());
}

void ChatApp::OnLoginSuccessful(const std::vector<std::pair<const msg::targetId, std::string>>& conversationList)
{
	for (const auto &cnv: conversationList)
	{
		// @METO:	This direct approach works because OnLoginSuccesfull() is only called on the main thread.
		//			Alternatavely I can queue a EVT_CONVERSATION for every conversation but it seems like an overkll. Should I?
		mainWindow->CreateConversation(static_cast<wxWindowID>(cnv.first), cnv.second);
	}
	mainWindow->SetStatusText("Welcome to Chat!");
}

void ChatApp::OnLoginFailed()
{
	wxMessageBox("Your login request was denied...", "Login Failed");
}

void ChatApp::OnNewConversation(const msg::targetId id, const std::string& name)
{
	wxThreadEvent networkEvent(EVT_CONVERSATION);
	networkEvent.SetId(static_cast<int>(id));
	networkEvent.SetString(name);
	QueueEvent(networkEvent.Clone());
}

void ChatApp::OnMessageReceived(const msg::targetId target, const msg::targetId sender, const std::string& message)
{
	wxThreadEvent networkEvent(EVT_MESSAGE);
	if (IsMe(target))
	{
		networkEvent.SetId(static_cast<int>(sender));
	}
	else
	{
		networkEvent.SetId(static_cast<int>(target));
	}
	networkEvent.SetInt(static_cast<int>(sender));
	networkEvent.SetString(message);
	QueueEvent(networkEvent.Clone());
}

void ChatApp::OnDisconnect()
{
	//TODO: Maybe clear main window contact list
	if (updateThread)
	{
		updateThread->join();
		delete(updateThread);
	}
}

void ChatApp::Connect()
{
	if (IsConnected())
	{
		ChatClient::Disconnect();
	}
	ConnectDialog connectPopup(nullptr, serverIP, serverPort, userName);
	if (connectPopup.ShowModal() == wxID_OK)
	{
		wxBusyInfo busyWindow("Connecting...", GetTopWindow());
		if (ConnectToServer(std::string(serverIP), wxAtoi(serverPort), std::string(userName), 2, 1000))
		{
			Bind(EVT_CONVERSATION, &ChatWindow::OnNewConversation, mainWindow);
			Bind(EVT_MESSAGE, &ChatWindow::OnMessageReceived, mainWindow);
			updateThread = new std::thread(&ChatApp::Update, this);
		}
	}
}

void ChatApp::Update()
{
	while (IsConnected())
	{
		Sleep(1000);
		ListenForMessage(1000);
	}
}

void ChatApp::HandleErrorEvent(wxThreadEvent& event)
{
	wxMessageBox(event.GetString(), "Error", wxICON_ERROR | wxSTAY_ON_TOP);
}


void ChatApp::OnSendMessage(const wxCommandEvent& event)
{
	//@ METO: _ASSERTE(__acrt_first_block == header) fails when exiting function scope!!! No fix for that bugger.
	const std::string message = event.GetString().ToStdString();
	SendTextMessage(event.GetInt(), message);
}

void ChatApp::OnConnect(const wxCommandEvent& event)
{
	Connect();
}

void ChatApp::OnDisonnect(const wxCommandEvent& event)
{
	wxMessageBox(wxString::Format("TODO: OnDisonnect()\nevent.GetId() %i\nevent.GetInt() %i\nevent.GetString() %s\n\nID_DISCONNECT: %i",
		event.GetId(),
		event.GetInt(),
		event.GetString(),
		ID_DISCONNECT
	), "ChatApp");
	ChatClient::Disconnect();
}
