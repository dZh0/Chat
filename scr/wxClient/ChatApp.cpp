#include <wx/busyinfo.h>
#include <wx/log.h>
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

ChatWindow::ChatWindow()
	: wxFrame(NULL, wxID_ANY, "Chat Client", wxDefaultPosition, wxSize(800, 600))
{
	// Dropdown menu bar
	wxMenu* fileDropdown = new wxMenu;
	const int ID_CONNECT = 40;
	fileDropdown->Append(ID_CONNECT, "&Connect...\tCtrl-C", "Connect to server.");
	fileDropdown->AppendSeparator();
	fileDropdown->Append(wxID_EXIT);
	wxMenu* helpDropdown = new wxMenu;
	helpDropdown->Append(wxID_ABOUT);
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(fileDropdown, "&File");
	menuBar->Append(helpDropdown, "&Help");
	SetMenuBar(menuBar);


	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnConnect, this, ID_CONNECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnExit, this, wxID_EXIT);

	wxPanel* panel = new wxPanel(this, -1);

	wxBoxSizer* contentBox = new wxBoxSizer(wxHORIZONTAL);
	wxListBox* conversationList = new wxListBox(panel, wxID_ANY, wxPoint(-1, -1), wxSize(200, -1), wxArrayString(40,"Item"));
	contentBox->Add(conversationList, 1, wxEXPAND);

	wxBoxSizer* chatBox = new wxBoxSizer(wxVERTICAL);
	messageBoard = new MessageBoard(panel, wxID_ANY);

	inputField = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600,-1), wxTE_PROCESS_ENTER);
	inputField->SetHint(wxT("Enter message"));
	inputField->Bind(wxEVT_TEXT_ENTER, &ChatWindow::OnSendMessage, this, wxID_ANY);

	chatBox->Add(messageBoard, 1, wxEXPAND);
	chatBox->Add(inputField, 0, wxEXPAND);
	contentBox->Add(chatBox, 3, wxEXPAND);

	panel->SetSizer(contentBox);

	Centre();
	// Status bar
	CreateStatusBar();
	SetStatusText("Welcome to Chat!");
}

void ChatWindow::OnConnect(wxCommandEvent&)
{
	wxGetApp().Connect();
}

void ChatWindow::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ChatWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This about section will probably never be filled...", "About", wxOK | wxICON_INFORMATION);
}

void ChatWindow::OnMessageRecieved(wxCommandEvent& event)
{
	messageBoard->AddMessage(wxT("Thread"), event.GetString());
}

void ChatWindow::OnSendMessage(wxCommandEvent& event)
{
	if(!inputField->IsEmpty())
	{
		messageBoard->AddMessage(wxT("Me"), inputField->GetValue());
		inputField->Clear();
	}
}

ConnectDialog::ConnectDialog()
	: wxDialog(nullptr, wxID_ANY, "Connect to", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER, "ConnectDialog")
{
	wxPanel* panel = new wxPanel(this, -1);

	wxBoxSizer* contentBox =	new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* controls =		new wxBoxSizer(wxHORIZONTAL);
	wxFlexGridSizer* grid =		new wxFlexGridSizer(3, 2, 9, 10);
	wxStaticText* ipText =		new wxStaticText(panel, wxID_ANY, "Server IP:");
	wxStaticText* socketText =	new wxStaticText(panel, wxID_ANY, "Server Socket:");
	wxStaticText* nameText =	new wxStaticText(panel, wxID_ANY, "User name:");
	wxTextCtrl* ipInput =		new wxTextCtrl(panel, wxID_ANY, DEFAULT_SERVER_IP);
	wxTextCtrl* socketInput =	new wxTextCtrl(panel, wxID_ANY, DEFAULT_PORT);
	wxTextCtrl* nameInput =		new wxTextCtrl(panel, wxID_ANY, "Bob");
	wxButton* connectButton =	new wxButton(panel, wxID_OK, "Connect");
	wxButton* cancelButton =	new wxButton(panel, wxID_CANCEL, "Cancel");

	grid->Add(ipText);
	grid->Add(ipInput, 1, wxEXPAND);
	grid->Add(socketText);
	grid->Add(socketInput, 1, wxEXPAND);
	grid->Add(nameText, 1, wxEXPAND);
	grid->Add(nameInput, 1, wxEXPAND);

	grid->AddGrowableCol(1, 1);

	contentBox->	Add(grid, 1, wxALL | wxEXPAND, 15);
	controls->		Add(connectButton, 0);
	controls->		Add(cancelButton, 0, wxLEFT, 5);
	contentBox->	Add(controls, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
	panel->SetSizer(contentBox);

}

bool ConnectDialog::Validate()
{
	//TODO validate window data
	return true;
}

bool ConnectDialog::TransferDataFromWindow()
{
	//TODO do I need to transfer anything?
	return false;
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



MessageBoard::MessageBoard(wxWindow* parent, wxWindowID id):
	wxScrolledWindow(parent, id)
{
	sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(sizer);
	this->FitInside();
	this->SetScrollRate(-1, 5);
}

void MessageBoard::AddMessage(wxString senderName, wxString messageContent)
{
	TextMessage* msg = new TextMessage(this, wxID_ANY, senderName, messageContent);
	sizer->Add(msg, 0, wxALL, 3);
	this->SetSizer(sizer);
	this->FitInside();
	this->SetScrollRate(-1, 5);
}

TextMessage::TextMessage(wxWindow* parent, wxWindowID id, wxString senderName, wxString content):wxPanel(parent,id)
{
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(this, wxID_ANY, senderName),0, wxALIGN_LEFT);
	wxStaticText* msgText = new wxStaticText(this, wxID_ANY, content);
	hbox->Add(msgText, 1);
	this->SetSizer(hbox);
}
