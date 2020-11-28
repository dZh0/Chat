#include <wx/busyinfo.h>
#include <wx/log.h>
#include "ConnectDialog.h"
#include "ChatWindow.h"
#include "ChatApp.h"

wxIMPLEMENT_APP(ChatApp);
wxDEFINE_EVENT(EVT_NETWORK, wxThreadEvent);

bool ChatApp::OnInit()
{
	if (!ChatClient::InitNetwork())
	{
		exit(EXIT_FAILURE);
	}
	ChatWindow* mainWindow = new ChatWindow();
	mainWindow->Show(true);
	Bind(EVT_NETWORK, &ChatWindow::OnMessageRecieved, mainWindow);
	Connect();
	return true;
}

void ChatApp::OnMessageRecieved(wxThreadEvent& event)
{
	//wxMessageBox("OnMessageRecieved", "OnMessageRecieved");
	event.Skip();
}


int ChatApp::OnExit()
{
	ChatClient::Disconnect();
	return 0;
}

void ChatApp::OnError(const std::string& errorMsg) const
{
	wxMessageBox(errorMsg, "Error", wxICON_ERROR | wxSTAY_ON_TOP);
}

void ChatApp::OnDisconnect()
{
	if (updateThread)
	{
		exitSignal.set_value();
		updateThread->join();
		delete(updateThread);
	}
}

void ChatApp::Connect()
{
	if (isConnected)
	{
		ChatClient::Disconnect();
	}
	ConnectDialog connectPopup(nullptr, serverIP, serverPort, userName);
	if (connectPopup.ShowModal() == wxID_OK)
	{
		wxBusyInfo busyWindow("Connecting...", GetTopWindow());
		if (ConnectToServer(std::string(serverIP), wxAtoi(serverPort), std::string(userName), 2, 1000))
		{
			Bind(EVT_NETWORK, &ChatApp::OnMessageRecieved, this);
			exitSignal = std::promise<void>();
			updateThread = new std::thread(&ChatApp::ThreadTest, this, std::move(exitSignal.get_future()));
		}
	}
}

constexpr int MESSAGE_RECIEVED_ID = 10000;
void ChatApp::ThreadTest(std::future<void> futureObj)
{
	int n=0;
	while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
	{
		Sleep(5000);
		//Update();
		wxThreadEvent event(EVT_NETWORK);
		event.SetString(wxString::Format("Test recieved message %i\nNext line of the message is made to test sizing and word warp (which currently does not work):\nLorem ipsum dolor sit amet, consectetur adipiscing elit.Quisque viverra nunc quis sodales fringilla.Aenean nibh velit, rutrum vel magna et, gravida rhoncus libero.Quisque eu gravida libero.Interdum et malesuada fames ac ante ipsum primis in faucibus.Aliquam ante nulla, vehicula finibus magna eu, egestas maximus odio.Morbi malesuada risus eget metus porttitor dapibus et ut magna.In scelerisque a mi eu commodo.Nullam dictum molestie nisi, tincidunt cursus ligula facilisis at.Cras quis leo at felis semper porta id a enim.Morbi ornare, felis in maximus lobortis, sem leo egestas enim, id varius sem ante gravida risus.Ut quis sem commodo, consectetur lacus quis, sagittis elit.Etiam molestie neque vitae risus blandit, et elementum felis volutpat.Etiam blandit est in porta convallis.Nulla facilisi.Proin posuere ut mi a porttitor.", n));
		QueueEvent(event.Clone());
		n++;
	}
}


