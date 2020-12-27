#include <wx/busyinfo.h>
#include <wx/log.h>
#include "ConnectDialog.h"
#include "ChatWindow.h"
#include "ChatApp.h"

wxIMPLEMENT_APP(ChatApp);
wxDEFINE_EVENT(EVT_NETWORK, wxThreadEvent);
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
	Bind(EVT_NETWORK, &ChatWindow::OnNetworkEvent, mainWindow);
	mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnConnect, this, ID_CONNECT);
	//mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnDisconnect, this, ID_DISCONNECT);
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

void ChatApp::OnPing()
{
	wxThreadEvent networkEvent(EVT_NETWORK, static_cast<int>(msg::type::PING));
	networkEvent.SetString("Test ping recieved; The next line of the message is made to test sizing and word warp (which currently does not work):\nLorem ipsum dolor sit amet, consectetur adipiscing elit.Quisque viverra nunc quis sodales fringilla.Aenean nibh velit, rutrum vel magna et, gravida rhoncus libero.Quisque eu gravida libero.Interdum et malesuada fames ac ante ipsum primis in faucibus.Aliquam ante nulla, vehicula finibus magna eu, egestas maximus odio.Morbi malesuada risus eget metus porttitor dapibus et ut magna.In scelerisque a mi eu commodo.Nullam dictum molestie nisi, tincidunt cursus ligula facilisis at.Cras quis leo at felis semper porta id a enim.Morbi ornare, felis in maximus lobortis, sem leo egestas enim, id varius sem ante gravida risus.Ut quis sem commodo, consectetur lacus quis, sagittis elit.Etiam molestie neque vitae risus blandit, et elementum felis volutpat.Etiam blandit est in porta convallis.Nulla facilisi.Proin posuere ut mi a porttitor.");
	QueueEvent(networkEvent.Clone());
}

void ChatApp::OnLoginSuccessful(const std::vector<std::pair<const msg::targetId, std::string>>& conversationList)
{
	for (const auto &cnv: conversationList)
	{
		// @METO:	This only works because OnLoginSuccesfull() is called on the main thread.
		//			Alternatavely I can queue a EVT_NETWORK for every conversation but it seems like an overkll.
		mainWindow->CreateConversation(static_cast<wxWindowID>(cnv.first), cnv.second);
	}
	//TODO:	Select the first conversation envoking wxEVT_LISTBOX event.
	//		Calling mainWindow->convList->Select(0) does not fire the event;
}

void ChatApp::OnLoginFailed()
{
	wxMessageBox("Your login request was denied...", "Login Failed");
}

void ChatApp::OnNewConversation(const msg::targetId id, const std::string& name)
{
	wxThreadEvent networkEvent(EVT_NETWORK);
	networkEvent.SetId(static_cast<int>(msg::type::NEW_CONVERSATION));
	networkEvent.SetInt(id);
	networkEvent.SetString(name);
	QueueEvent(networkEvent.Clone());
}

void ChatApp::OnMessageReceived(const msg::targetId senderId, const std::string& message)
{
}

void ChatApp::OnDisconnect()
{
	if (!mainWindow->IsBeingDeleted())
	{
		//mainWindow->convList->Clear();
	}
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
			Bind(EVT_NETWORK, &ChatWindow::OnNewConversation, mainWindow, static_cast<int>(msg::type::NEW_CONVERSATION));
			mainWindow->Bind(EVT_MESSAGE, &ChatApp::OnSendMessage, this, ID_SEND);
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
	wxMessageBox("OnSendMessage()", "ChatApp");
	SendTextMessage(event.GetInt(), event.GetString().ToStdString());
}

void ChatApp::OnConnect(const wxCommandEvent& event)
{
	wxMessageBox("OnConnect()", "ChatApp");
	Connect();
}

void ChatApp::OnDisonnect(const wxCommandEvent& event)
{
	wxMessageBox("OnDisconnect()", "ChatApp");
	ChatClient::Disconnect();
}
