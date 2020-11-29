#include <wx/splitter.h>
#include "ChatWindow.h"

ChatWindow::ChatWindow()
	: wxFrame(nullptr, wxID_ANY, "Chat Client", wxDefaultPosition, wxSize(800, 600))
{
	const wxWindowID ID_CONNECT = wxWindow::NewControlId();
	// Dropdown menu bar
	wxMenu* fileDropdown = new wxMenu;
	fileDropdown->Append(ID_CONNECT, "&Connect...\tCtrl-C", "Connect to server.");
	fileDropdown->AppendSeparator();
	fileDropdown->Append(wxID_EXIT);
	wxMenu* helpDropdown = new wxMenu;
	helpDropdown->Append(wxID_ABOUT);
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(fileDropdown, "&File");
	menuBar->Append(helpDropdown, "&Help");
	SetMenuBar(menuBar);

	// Dropdown bindings
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnConnect, this, ID_CONNECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnExit, this, wxID_EXIT);
	
	// Main window
	wxSplitterWindow* splitter = new  wxSplitterWindow(this);
	splitter->SetMinimumPaneSize(21);
	convList = new ConversationList(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE);
	
	wxPanel* panel = new wxPanel(splitter, wxID_ANY);
	wxBoxSizer* chatBox = new wxBoxSizer(wxVERTICAL);
	messageBoard = new MessageBoard(panel, wxID_ANY);

	inputField = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	inputField->SetHint(wxT("Enter message"));
	chatBox->Add(messageBoard, wxSizerFlags(1).Expand());
	chatBox->Add(inputField, wxSizerFlags(0).Expand());
	panel->SetSizer(chatBox);

	// Main Window bindings
	inputField->Bind(wxEVT_TEXT_ENTER, &ChatWindow::OnSendMessage, this);

	splitter->SplitVertically(convList, panel, 200);

	// Status bar
	CreateStatusBar();
	SetStatusText("Welcome to Chat!");
}

void ChatWindow::OnConnect(wxCommandEvent&)
{
}

void ChatWindow::OnExit(wxCommandEvent& event)
{
	Disconnect();
}

void ChatWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This about section will probably never be filled...", "About", wxOK | wxICON_INFORMATION);
}

void ChatWindow::OnMessageRecieved(wxThreadEvent& event)
{
	messageBoard->AddMessage(event.GetString(), "Thread");
}

void ChatWindow::OnSendMessage(wxCommandEvent& event)
{
	if (!inputField->IsEmpty())
	{
		messageBoard->AddMessage(inputField->GetValue(), "Me");
		convList->AddConversation(inputField->GetValue());
		inputField->Clear();
	}
}