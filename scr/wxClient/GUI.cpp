#include "GUI.h"

constexpr char SERVER_IP[] = "localhost";				// The IP of the server, the client is trying to connect to;
constexpr int PORT = 1234;								// The port of the server, the client is trying to connect to;

ChatWindow::ChatWindow()
	: wxFrame(NULL, wxID_ANY, "Chat Client" , wxDefaultPosition, wxSize(450, 340))
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(1, "&Connect...\tCtrl-C", "Connect to server.");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
	CreateStatusBar();
	SetStatusText("Welcome to Chat!");
}

void ChatWindow::Connect()
{
	wxMessageBox("Connecting...", "Connecting", wxCANCEL);
	SetStatusText("Connecting...");
}

void ChatWindow::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ChatWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets' Hello world sample",
		"About Hello World", wxOK | wxICON_INFORMATION);
}

void ChatWindow::Update(wxIdleEvent& event)
{

}

void ChatWindow::OnConnect(wxCommandEvent& event)
{
	Connect();
}

void ChatWindow::OnConnected(wxCommandEvent& event)
{
	SetStatusText("Connected!");
}

ConnectDialog::ConnectDialog()
: wxDialog(nullptr, wxID_ANY, "Connect to",wxDefaultPosition,wxDefaultSize, wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER,"ConnectDialog")
{
	wxPanel* panel = new wxPanel(this, -1);

	wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* controls = new wxBoxSizer(wxHORIZONTAL);
	wxFlexGridSizer* grid = new wxFlexGridSizer(3, 2, 9, 25);
	wxStaticText* ipText = new wxStaticText(panel, wxID_ANY, "Server IP:");
	wxStaticText* socketText = new wxStaticText(panel, wxID_ANY, "Server Socket:");
	wxStaticText* nameText = new wxStaticText(panel, wxID_ANY, "User name:");
	wxTextCtrl* ipInput = new wxTextCtrl(panel, wxID_ANY, "localhost");
	wxTextCtrl* socketInput = new wxTextCtrl(panel, wxID_ANY, "1234");
	wxTextCtrl* nameInput = new wxTextCtrl(panel, wxID_ANY, "Bob");
	wxButton* connectButton = new wxButton(panel, wxID_OK, "Connect");
	wxButton* cancelButton = new wxButton(panel, wxID_CANCEL, "Cancel");

	grid->Add(ipText);
	grid->Add(ipInput, 1, wxEXPAND);
	grid->Add(socketText);
	grid->Add(socketInput, 1, wxEXPAND);
	grid->Add(nameText, 1, wxEXPAND);
	grid->Add(nameInput, 1, wxEXPAND);

	grid->AddGrowableCol(1, 1);

	contentBox->Add(grid, 1, wxALL | wxEXPAND, 15);
	controls->Add(connectButton, 0);
	controls->Add(cancelButton, 0, wxLEFT, 5);
	contentBox->Add(controls,0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
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
