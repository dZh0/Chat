#include <wx/splitter.h>
#include <assert.h>
#include "ChatApp.h"
#include "ChatWindow.h"

wxDEFINE_EVENT(EVT_CONVERSATION, wxCommandEvent);
wxDEFINE_EVENT(EVT_MESSAGE, wxCommandEvent);

ChatWindow::ChatWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, 	const wxString& name)
	: wxFrame(parent, id, title, pos, size, style, name)
{
	// Dropdown menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	wxMenu* fileDropdown = new wxMenu;
	{
		fileDropdown->Append(ID_CONNECT, "&Connect...\tCtrl-C", "Connect to server.");
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnConnect, this, ID_CONNECT);
		fileDropdown->AppendSeparator();
		fileDropdown->Append(wxID_EXIT);
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnClose, this, wxID_EXIT);
	}
	menuBar->Append(fileDropdown, "&File");
	wxMenu* helpDropdown = new wxMenu;
	{
		helpDropdown->Append(wxID_ABOUT);
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ChatWindow::OnAbout, this, wxID_ABOUT);
		menuBar->Append(helpDropdown, "&Help");
	}
	SetMenuBar(menuBar);
	
	// Main window
	wxSplitterWindow* splitter = new  wxSplitterWindow(this);
	splitter->SetMinimumPaneSize(21);
	convList = new ConversationList(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE);
	Bind(EVT_MESSAGE, &ConversationList::OnMessage, convList, ID_RECEIVE);
	
	messagePannel = new wxPanel(splitter, wxID_ANY);
	//HACK - Fuggure out a way to remove emptyMessageBoard
	MessageBoard* emptyMessageBoard = new MessageBoard(messagePannel, wxID_ANY);
	activeMessageBoard = emptyMessageBoard;
	convList->Bind(wxEVT_LISTBOX, &ChatWindow::OnSelectMessageBoard, this);
	Bind(EVT_MESSAGE, &MessageBoard::OnMessage, messageBoard);

	inputField = new wxTextCtrl(messagePannel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	inputField->Bind(wxEVT_TEXT_ENTER, &ChatWindow::OnSendMessage, this);
	inputField->Bind(wxEVT_TEXT_ENTER, &MessageBoard::OnMessage, emptyMessageBoard);

	wxBoxSizer* chatBox = new wxBoxSizer(wxVERTICAL);
	chatBox->Add(emptyMessageBoard, wxSizerFlags(1).Expand());
	chatBox->Add(inputField, wxSizerFlags(0).Expand());
	messagePannel->SetSizer(chatBox);

	// Main Window bindings
	splitter->SplitVertically(convList, messagePannel, 200);

	// Status bar
	CreateStatusBar();
	SetStatusText("Welcome to Chat!");
}

void ChatWindow::OnNewConversation(const wxThreadEvent& event)
{
	CreateConversation(event.GetInt(), event.GetString());
}

MessageBoard* ChatWindow::CreateConversation(const wxWindowID id, const wxString& name)
{
	MessageBoard* newCnv = new MessageBoard(messagePannel, id, wxDefaultPosition, wxDefaultSize, wxScrolledWindowStyle, name);
	// TODO: Remove after debug
	newCnv->AddMessage(wxString::Format("Now talking to %s.", name), wxString::Format("%i", id));

	newCnv->Hide();
	conversations.push_back(newCnv);
	convList->Append(name, newCnv);
	return newCnv;
}


void ChatWindow::OnConnect(wxCommandEvent&)
{
	wxMessageBox("OnConnect()", "ChatWindow");
}


void ChatWindow::OnClose(wxCommandEvent& event)
{
	wxMessageBox("OnExit()", "ChatWindow");
	Disconnect();
}

void ChatWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This about section will probably never be filled...", "About", wxOK | wxICON_INFORMATION);
}

void ChatWindow::OnNetworkEvent(wxThreadEvent& event)
{
	wxMessageBox(wxString::Format("OnNetworkEvent(%i): %s", event.GetId(), event.GetString()), "ChatWindow", wxOK | wxICON_INFORMATION);
}

void ChatWindow::OnSendMessage(wxCommandEvent& event)
{
	wxMessageBox("OnSendMessage()", "ChatWindow");
	wxString message = event.GetString();
	wxCommandEvent forward_event(EVT_MESSAGE, ID_SEND);
	wxPostEvent(this, forward_event);
	if (message.IsEmpty())
	{
		return;
	}
}

void ChatWindow::OnSelectMessageBoard(const wxCommandEvent &event)
{
	wxSizer* chatBox = messagePannel->GetSizer();
	MessageBoard* newMsgBoard = static_cast<MessageBoard*>(event.GetClientData());
	assert(newMsgBoard != nullptr); //Should never recive a wxEVT_LISTBOX event without ClientData*. Check if ChatWindow::CreateConversation() has properly created a MessageBoard.
	chatBox->Replace(activeMessageBoard, newMsgBoard);
	activeMessageBoard->Hide();
	activeMessageBoard = newMsgBoard;
	activeMessageBoard->Show();
	messagePannel->Layout();
	inputField->SetHint(wxString::Format("Write to %s . . .", activeMessageBoard->GetName()));
}
