#include <wx/app.h>
#include "ChatWindow.h"

ChatWindow::ChatWindow()
	: wxFrame(nullptr, wxID_ANY, "Chat Client", wxDefaultPosition, wxSize(800, 600))
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
	wxListBox* conversationList = new wxListBox(panel, wxID_ANY, wxPoint(-1, -1), wxSize(200, -1), wxArrayString(40, "Item"));
	contentBox->Add(conversationList, 1, wxEXPAND);

	wxBoxSizer* chatBox = new wxBoxSizer(wxVERTICAL);
	messageBoard = new MessageBoard(panel, wxID_ANY);

	inputField = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, -1), wxTE_PROCESS_ENTER);
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
	//GetParent()->Connect();
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
	if (!inputField->IsEmpty())
	{
		messageBoard->AddMessage(wxT("Me"), inputField->GetValue());
		inputField->Clear();
	}
}

MessageBoard::MessageBoard(wxWindow* parent, wxWindowID id) :
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

TextMessage::TextMessage(wxWindow* parent, wxWindowID id, wxString senderName, wxString content) :wxPanel(parent, id)
{
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(this, wxID_ANY, senderName), 0, wxALIGN_LEFT);
	wxStaticText* msgText = new wxStaticText(this, wxID_ANY, content);
	hbox->Add(msgText, 1);
	this->SetSizer(hbox);
}
