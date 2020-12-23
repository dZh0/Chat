#include <wx/button.h>
#include <wx/wupdlock.h>
#include <wx/msgdlg.h> //TODO: Remove after testing
#include "MessageBoard.h"

#include <wx/stattext.h>

constexpr int VERT_SCROLL_SPEED = 5;

MessageBoard::MessageBoard::MessageBoard(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
	wxScrolledWindow(parent, winid, pos, size, style | wxVSCROLL, name)
{
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);
	SetScrollRate(-1, VERT_SCROLL_SPEED);
}

bool TextMessage::Create(wxWindow* parent, wxWindowID winid, const wxString& senderName, const wxString& content)
{
	if (!wxPanel::Create(parent, winid, wxDefaultPosition, wxDefaultSize))
	{
		return false;
	}
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(new wxButton(this, wxID_ANY, senderName), wxSizerFlags(0).Top());
	wxStaticText* textField = new wxStaticText(this, wxID_ANY, content);
	sizer->Add(textField, wxSizerFlags(1).Expand().Border(wxLEFT));
	SetSizerAndFit(sizer);
	return true;
}

//TODO: Remove when the events start working
void MessageBoard::AddMessage(const wxString& messageContent, const wxString& senderName)
{
	//int scrollFromBottom = GetScrollRange(wxVERTICAL) - GetClientSize().GetY()/VERT_SCROLL_SPEED - GetScrollPos(wxVERTICAL); //@METO: Not working
	TextMessage* message = new TextMessage();
	message->Hide();
	message->Create(this, wxID_ANY, senderName, messageContent);
	GetSizer()->Add(message, wxSizerFlags(0).Left().ReserveSpaceEvenIfHidden().Expand().Border());
	FitInside();
	Scroll(message->GetPosition()); //Always scroll as scrollFromBottom can not be derived reliably
	message->Show();
}

void MessageBoard::OnMessage(wxCommandEvent& event)
{
	event.Skip();
	if (event.GetId() == 1)
	{
		wxMessageBox("OnMessage() - Send", "MessageBoard");
	}
	if (event.GetId() == 2)
	{
		wxMessageBox("OnMessage() - Recieve", "MessageBoard");
	}

}