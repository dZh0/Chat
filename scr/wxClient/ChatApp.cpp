#include <wx/busyinfo.h>
#include <wx/log.h>
#include "ConnectDialog.h"
#include "ChatWindow.h"
#include "ChatApp.h"


constexpr char DEFAULT_SERVER_IP[] = "localhost";				// The IP of the server, the client is trying to connect to;
constexpr char DEFAULT_PORT[] = "1234";								// The port of the server, the client is trying to connect to;

wxIMPLEMENT_APP(ChatApp);

bool ChatApp::OnInit()
{
	ChatWindow* mainWindow = new ChatWindow();
	mainWindow->Show(true);

	UpdateThread* updThread = new UpdateThread(mainWindow);
	wxThreadError err = updThread->Create();
	if (err != wxTHREAD_NO_ERROR)
	{
		wxMessageBox(_("Couldn't create thread!"));
		return false;
	}
	err = updThread->Run();
	if (err != wxTHREAD_NO_ERROR)
	{
		wxMessageBox(_("Couldn't run thread!"));
		return false;
	}
	mainWindow->Bind(EVT_NETWORK, &ChatWindow::OnMessageRecieved, mainWindow);
	Connect();
	return true;
}

wxDEFINE_EVENT(EVT_NETWORK, wxCommandEvent);

void ChatApp::Connect()
{
	if (!isConnectionSet)
	{
		ConnectDialog* connectPopup = new ConnectDialog();
		connectPopup->Show(true);
	}
}

UpdateThread::UpdateThread(wxFrame* parent):m_parent(parent)
{

}

wxThread::ExitCode UpdateThread::Entry()
{
	for (int n = 0; n < 5000; n++)
	{
		this->Sleep(3000);

		wxCommandEvent event(EVT_NETWORK, MESSAGE_RECIEVED_ID);
		event.SetString(wxString::Format("Test recieved message %i", n));
		m_parent->GetEventHandler()->AddPendingEvent(event);
	}
	return 0;
}
