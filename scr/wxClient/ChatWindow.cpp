#include <wx/splitter.h>
#include <assert.h>
#include "ChatWindow.h"

ChatWindow::ChatWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, 	const wxString& name)
	: wxFrame(parent, id, title, pos, size, style, name)
{
	// Dropdown menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	wxMenu* fileDropdown = new wxMenu;
	{
		fileDropdown->Append(ID_CONNECT, "&Connect...\tCtrl-C", "Connect to server.");
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
	
	messagePannel = new wxPanel(splitter, wxID_ANY);
	//TODO Fuggure out a way to remove emptyMessageBoard
	MessageBoard* emptyMessageBoard = new MessageBoard(messagePannel, wxID_ANY);
	activeMessageBoard = emptyMessageBoard;
	convList->Bind(wxEVT_LISTBOX, &ChatWindow::OnSelectMessageBoard, this);

	inputField = new wxTextCtrl(messagePannel, ID_SEND_MESSAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_EMPTY));
	inputField->Bind(wxEVT_TEXT_ENTER, &ChatWindow::OnSendMessage, this);
	inputField->Disable();

	wxBoxSizer* chatBox = new wxBoxSizer(wxVERTICAL);
	chatBox->Add(emptyMessageBoard, wxSizerFlags(1).Expand());
	chatBox->Add(inputField, wxSizerFlags(0).Expand());
	messagePannel->SetSizer(chatBox);

	splitter->SplitVertically(convList, messagePannel, 200);

	// Status bar
	CreateStatusBar();
}

void ChatWindow::OnNewConversation(wxThreadEvent& event)
{
	CreateConversation(event.GetId(), event.GetString());
}


void ChatWindow::OnMessageReceived(wxThreadEvent& event)
{
	MessageBoard* cnv = GetConversation(event.GetId());
	if (!cnv) //@METO: I dont think it is possible for a message to be recieved before the conversation is sent by the server but in the current implementation...
	{
		cnv = CreateConversation(event.GetId(), wxString::Format("Conversation_%i", event.GetId()));
	}
	const wxString& name = GetConversation(event.GetInt())->GetName(); //TODO: Find a better way store and retrieve client names.
	cnv->AddMessage(event.GetString(), name);
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

void ChatWindow::OnSendMessage(wxCommandEvent& event)
{
	if (event.GetString().IsEmpty()) //@METO: There is no way to make wxTextValidator(wxFILTER_EMPTY) suspend event emission. Hence, the check.
	{
		return;
	}
	activeMessageBoard->AddMessage(event.GetString(), "Me"); //@METO: Directly adding the message. Might alternatevly capture the wxEVT_TEXT_ENTER event in activeMessageBoard but, considering the crappy handling, I opted for this.
	event.SetInt(activeMessageBoard->GetId());
	event.Skip();
	inputField->Clear();
}

void ChatWindow::OnSelectMessageBoard(wxCommandEvent &event)
{
	ActivateMessageBoard(static_cast<MessageBoard*>(event.GetClientData()));
}

void ChatWindow::ActivateMessageBoard(MessageBoard* msgBoard)
{
	if (msgBoard == nullptr)
	{
		return;
	}
	wxSizer* chatBox = messagePannel->GetSizer();
	chatBox->Replace(activeMessageBoard, msgBoard);
	activeMessageBoard->Hide();
	activeMessageBoard = msgBoard;
	activeMessageBoard->Show();
	messagePannel->Layout();
	inputField->SetHint(wxString::Format("Write to %s . . .", activeMessageBoard->GetName()));
	if (!inputField->IsEnabled())
	{
		inputField->Enable();
	}
}

MessageBoard* ChatWindow::GetConversation(const wxWindowID id) const
{
	auto found = targets.find(id);
	if (found == targets.end())
	{
		return nullptr;
	}
	else
	{
		return found->second;
	}
}

MessageBoard* ChatWindow::CreateConversation(const wxWindowID id, const wxString& name)
{
	bool wasEmpty = convList->IsEmpty();
	MessageBoard* cnv = GetConversation(id);
	if (!cnv)
	{
		cnv = new MessageBoard(messagePannel, id);
		targets[id] = cnv;
	}
	targets[id]->SetName(name);
	convList->Append(targets[id]->GetName(), targets[id]);
	if (wasEmpty)
	{
		convList->SetSelection(0);
		ActivateMessageBoard(targets[id]);
	}
	return targets[id];
}
