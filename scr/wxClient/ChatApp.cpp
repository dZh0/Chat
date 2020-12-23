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
	//@METO:	Not sure how the mainWindow should access Client functions...
	//			To make the UI as independant as possible all bindings are made in the ChatApp
	mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnConnect, this, ID_CONNECT);
	//mainWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatApp::OnDisconnect, this, ID_DISCONNECT);
	Connect();
	return true;
}


/*
void ChatApp::OnMessageRecieved(wxThreadEvent& event)
{
	//wxMessageBox("OnMessageRecieved", "OnMessageRecieved");
	event.Skip();
}

void ChatApp::OnNewConversation(wxThreadEvent& event)
{
	wxMessageBox(event.GetString(), wxString::Format(wxT("%d %s"), event.GetInt(), event.GetString()));
}

void ChatApp::OnNewConversation(wxThreadEvent& event)
{
	wxMessageBox(event.GetString(), wxString::Format(wxT("%d %s"), event.GetInt(), event.GetString()));
}
*/

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
			Bind(EVT_NETWORK, &ChatWindow::OnNetworkEvent, mainWindow);
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

void ChatApp::OnPingMessageRecieved()
{
	wxThreadEvent networkEvent(EVT_NETWORK, static_cast<int>(message::type::PING));
	networkEvent.SetString("Test ping recieved; The next line of the message is made to test sizing and word warp (which currently does not work):\nLorem ipsum dolor sit amet, consectetur adipiscing elit.Quisque viverra nunc quis sodales fringilla.Aenean nibh velit, rutrum vel magna et, gravida rhoncus libero.Quisque eu gravida libero.Interdum et malesuada fames ac ante ipsum primis in faucibus.Aliquam ante nulla, vehicula finibus magna eu, egestas maximus odio.Morbi malesuada risus eget metus porttitor dapibus et ut magna.In scelerisque a mi eu commodo.Nullam dictum molestie nisi, tincidunt cursus ligula facilisis at.Cras quis leo at felis semper porta id a enim.Morbi ornare, felis in maximus lobortis, sem leo egestas enim, id varius sem ante gravida risus.Ut quis sem commodo, consectetur lacus quis, sagittis elit.Etiam molestie neque vitae risus blandit, et elementum felis volutpat.Etiam blandit est in porta convallis.Nulla facilisi.Proin posuere ut mi a porttitor.");
	QueueEvent(networkEvent.Clone());
}

void ChatApp::OnLoginSuccessful(const std::vector<std::pair<const uint32_t, std::string>>& conversationList)
{
/*
	mainWindow->convList->Clear();
	wxCommandEvent event(EVT_CONVERSATION_NEW);
	for (auto cnv : conversationList)
	{
		//@METO: On login might be better to send the whole list in a single event instead of posting potentially hundreds of events.
		event.SetInt(cnv.first);
		event.SetString(cnv.second);
		wxPostEvent(this, event);
	}
	//mainWindow->convList->SetSelection(0);
	*/
}

void ChatApp::HandleErrorEvent(wxThreadEvent& event)
{
	wxMessageBox(event.GetString(), "Error", wxICON_ERROR | wxSTAY_ON_TOP);
}

void ChatApp::OnSendMessage(const wxCommandEvent& event)
{
	wxMessageBox("OnSendMessage()", "ChatApp");
	SendMessageToTarget(event.GetInt(), event.GetString().ToStdString(), 1000);
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
