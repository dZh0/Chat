#include <wx/busyinfo.h>
#include <wx/log.h>
#include "ConnectDialog.h"
#include "ChatWindow.h"
#include "ChatApp.h"

wxIMPLEMENT_APP(ChatApp);

bool ChatApp::OnInit()
{
	ChatWindow* mainWindow = new ChatWindow();
	mainWindow->Show(true);
	if (!ChatClient::InitNetwork())
	{
		mainWindow->Destroy();
	}
	ConnectDialog* connectPopup = new ConnectDialog(serverIP, serverPort, userName);
	if (connectPopup->ShowModal() == wxID_OK)
	{
		wxBusyInfo busyWindow("Connecting...", mainWindow);
		if (ConnectTo(std::string(serverIP), wxAtoi(serverPort)))
		{
			busyWindow.UpdateLabel("Loggining in...");
			RequestLogIn(std::string(userName));
			while (id==0)
			{
				Update();
				Sleep(1000);
				//TODO: This will update to eternity so maybe add request attmpts and a graceful exit
			}
		}
	}

	UpdateThread* updThread = new UpdateThread(mainWindow);
	wxThreadError err = updThread->Create();
	if (err != wxTHREAD_NO_ERROR)
	{
		OnError("Can't create thread!");
		return false;
	}
	err = updThread->Run();
	if (err != wxTHREAD_NO_ERROR)
	{
		OnError("Couldn't run thread!");
		return false;
	}
	mainWindow->Bind(EVT_NETWORK, &ChatWindow::OnMessageRecieved, mainWindow);

	return true;
}

wxDEFINE_EVENT(EVT_NETWORK, wxCommandEvent);

int ChatApp::OnExit()
{
	
	return 0;
}

void ChatApp::OnError(const std::string& errorMsg) const
{
	wxMessageBox(errorMsg, "Error", wxICON_ERROR);
}

UpdateThread::UpdateThread(wxFrame* parent) : m_parent(parent)
{ };

wxThread::ExitCode UpdateThread::Entry()
{
	for (int n = 0; n < 5000; n++)
	{
		Sleep(3000);

		wxCommandEvent event(EVT_NETWORK, MESSAGE_RECIEVED_ID);
		event.SetString(wxString::Format("Test recieved message %i\nNext line of the message is made to test sizing and word warp (which currently does not work):\nLorem ipsum dolor sit amet, consectetur adipiscing elit.Quisque viverra nunc quis sodales fringilla.Aenean nibh velit, rutrum vel magna et, gravida rhoncus libero.Quisque eu gravida libero.Interdum et malesuada fames ac ante ipsum primis in faucibus.Aliquam ante nulla, vehicula finibus magna eu, egestas maximus odio.Morbi malesuada risus eget metus porttitor dapibus et ut magna.In scelerisque a mi eu commodo.Nullam dictum molestie nisi, tincidunt cursus ligula facilisis at.Cras quis leo at felis semper porta id a enim.Morbi ornare, felis in maximus lobortis, sem leo egestas enim, id varius sem ante gravida risus.Ut quis sem commodo, consectetur lacus quis, sagittis elit.Etiam molestie neque vitae risus blandit, et elementum felis volutpat.Etiam blandit est in porta convallis.Nulla facilisi.Proin posuere ut mi a porttitor.", n));
		m_parent->GetEventHandler()->AddPendingEvent(event);
	}
	return 0;
}
